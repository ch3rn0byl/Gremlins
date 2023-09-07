#include <ntddk.h>
#include "typesndefs.h"
#include "dispatchfunctions.h"
#include "AuxWrapper.h"

UNICODE_STRING g_DeviceName = RTL_CONSTANT_STRING(L"\\Device\\Gremlins");
UNICODE_STRING g_SymbolicName = RTL_CONSTANT_STRING(L"\\??\\Gremlins");

PGLOBALS g_Globals = nullptr;

void banner()
{
	//
	// Using LOG_ERR because it prints regardless of what flag you specify.
	//
	LOG_ERR("\n                                      @                                      \n", NULL);
	LOG_ERR("   ,***((****((((*               @@@@@@@@@@@               ,****,,,,**,,,.   \n", NULL);
	LOG_ERR("****(############**(((         @@@@@@@@@@@@@@@        .***,,((((((((((((/,,,,\n", NULL);
	LOG_ERR("    #################************,@@@@@@@@@,,,,,,,,,,,,*(((((((((((((((((    \n", NULL);
	LOG_ERR("     ###################*************@@@,,,**,,,,*,*,(((((((((((((((((((     \n", NULL);
	LOG_ERR("      #################******(/*******,,*,*,,,//,,,,*,(((((((((((((((((      \n", NULL);
	LOG_ERR("       ################*****#***/##***.,*,((/***(,,*,,((((((((((((((((       \n", NULL);
	LOG_ERR("        ###############(*****##/##*,,*,,..,((*((,,,,,/(((((((((((((((        \n", NULL);
	LOG_ERR("           ############****,**********,,,,,,,,,,,,,,,,((((((((((((           \n", NULL);
	LOG_ERR("                  #####*******.%******,,,,,,,%.,,,,,,*(((((                  \n", NULL);
	LOG_ERR("                       ,,,******....*%*%**.(.,,,,,,...                       \n", NULL);
	LOG_ERR("                   ,,,,,,,,,,*********,,,,,,,,,,..........                   \n", NULL);
	LOG_ERR("                  ,,,,,,,,,/(/,,**,***,,,,.,,,.*/*.........                  \n", NULL);
	LOG_ERR("                 .,,,,,,,,,,,#####*,,,,...,(((((...........                  \n", NULL);
	LOG_ERR("                ,,,,,,,,,,,,#######***.,,,(((((((............                \n", NULL);
	LOG_ERR("                      , ,,,,##########(((((((((((.... .                      \n", NULL);
	LOG_ERR("                                ########(((((*                               \n", NULL);
	LOG_ERR("                         Gremlins: A Syscall Fuzzer                          \n\n", NULL);
}

_Function_class_(DRIVER_DISPATCH)
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
DriverDispatchRoutine(
	_In_ PDEVICE_OBJECT,
	_In_ PIRP Irp
)
{
	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

	ULONG Information = NULL;
	UINT32 IoControlCode = NULL;

	PIO_STACK_LOCATION IoStackLocation = nullptr;
	PINPUT_BUFFER InputBuffer = nullptr;

	IoStackLocation = IoGetCurrentIrpStackLocation(Irp);
	IoControlCode = IoStackLocation->Parameters.DeviceIoControl.IoControlCode;

	//
	// METHOD_BUFFERED is being used; therefore, access dat thang via SystemBuffer.
	//
	InputBuffer = static_cast<PINPUT_BUFFER>(
		Irp->AssociatedIrp.SystemBuffer
		);

	LOG_TRACE("[%ws::%d] Reached dispatch with IoControlCode: 0x%X\n", __FUNCTIONW__, __LINE__, IoControlCode);
	switch (IoControlCode)
	{
	case IsInitializedIoctl:
		LOG_TRACE("[%ws::%d] checking if initialize module.\n", __FUNCTIONW__, __LINE__);
		Status = IsModuleInitialized(InputBuffer, &Information);
		break;
	case InitializeIoctl:
		LOG_TRACE("[%ws::%d] Goign to initialize module.\n", __FUNCTIONW__, __LINE__);
		Status = InitializeModule();
		break;
	case IsHookedIoctl:
		Status = IsSyscallHooked(InputBuffer, &Information);
		break;
	case HookIoctl:
		Status = HookSyscall(InputBuffer);
		break;
	case UnhookIoctl:
		Status = UnhookSyscall(InputBuffer);
		break;
	case ExcludeDriverIoctl:
		Status = ExcludeDriver(InputBuffer);
		break;
	case ImageAnalysisIoctl:
		Status = AnalyzeImage(InputBuffer);
		break;
	default:
		break;
	}

	Irp->IoStatus.Status = Status;
	Irp->IoStatus.Information = Information;

	IofCompleteRequest(Irp, IO_NO_INCREMENT);

	return Status;
}

