#include "typedefs.h"
#include "dispatchfunctions.h"

UNICODE_STRING g_DeviceName = RTL_CONSTANT_STRING(L"\\Device\\Gremlins");
UNICODE_STRING g_SymbolicName = RTL_CONSTANT_STRING(L"\\??\\Gremlins");

PGLOBALS g_Globals = nullptr;

void banner()
{
    DbgPrint("                                      @                                      \n");
    DbgPrint("   ,***((****((((*               @@@@@@@@@@@               ,****,,,,**,,,.   \n");
    DbgPrint("****(############**(((         @@@@@@@@@@@@@@@        .***,,((((((((((((/,,,,\n");
    DbgPrint("    #################************,@@@@@@@@@,,,,,,,,,,,,*(((((((((((((((((    \n");
    DbgPrint("     ###################*************@@@,,,**,,,,*,*,(((((((((((((((((((     \n");
    DbgPrint("      #################******(/*******,,*,*,,,//,,,,*,(((((((((((((((((      \n");
    DbgPrint("       ################*****#***/##***.,*,((/***(,,*,,((((((((((((((((       \n");
    DbgPrint("        ###############(*****##/##*,,*,,..,((*((,,,,,/(((((((((((((((        \n");
    DbgPrint("           ############****,**********,,,,,,,,,,,,,,,,((((((((((((           \n");
    DbgPrint("                  #####*******.%******,,,,,,,%.,,,,,,*(((((                  \n");
    DbgPrint("                       ,,,******....*%*%**.(.,,,,,,...                       \n");
    DbgPrint("                   ,,,,,,,,,,*********,,,,,,,,,,..........                   \n");
    DbgPrint("                  ,,,,,,,,,/(/,,**,***,,,,.,,,.*/*.........                  \n");
    DbgPrint("                 .,,,,,,,,,,,#####*,,,,...,(((((...........                  \n");
    DbgPrint("                ,,,,,,,,,,,,#######***.,,,(((((((............                \n");
    DbgPrint("                      , ,,,,##########(((((((((((.... .                      \n");
    DbgPrint("                                ########(((((*                               \n");
}

NTSTATUS DriverDispatchTable(
    _In_ PDEVICE_OBJECT,
    _In_ PIRP Irp
)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    PIO_STACK_LOCATION pCurrentIrpLocation = IoGetCurrentIrpStackLocation(Irp);
    ULONG Information = 0;

    UINT32 ui32IoControlCode = pCurrentIrpLocation->Parameters.DeviceIoControl.IoControlCode;

    switch (ui32IoControlCode)
    {
    case IOCTL_FUNCTION::IsInitialized:
        Status = isInitialized(Irp, &Information);
        break;
    case IOCTL_FUNCTION::Initialize:
        Status = initialize(Irp);
        break;
    case IOCTL_FUNCTION::IsHooked:
        Status = isSyscallHooked(Irp, &Information);
        break;
    case IOCTL_FUNCTION::Hook:
        Status = hookSyscall(Irp);
        break;
    case IOCTL_FUNCTION::Unhook:
        Status = unhookSyscall(Irp);
        break;
    default:
        break;
    }

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = Information;

    IofCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}

NTSTATUS DriverCreateClose(
    _In_ PDEVICE_OBJECT,
    _In_ PIRP Irp
)
{
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;

    IofCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

void DriverUnload(
    _In_ PDRIVER_OBJECT DriverObject
)
{
    if (!IsListEmpty(&g_Globals->ListHead))
    {
        hook::cleanup();
    }

    IoDeleteSymbolicLink(&g_SymbolicName);
    IoDeleteDevice(DriverObject->DeviceObject);
}

EXTERN_C NTSTATUS DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING
)
{
    NTSTATUS Status = STATUS_FAILED_DRIVER_ENTRY;
    PDEVICE_OBJECT DeviceObject = nullptr;

    banner();

    g_Globals = static_cast<PGLOBALS>(
        ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(GLOBALS), POOLTAG)
        );
    if (g_Globals == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;

#ifndef _DEBUG
        DbgPrint("[%ws::%d] ExAllocatePoolWithTag failed: %08x\n", __FUNCTIONW__, __LINE__, Status);
#endif 
        return Status;
    }
    else
    {
        RtlSecureZeroMemory(g_Globals, sizeof(GLOBALS));
    }

    KeInitializeSpinLock(&g_Globals->kSpinLock);
    KeInitializeSpinLock(&g_Globals->kInterlockedSpinLock);

    ExInitializeFastMutex(&g_Globals->fMutex);
    InitializeListHead(&g_Globals->ListHead);

    Status = IoCreateDevice(DriverObject, 0, &g_DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, TRUE, &DeviceObject);
    if (!NT_SUCCESS(Status))
    {
#ifndef _DEBUG
        DbgPrint("[%ws::%d] IoCreateDevice failed: %08x\n", __FUNCTIONW__, __LINE__, Status);
#endif 
        if (DeviceObject)
        {
            IoDeleteDevice(DeviceObject);
        }
        return Status;
    }

    Status = IoCreateSymbolicLink(&g_SymbolicName, &g_DeviceName);
    if (!NT_SUCCESS(Status))
    {
#ifndef _DEBUG
        DbgPrint("[%ws::%d] IoCreateSymbolicLink failed: %08x\n", __FUNCTIONW__, __LINE__, Status);
#endif 
        IoDeleteDevice(DeviceObject);
        return Status;
    }

    DeviceObject->Flags |= DO_BUFFERED_IO;

    DriverObject->DriverUnload = DriverUnload;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDispatchTable;

#ifndef _DEBUG
    DbgPrint("[%ws::%d] Completed successfully.\n", __FUNCTIONW__, __LINE__, Status);
#endif 
    return Status;
}


/// EOF