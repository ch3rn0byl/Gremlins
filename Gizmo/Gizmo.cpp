#include "Gizmo.h"

Gizmo::Gizmo() :
    m_pNtUndoc(std::make_unique<NtUndoc>()),
    m_wszServiceName(L"\\\\.\\Gremlins"),
    m_pError(nullptr)
{
    INPUT_BUFFER lpInputBuffer = {};

    bool bStatus = init(m_wszServiceName);
    if (!bStatus)
    {
        m_pError = std::make_unique<ErrorHandler>(GetLastError());
        throw std::runtime_error(m_pError->GetLastErrorAsStringA());
    }

    //
    // Check if Gremlins has been initialized. If it hasn't, do the damn thang.
    //
    bStatus = SendIoControlRequest(
        IsInitialized, 
        &lpInputBuffer, 
        sizeof(lpInputBuffer), 
        &lpInputBuffer, 
        sizeof(lpInputBuffer)
    );
    if (bStatus)
    {
        //
        // If this is false, this indicates Gremlins is NOT initialized. Let's
        // start it up.
        //
        if (!lpInputBuffer.status)
        {
            bStatus = SendIoControlRequest(Initialize);
            if (!bStatus)
            {
                m_pError = std::make_unique<ErrorHandler>(GetLastError());
                throw std::runtime_error(m_pError->GetLastErrorAsStringA());
            }
        }
    }
    else
    {
        m_pError = std::make_unique<ErrorHandler>(GetLastError());
        throw std::runtime_error(m_pError->GetLastErrorAsStringA());
    }
}

Gizmo::~Gizmo()
{
}

BOOL Gizmo::IsBeingKernelDebugged()
{
    std::unique_ptr<UINT8[]> pSysKernBytes = nullptr;
    PSYSTEM_KERNEL_DEBUGGER_INFORMATION pIsKdAttached = nullptr;

    ULONG dwSysInfoLength = 0;

    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    do
    {
        //
        // First time around will be NULL. This is to dynamically resolve the size needed
        // for the buffer.
        //
        pSysKernBytes = std::make_unique<UINT8[]>(dwSysInfoLength);

        Status = m_pNtUndoc->NtQuerySystemInformation(
            SystemKernelDebuggerInformation,
            pSysKernBytes.get(),
            dwSysInfoLength,
            &dwSysInfoLength
        );
        if (!NT_SUCCESS(Status) && Status == STATUS_INFO_LENGTH_MISMATCH)
        {
            pSysKernBytes.reset();
        }
        else
        {
            break;
        }
    } while (Status != STATUS_SUCCESS);

    pIsKdAttached = reinterpret_cast<PSYSTEM_KERNEL_DEBUGGER_INFORMATION>(pSysKernBytes.get());
    if (!pIsKdAttached->DebuggerEnabled || pIsKdAttached->DebuggerNotPresent)
    {
        return FALSE;
    }

    return TRUE;
}

_Use_decl_annotations_
bool 
Gizmo::IsSyscallHooked(
    LPCSTR lpProcName, 
    PINPUT_BUFFER lpInputBuffer
)
{
    //
    // Resolve the syscall first.
    //
    bool bStatus = ResolveSyscall(lpProcName, &lpInputBuffer->syscall);
    if (!bStatus)
    {
        return bStatus;
    }

    //
    // Query Gremlins to check if the function is hooked or not.
    //
    bStatus = SendIoControlRequest(
        IsHooked, 
        lpInputBuffer, 
        sizeof(INPUT_BUFFER), 
        lpInputBuffer, 
        sizeof(INPUT_BUFFER)
    );
    if (!bStatus)
    {
        m_pError = std::make_unique<ErrorHandler>(GetLastError());
    }

    return bStatus;
}

_Use_decl_annotations_
bool 
Gizmo::HookSyscall(
    PINPUT_BUFFER lpInputBuffer
)
{
    bool bStatus = SendIoControlRequest(Hook, lpInputBuffer, sizeof(INPUT_BUFFER));
    if (!bStatus)
    {
        m_pError = std::make_unique<ErrorHandler>(GetLastError());
    }

    return bStatus;
}

_Use_decl_annotations_
bool 
Gizmo::UnhookSyscall(
    PINPUT_BUFFER lpInputBuffer
)
{
    bool bStatus = SendIoControlRequest(Unhook, lpInputBuffer, sizeof(INPUT_BUFFER));
    if (!bStatus)
    {
        m_pError = std::make_unique<ErrorHandler>(GetLastError());
    }

    return bStatus;
}

std::wstring Gizmo::what()
{
    return m_pError->GetLastErrorAsStringW();
}

_Use_decl_annotations_
bool 
Gizmo::ResolveSyscall(
    LPCSTR lpProcName, 
    PUINT16 SyscallValue
)
{
    HMODULE hModule = NULL;
    FARPROC fpProcName = nullptr;

    bool bStatus = false;

    size_t length = 0;

    //
    // Grab a handle to ntdll because we're going to pull the export via 
    // GetProcAddress.
    //
    hModule = GetModuleHandle(L"ntdll.dll");
    if (hModule != NULL)
    {
        fpProcName = GetProcAddress(hModule, lpProcName);
        if (fpProcName != nullptr)
        {
            //
            // Once the function is located, the bytes should match the Syscall stub
            // which consists of the following:
            //   mov r10, rcx
            //   mov eax, some_number
            // 
            // If the function does not start with that, i.e. a jmp (0xe9), chances 
            // are AV exists on the system. Set the error accordingly and exit.
            //
            if (*reinterpret_cast<PUINT8>(fpProcName) != 0xe9)
            {

                length = RtlCompareMemory(fpProcName, SyscallSig, sizeof(SyscallSig));
                if (length == sizeof(SyscallSig))
                {
                    *SyscallValue = *reinterpret_cast<PUINT8>(
                        reinterpret_cast<PUINT8>(fpProcName) + sizeof(SyscallSig)
                        );

                    bStatus = true;
                }
                else
                {
                    SetLastError(ERROR_MOD_NOT_FOUND);
                    m_pError = std::make_unique<ErrorHandler>(GetLastError());
                }
            }
            else
            {
                SetLastError(ERROR_INVALID_MODULETYPE);
                m_pError = std::make_unique<ErrorHandler>(GetLastError());
            }
        }
        else
        {
            m_pError = std::make_unique<ErrorHandler>(GetLastError());
        }
    }
    else
    {
        m_pError = std::make_unique<ErrorHandler>(GetLastError());
    }

    if (hModule != NULL)
    {
        FreeLibrary(hModule);
        hModule = NULL;
    }

    if (fpProcName != NULL)
    {
        fpProcName = nullptr;
    }

    return bStatus;
}


/// EOF