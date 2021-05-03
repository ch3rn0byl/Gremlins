#include "dispatchfunctions.h"

NTSTATUS isInitialized(PIRP Irp, PULONG Information)
{
    __try
    {
        RtlCopyMemory(
            Irp->AssociatedIrp.SystemBuffer,
            &g_Globals->isInitialized,
            sizeof(g_Globals->isInitialized)
        );
        *Information = sizeof(g_Globals->isInitialized);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return STATUS_ACCESS_VIOLATION;
    }
    return STATUS_SUCCESS;
}

NTSTATUS initialize(PIRP Irp)
{
    ANSI_STRING as = { 0 };
    
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    UINT64 ui64KiSystemServiceUser = 0;
    UINT64 ui64KeServiceDescriptorTable = 0;
    UINT64 ui64Lstar = __readmsr(IA32_LSTAR_MSR);

    ///
    /// Sanity check
    /// 
    if (Irp)
    {
        RtlInitAnsiString(&as, static_cast<PCSZ>(Irp->AssociatedIrp.SystemBuffer));
        
        Status = RtlAnsiStringToUnicodeString(&g_Globals->BinaryName, &as, TRUE);
        if (!NT_SUCCESS(Status))
        {
            DbgPrint("[%ws::%d] RtlAnsiStringToUnicodeString failed: %08x\n", __FUNCTIONW__, __LINE__, &Status);
            return Status;
        }
    }
    else
    {
        DbgPrint("[%ws::%d] Received a null irp.\n", __FUNCTIONW__, __LINE__);
        return STATUS_IO_DEVICE_ERROR;
    }

    Status = resolve::KiSystemServiceUser(ui64Lstar, &ui64KiSystemServiceUser);
    if (!NT_SUCCESS(Status) || ui64KiSystemServiceUser == 0)
    {
        DbgPrint("[%ws::%d] Unable to resolve KiSystemServiceUser.\n", __FUNCTIONW__, __LINE__);
        return STATUS_NOT_FOUND;
    }

    Status = resolve::KeServiceDescriptorTable(ui64KiSystemServiceUser, &ui64KeServiceDescriptorTable);
    if (!NT_SUCCESS(Status) || ui64KeServiceDescriptorTable == 0)
    {
        DbgPrint("[%ws::%d] Unable to resolve KeServiceDescriptorTable.\n", __FUNCTIONW__, __LINE__);
        return STATUS_NOT_FOUND;
    }

    g_Globals->ssdt = reinterpret_cast<PKSERVICE_DESCRIPTOR_TABLE>(ui64KeServiceDescriptorTable);
    g_Globals->isInitialized = true;

    return Status;
}