_Function_class_(DRIVER_DISPATCH)
_Dispatch_type_(IRP_MJ_CREATE)
_Dispatch_type_(IRP_MJ_CLOSE)
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
DriverCreateClose(
	_In_ PDEVICE_OBJECT,
	_In_ PIRP Irp
)
{
	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = STATUS_SUCCESS;

	IofCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

_Function_class_(DRIVER_UNLOAD)
_IRQL_requires_(PASSIVE_LEVEL)
VOID
DriverUnload(
	_In_ PDRIVER_OBJECT DriverObject
)
{
	//
	// Iterate through all the linked lists and free the memory.
	//
	//hook::cleanup();
	//exclusions::cleanup();

	//
	// If g_Globals isn't empty, clear it and release the pool.
	//
	if (g_Globals != nullptr)
	{
		RtlSecureZeroMemory(g_Globals, sizeof(g_Globals));
#ifdef DBG
		ExFreePoolWithTag(g_Globals, POOLTAG_DBG);
#else
		ExFreePoolWithTag(g_Globals, POOLTAG);
#endif // DBG
		g_Globals = nullptr;
	}

	IoDeleteSymbolicLink(&g_SymbolicName);
	IoDeleteDevice(DriverObject->DeviceObject);

	LOG_TRACE("[%ws::%d] Completed successfully.\n", __FUNCTIONW__, __LINE__);
}

_Function_class_(DRIVER_INITIALIZE)
_IRQL_requires_(PASSIVE_LEVEL)
EXTERN_C
NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING
)
{
	PDEVICE_OBJECT DeviceObject = nullptr;

	NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;

	banner();

	//
	// Allocate all the global variables. The buffer is initialized to zero.
	// 
	g_Globals = static_cast<PGLOBALS>(
#ifdef DBG
		ExAllocatePool2(POOL_FLAG_NON_PAGED | POOL_FLAG_SPECIAL_POOL, sizeof(GLOBALS), POOLTAG_DBG)
#else
		ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(GLOBALS), POOLTAG)
#endif // DBG
		);
	if (g_Globals == NULL)
	{
		LOG_ERR("[%ws::%d] Failed with 0x%08x.\n", __FUNCTIONW__, __LINE__, Status);
		return Status;
	}

	LOG_TRACE("[%ws::%d] g_Globals allocated at %p.\n", __FUNCTIONW__, __LINE__, g_Globals);

	// 
	// Initialize the spinlocks, mutex, and list head. 
	// 
	// The kSpinLock is used for the ListHead.
	// The fMutex is used for printing information to the debugger.
	//
	KeInitializeSpinLock(&g_Globals->kSpinLock);
	KeInitializeSpinLock(&g_Globals->kInterlockedSpinLock);

	ExInitializeFastMutex(&g_Globals->fMutex);

	InitializeListHead(&g_Globals->HookedListHead);
	InitializeListHead(&g_Globals->ExcludeDriverListHead);
	InitializeListHead(&g_Globals->AnalyzeKernelImageListHead);

	// TODO: Register a process callback to detect processes that contain "gremlins" or if the process is blacklisted.
	//Status = PsSetCreateProcessNotifyRoutine()

	Status = IoCreateDevice(
		DriverObject,
		NULL,
		&g_DeviceName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		TRUE,
		&DeviceObject
	);
	if (!NT_SUCCESS(Status))
	{
		LOG_ERR("[%ws::%d] Failed with 0x%08x.\n", __FUNCTIONW__, __LINE__, Status);

		if (DeviceObject)
		{
			IoDeleteDevice(DeviceObject);
		}
		return Status;
	}

	//
	// Create the symbolic link to interact with the driver.
	//
	Status = IoCreateSymbolicLink(&g_SymbolicName, &g_DeviceName);
	if (!NT_SUCCESS(Status))
	{
		LOG_ERR("[%ws::%d] Failed with 0x%08x.\n", __FUNCTIONW__, __LINE__, Status);

		IoDeleteDevice(DeviceObject);
		return Status;
	}

	//
	// We are using METHOD BUFFERED. Reflect that in the flags.
	//
	DeviceObject->Flags |= DO_BUFFERED_IO;

	DriverObject->DriverUnload = DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDispatchRoutine;

	LOG_TRACE("[%ws::%d] Completed successfully.\n", __FUNCTIONW__, __LINE__);

	return Status;
}


/// EOF