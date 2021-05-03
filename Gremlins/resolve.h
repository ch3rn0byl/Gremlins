#pragma once
#include <ntddk.h>

/// <summary>
/// The resolve namespace will be responsible for dynamically
/// resolving the System's Service Descriptor Table and for 
/// resolving the some of the sub functions used to forward 
/// requests from NtDeviceIoControlFile and NtCreateFile. 
/// </summary>
namespace resolve
{
	_Success_(return >= 0)
	NTSTATUS KiSystemServiceUser(
		_In_ UINT64 address,
		_Out_ _Ret_maybenull_ PUINT64 result
	);

	_Success_(return >= 0)
	NTSTATUS KeServiceDescriptorTable(
		_In_ UINT64 address,
		_Out_ _Ret_maybenull_ PUINT64 result
	);

	_Success_(return >= 0)
	NTSTATUS IopXxxControlFile(
		_In_ UINT64 address,
		_Out_ _Ret_maybenull_ PUINT64 result
	);

	_Success_(return >= 0)
	NTSTATUS IopCreateFile(
		_In_ UINT64 address,
		_Out_ _Ret_maybenull_ PUINT64 result
	);
}


/// EOF