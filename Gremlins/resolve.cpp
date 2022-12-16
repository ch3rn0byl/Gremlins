#include "resolve.h"

/// <summary>
/// Resolves nt!KiSystemServiceUser by searching for the ret instruction:
///     jmp nt!KiSystemServiceUser (fffff806`4a007500)
///     ret
/// The jmp instruction directly before is what I am after because of it jumping 
/// directly to nt!KiSystemServiceUser.
/// </summary>
/// <param name="address"></param>
/// <param name="result"></param>
/// <returns>STATUS_SUCCESS/STATUS_NOT_FOUND</returns>
_Use_decl_annotations_
NTSTATUS resolve::KiSystemServiceUser(UINT64 address, PUINT64 result)
{
	UINT8 value = 0;

	do
	{
		value = *reinterpret_cast<PUINT8>(address);
		if (value == 0xc3)
		{
			UINT64 mask = 0xffffffff00000000;
			UINT32 offset = 0;

			RtlCopyMemory(&offset, reinterpret_cast<PVOID>(address - 4), sizeof(offset));

			mask |= offset;

			*result = address + mask;
			return STATUS_SUCCESS;
		}
		address++;
	} while (true);

	return STATUS_NOT_FOUND;
}

/// <summary>
/// Resolves nt!KeServiceDescriptorTable by searching for the following lea instruction:
///     lea     r10,[nt!KeServiceDescriptorTable (fffff806`4aa018c0)]
/// This works because their is only one instance of it in the entire nt!KiSystemServiceUser.
/// </summary>
/// <param name="address"></param>
/// <param name="result"></param>
/// <returns>STATUS_SUCCESS/STATUS_NOT_FOUND</returns>
_Use_decl_annotations_
NTSTATUS resolve::KeServiceDescriptorTable(UINT64 address, PUINT64 result)
{
	UINT8 SsdtStub[] = {
		0x4c, 0x8d, 0x15
	};

	do
	{
		if (RtlCompareMemory(reinterpret_cast<PVOID>(address), SsdtStub, sizeof(SsdtStub)) == sizeof(SsdtStub))
		{
			UINT32 offset = 0;

			address += sizeof(SsdtStub);

			RtlCopyMemory(&offset, reinterpret_cast<PVOID>(address), sizeof(offset));

			address += offset + 4;
			*result = address;

			return STATUS_SUCCESS;
		}
		address++;
	} while (true);
	return STATUS_NOT_FOUND;
}

/// <summary>
/// Resolves nt!IopXxxControlFile by searching for the following call instruction:
///     call    nt!IopXxxControlFile (fffff806`7cdfcc10)
/// This works because there is only one instance of it in the entire nt!NtDeviceIoControlFile
/// function. 
/// </summary>
/// <param name="address"></param>
/// <param name="result"></param>
/// <returns>STATUS_SUCCESS/STATUS_NOT_FOUND</returns>
_Use_decl_annotations_
NTSTATUS resolve::IopXxxControlFile(UINT64 address, PUINT64 result)
{
	UINT8 value = 0;

	do
	{
		value = *reinterpret_cast<PUINT8>(address);
		if (value == 0xe8)
		{
			UINT32 offset = 0;

			RtlCopyMemory(&offset, reinterpret_cast<PVOID>(address + 1), sizeof(offset));

			*result = address + offset + 5;

			return STATUS_SUCCESS;
		}
		address++;
	} while (value != 0xc3);

	return STATUS_NOT_FOUND;
}

/// <summary>
/// Resolves nt!IopCreateFile by searching for the following call instruction:
///     call    nt!IopCreateFile (fffff806`7ce6d230)
/// This works because there is only one instance of it in the entire nt!NtDeviceIoControlFile
/// function. 
/// </summary>
/// <param name="address"></param>
/// <param name="result"></param>
/// <returns>STATUS_SUCCESS/STATUS_NOT_FOUND</returns>
_Use_decl_annotations_
NTSTATUS resolve::IopCreateFile(UINT64 address, PUINT64 result)
{
	UINT8 value = 0;

	do
	{
		value = *reinterpret_cast<PUINT8>(address);
		if (value == 0xe8)
		{
			UINT32 offset = 0;

			RtlCopyMemory(&offset, reinterpret_cast<PVOID>(address + 1), sizeof(offset));

			*result = address + offset + 5;

			return STATUS_SUCCESS;
		}
		address++;
	} while (true);
}


/// EOF