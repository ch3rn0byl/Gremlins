#pragma once
#include <intrin.h>

#include "typesndefs.h"
#include "resolve.h"
#include "hook.h"
#include "exclusions.h"
#include "ntdeviceiocontrolfile.h"
#include "ntcreatefile.h"

NTSTATUS
IsModuleInitialized(
	_In_ PINPUT_BUFFER& InputBuffer,
	_Out_ PULONG Information
);

NTSTATUS
InitializeModule();

NTSTATUS
IsSyscallHooked(
	_In_ PINPUT_BUFFER& InputBuffer,
	_Out_ PULONG Information
);

NTSTATUS
HookSyscall(
	_In_ PINPUT_BUFFER& InputBuffer
);

NTSTATUS
UnhookSyscall(
	_In_ PINPUT_BUFFER InputBuffer
);

NTSTATUS
ExcludeDriver(
	_In_ PVOID InputBuffer
);

NTSTATUS
AnalyzeImage(
	_In_ PVOID InputBuffer
);


/// EOF