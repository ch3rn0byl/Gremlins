#pragma once
#include <intrin.h>

#include "typesndefs.h"
#include "resolve.h"
#include "hook.h"
#include "ntdeviceiocontrolfile.h"

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


/// EOF