NTSTATUS hookSyscall(PIRP Irp)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    ULARGE_INTEGER ulBase = { 0 };

    UINT64 f_hNtFunction = 0;

    PINPUT_BUFFER pUsermodeBuffer = nullptr;

    ///
    /// Sanity check
    /// 
    if (Irp)
    {
        __try
        {
            pUsermodeBuffer = static_cast<PINPUT_BUFFER>(
                Irp->AssociatedIrp.SystemBuffer
                );

            DbgPrint("[%ws::%d] Syscall: %d\n", __FUNCTIONW__, __LINE__, pUsermodeBuffer->syscall);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            DbgPrint("[%ws::%d] Access violation occured.\n", __FUNCTIONW__, __LINE__);
            return STATUS_ACCESS_VIOLATION;
        }
    }
    else
    {
        DbgPrint("[%ws::%d] Received a null irp.\n", __FUNCTIONW__, __LINE__);
        return STATUS_IO_DEVICE_ERROR;
    }

    ///
    /// Validate the user's input
    /// 
    if (pUsermodeBuffer->syscall > g_Globals->ssdt->NumberOfServices || 
        pUsermodeBuffer->syscall < 0
        )
    {
        DbgPrint("[%ws::%d] %d is not a valid index.\n", __FUNCTIONW__, __LINE__, pUsermodeBuffer->syscall);
        return STATUS_INVALID_PARAMETER;
    }

    UINT64 ui64SyscallFunction = reinterpret_cast<UINT64>(g_Globals->ssdt->ServiceTableBase);
    ui64SyscallFunction += g_Globals->ssdt->ServiceTableBase[pUsermodeBuffer->syscall] >> 4;

    switch (pUsermodeBuffer->syscall)
    {
    case SYSCALL_INDEX::NtDeviceIoControlFileIndex:
    {
        if (!hook::isFunctionHookedByAddress(reinterpret_cast<PVOID>(ui64SyscallFunction)))
        {
            g_Globals->hooked = static_cast<PHOOKED_NT_FUNCTION>(
                ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(HOOKED_NT_FUNCTION), POOLTAG)
                );
            if (g_Globals->hooked == NULL)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;

                DbgPrint("[%ws::%d] ExAllocatePoolWithTag failed: %08x\n", __FUNCTIONW__, __LINE__, Status);
                return Status;
            }
            else
            {
                RtlSecureZeroMemory(g_Globals->hooked, sizeof(HOOKED_NT_FUNCTION));
            }

            g_Globals->hooked->address = reinterpret_cast<PVOID>(ui64SyscallFunction);
            g_Globals->hooked->index = pUsermodeBuffer->syscall;

            Status = resolve::IopXxxControlFile(
                reinterpret_cast<UINT64>(g_Globals->hooked->address),
                reinterpret_cast<PUINT64>(&g_Globals->sub.IopXxxControlFile)
            );
            if (!NT_SUCCESS(Status) || g_Globals->sub.IopXxxControlFile == NULL)
            {
                DbgPrint("[%ws::%d] Unable to resolve IopXxxControlFile.\n", __FUNCTIONW__, __LINE__);
                return STATUS_NOT_FOUND;
            }

            f_hNtFunction = reinterpret_cast<UINT64>(NtDICF);

            RtlCopyMemory(
                g_Globals->hooked->original,
                g_Globals->hooked->address,
                sizeof(g_Globals->hooked->original)
            );

            RtlCopyMemory(hookedcode + 2, &f_hNtFunction, sizeof(f_hNtFunction));

            Status = detour::hook(
                reinterpret_cast<PVOID>(ui64SyscallFunction),
                hookedcode,
                sizeof(hookedcode)
            );
            if (!NT_SUCCESS(Status))
            {
                return Status;
            }
            else
            {
                g_Globals->hooked->isHooked = true;
            }

            ExInterlockedInsertTailList(
                &g_Globals->ListHead,
                &g_Globals->hooked->entry,
                &g_Globals->kInterlockedSpinLock
            );
        }
        break;
    }
    case SYSCALL_INDEX::NtCreateFileIndex:
    {
        if (!hook::isFunctionHookedByAddress(reinterpret_cast<PVOID>(ui64SyscallFunction)))
        {
            g_Globals->hooked = static_cast<PHOOKED_NT_FUNCTION>(
                ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(HOOKED_NT_FUNCTION), POOLTAG)
                );
            if (g_Globals->hooked == NULL)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;

                DbgPrint("[%ws::%d] ExAllocatePoolWithTag failed: %08x\n", __FUNCTIONW__, __LINE__, Status);
                return Status;
            }
            else
            {
                RtlSecureZeroMemory(g_Globals->hooked, sizeof(HOOKED_NT_FUNCTION));
            }

            g_Globals->hooked->address = reinterpret_cast<PVOID>(ui64SyscallFunction);
            g_Globals->hooked->index = pUsermodeBuffer->syscall;

            Status = resolve::IopCreateFile(
                reinterpret_cast<UINT64>(g_Globals->hooked->address),
                reinterpret_cast<PUINT64>(&g_Globals->sub.IopCreateFile)
            );
            if (!NT_SUCCESS(Status) || g_Globals->sub.IopCreateFile == NULL)
            {
                DbgPrint("[%ws::%d] Unable to resolve IopCreateFile.\n", __FUNCTIONW__, __LINE__);
                return STATUS_NOT_FOUND;
            }

            f_hNtFunction = reinterpret_cast<UINT64>(NtCF);

            RtlCopyMemory(
                g_Globals->hooked->original,
                g_Globals->hooked->address,
                sizeof(g_Globals->hooked->original)
            );

            RtlCopyMemory(hookedcode + 2, &f_hNtFunction, sizeof(f_hNtFunction));

            Status = detour::hook(
                reinterpret_cast<PVOID>(ui64SyscallFunction),
                hookedcode,
                sizeof(hookedcode)
            );
            if (!NT_SUCCESS(Status))
            {
                return Status;
            }
            else
            {
                g_Globals->hooked->isHooked = true;
            }

            ExInterlockedInsertTailList(
                &g_Globals->ListHead,
                &g_Globals->hooked->entry,
                &g_Globals->kInterlockedSpinLock
            );
        }
        break;
    }
    default:
        DbgPrint("[%ws::%d] %p is not implemented.\n", __FUNCTIONW__, __LINE__, g_Globals->hooked->address);
        return STATUS_NOT_IMPLEMENTED;
    }

    return STATUS_SUCCESS;
}

