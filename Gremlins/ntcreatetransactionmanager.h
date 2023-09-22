#pragma once
#include "typesndefs.h"

EXTERN_C
NTSTATUS
fn_hNtCreateTransactionManager(
	_Out_ PHANDLE TmHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_opt_ PUNICODE_STRING LogFileName,
	_In_opt_ ULONG CreateOptions,
	_In_opt_ ULONG CommitStrength
);


/// EOF