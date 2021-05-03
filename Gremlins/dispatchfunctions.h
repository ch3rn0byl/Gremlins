#pragma once
#include "ntdeviceiocontrolfile.h"
#include "ntcreatefile.h"
#include "resolve.h"
#include "detour.h"
#include "typedefs.h"
#include "hook.h"

#include <intrin.h>

/// <summary>
/// All the functions that the dispatch table will 
/// callout to. These functions will be responsible for:
///     1. Checking if the drivers initialized.
///     2. Initialize the driver.
///     3. Hook the syscall it receives.
///     4. Unhook the syscall it receives. 
/// <summary>
_Success_(return >= 0)
NTSTATUS isInitialized(
	_In_ PIRP Irp,
	_Out_ PULONG Information
);

_Success_(return >= 0)
NTSTATUS initialize(
	_In_ PIRP Irp
);

_Success_(return >= 0)
NTSTATUS hookSyscall(
	_In_ PIRP Irp
);

_Success_(return >= 0)
NTSTATUS isSyscallHooked(
	_In_ PIRP Irp,
	_Out_ PULONG Information
);

_Success_(return >= 0)
NTSTATUS unhookSyscall(
	_In_ PIRP Irp
);


/// EOF