NTSTATUS isSyscallHooked(PIRP Irp, PULONG Information)
{
    PINPUT_BUFFER pUsermodeBuffer = nullptr;

    ///
    /// Sanity check
    /// 
    if (Irp)
    {
        __try
        {
            pUsermodeBuffer = static_cast<PINPUT_BUFFER>(
                Irp->AssociatedIrp.SystemBuffer
                );

            DbgPrint("[%ws::%d] Syscall: %d\n", __FUNCTIONW__, __LINE__, pUsermodeBuffer->syscall);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            DbgPrint("[%ws::%d] Access violation occured.\n", __FUNCTIONW__, __LINE__);
            return STATUS_ACCESS_VIOLATION;
        }
    }
    else
    {
        DbgPrint("[%ws::%d] Received a null irp.\n", __FUNCTIONW__, __LINE__);
        return STATUS_IO_DEVICE_ERROR;
    }

    ///
    /// Validate the user's input
    /// 
    if (pUsermodeBuffer->syscall > g_Globals->ssdt->NumberOfServices ||
        pUsermodeBuffer->syscall < 0
        )
    {
        DbgPrint("[%ws::%d] %d is not a valid index.\n", __FUNCTIONW__, __LINE__, pUsermodeBuffer->syscall);
        return STATUS_INVALID_PARAMETER;
    }

    DbgPrint("Status: %d\n", pUsermodeBuffer->status);

    pUsermodeBuffer->status = hook::isFunctionHookedByIndex(pUsermodeBuffer->syscall);

    DbgPrint("Status: %d\n", pUsermodeBuffer->status);

    __try
    {
        RtlCopyMemory(
            Irp->AssociatedIrp.SystemBuffer,
            pUsermodeBuffer,
            sizeof(INPUT_BUFFER)
        );

        *Information = sizeof(INPUT_BUFFER);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return STATUS_ACCESS_VIOLATION;
    }
    return STATUS_SUCCESS;
}

NTSTATUS unhookSyscall(PIRP Irp)
{
    PINPUT_BUFFER pUsermodeBuffer = nullptr;

    ///
    /// Sanity check
    /// 
    if (Irp)
    {
        __try
        {
            pUsermodeBuffer = static_cast<PINPUT_BUFFER>(
                Irp->AssociatedIrp.SystemBuffer
                );

            DbgPrint("[%ws::%d] Syscall: %d\n", __FUNCTIONW__, __LINE__, pUsermodeBuffer->syscall);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            DbgPrint("[%ws::%d] Access violation occured.\n", __FUNCTIONW__, __LINE__);
            return STATUS_ACCESS_VIOLATION;
        }
    }
    else
    {
        DbgPrint("[%ws::%d] Received a null irp.\n", __FUNCTIONW__, __LINE__);
        return STATUS_IO_DEVICE_ERROR;
    }

    ///
    /// Validate the user's input
    /// 
    if (pUsermodeBuffer->syscall > g_Globals->ssdt->NumberOfServices ||
        pUsermodeBuffer->syscall < 0
        )
    {
        DbgPrint("[%ws::%d] %d is not a valid index.\n", __FUNCTIONW__, __LINE__, pUsermodeBuffer->syscall);
        return STATUS_INVALID_PARAMETER;
    }
    DbgPrint("[%ws::%d] unhooking with function id: %d\n", __FUNCTIONW__, __LINE__, pUsermodeBuffer->syscall);

    return hook::unhookFunction(pUsermodeBuffer->syscall);
}
