#pragma once
#include <ntddk.h>
#include "typesndefs.h"

void PCreateProcessNotifyRoutine(
	_In_ HANDLE ParentId,
	_In_ HANDLE ProcessId,
	_In_ BOOLEAN Create
);


/// EOF