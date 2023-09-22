#include "ProcessCallbacks.h"

typedef struct _KAPC_STATE {
	LIST_ENTRY ApcListHead[MaximumMode];
	struct _KPROCESS* Process;
	union {
		UCHAR InProgressFlags;
		struct {
			BOOLEAN KernelApcInProgress : 1;
			BOOLEAN SpecialApcInProgress : 1;
		};
	};

	BOOLEAN KernelApcPending;
	union {
		BOOLEAN UserApcPendingAll;
		struct {
			BOOLEAN SpecialUserApcPending : 1;
			BOOLEAN UserApcPending : 1;
		};
	};
} KAPC_STATE, * PKAPC_STATE, * PRKAPC_STATE;

EXTERN_C_START
UCHAR* PsGetProcessImageFileName(PEPROCESS Process);
NTSTATUS PsLookupProcessByProcessId(HANDLE ProcessId, PEPROCESS* Process);
PPEB PsGetProcessPeb(PEPROCESS Process);

NTKERNELAPI
VOID
KeStackAttachProcess(
	_Inout_ PRKPROCESS PROCESS,
	_Out_ PRKAPC_STATE ApcState
);



_IRQL_requires_max_(APC_LEVEL)
NTKERNELAPI
VOID
KeUnstackDetachProcess(
	_In_ PRKAPC_STATE ApcState
);

EXTERN_C_END

typedef struct _CURDIR {
	UNICODE_STRING DosPath;
	PVOID Handle;
} CURDIR, * PCURDIR;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
	UINT32 MaximumLength;
	UINT32 Length;
	UINT32 Flags;
	UINT32 DebugFlags;
	PVOID ConsoleHandle;
	UINT32 ConsoleFlags;
	PVOID StandardInput;
	PVOID StandardOutput;
	PVOID StandardError;
	CURDIR CurrentDirectory;
	UNICODE_STRING DllPath;
	UNICODE_STRING ImagePathName;
	UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

_Use_decl_annotations_
void PCreateProcessNotifyRoutine(
	HANDLE ParentId, 
	HANDLE ProcessId,
	BOOLEAN Create
)
{
	UNREFERENCED_PARAMETER(ParentId);
	
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PEPROCESS Eproc = nullptr;

	if (Create)
	{
		LOG_TRACE("[%ws::%d] created %d\n", __FUNCTIONW__, __LINE__, ProcessId);
		Status = PsLookupProcessByProcessId(ProcessId, &Eproc);
		if (NT_SUCCESS(Status))
		{
			PPEB ProcessPeb = PsGetProcessPeb(Eproc);
			if (ProcessPeb)
			{
				KAPC_STATE ApcState;

				KeStackAttachProcess((PRKPROCESS)Eproc, &ApcState);

				PRTL_USER_PROCESS_PARAMETERS UserProcessParams = *reinterpret_cast<PRTL_USER_PROCESS_PARAMETERS*>(
					reinterpret_cast<PUINT8>(ProcessPeb) + 0x20
					);

				if (wcsstr(UserProcessParams->ImagePathName.Buffer, L"TRACE"))
				{
					LOG_TRACE("[%ws::%d] Found a process to trace: %wZ\n", __FUNCTIONW__, __LINE__, UserProcessParams->ImagePathName);
				}

				KeUnstackDetachProcess(&ApcState);
			}
			else
			{
				LOG_ERR("[%ws::%d] Failed to get peb.\n", __FUNCTIONW__, __LINE__);
			}
		}
		else
		{
			LOG_ERR("[%ws::%d] Failed with %08x\n", __FUNCTIONW__, __LINE__, Status);
		}
	}
	else
	{
		LOG_TRACE("[%ws::%d] terminated.\n", __FUNCTIONW__, __LINE__);
	}

	if (Eproc)
	{
		ObDereferenceObject(Eproc);
	}
}


/// EOF