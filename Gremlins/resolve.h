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
	_Ret_maybenull_
	NTSTATUS KiSystemServiceUser(
		_In_ UINT64 address,
		_Out_ PUINT64 result
	);

	_Ret_maybenull_
	NTSTATUS KeServiceDescriptorTable(
		_In_ UINT64 address,
		_Out_ PUINT64 result
	);

	_Ret_maybenull_
	NTSTATUS IopXxxControlFile(
		_In_ UINT64 address,
		_Out_ PUINT64 result
	);

	_Ret_maybenull_
	NTSTATUS IopCreateFile(
		_In_ UINT64 address,
		_Out_ PUINT64 result
	);
}


/// EOF