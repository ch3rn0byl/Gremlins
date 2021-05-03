#pragma once
#include <ntddk.h>

/// <summary>
/// The detour namespace is responsible for mapping a 
/// function into userspace and then overwriting that
/// region of memory. 
/// </summary>
namespace detour
{
	_Success_(return >= 0)
	NTSTATUS hook(
		_In_ PVOID BaseAddress,
		_In_ PUINT8 patch,
		_In_ size_t szSize
	);

	_Success_(return >= 0)
	NTSTATUS unhook(
		_In_ PVOID BaseAddress,
		_In_ PUINT8 Source,
		_In_ UINT8 szSize
	);
}


/// EOF