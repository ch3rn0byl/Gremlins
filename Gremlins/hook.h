#pragma once
#include "typedefs.h"
#include "detour.h"

/// <summary>
/// The hook namespace is responsible for checking if a function
/// is hooked and for manipulating the LIST_ENTRY.
/// </summary>
namespace hook
{
	_Success_(return >= 0)
	bool isFunctionHookedByAddress(
		_In_ PVOID address
	);

	_Success_(return >= 0)
	bool isFunctionHookedByIndex(
		_In_ UINT16 index
	);

	_Success_(return >= 0)
	NTSTATUS unhookFunction(
		_In_ UINT16 index
	);

	void cleanup();
}


/// EOF 