#include "detour.h"

/// <summary>
/// Hooks the region of memory specified. This is done by resolving the virtual
/// addresses physical address and then mapping it into userspace with 
/// MmMapIoSpace as read/write. 
/// After the hook is placed, the mapped address gets unmapped with MmUnmapIoSpace. 
/// </summary>
/// <param name="BaseAddress"></param>
/// <param name="patch"></param>
/// <param name="szSize"></param>
/// <returns>STATUS_SUCCESS/STATUS_INSUFFICIENT_RESOURCES</returns>
NTSTATUS detour::hook(PVOID BaseAddress, PUINT8 patch, size_t szSize)
{
	PHYSICAL_ADDRESS pa = MmGetPhysicalAddress(BaseAddress);

	PVOID mappedAddress = MmMapIoSpace(pa, 0x60, MmNonCached);
	if (mappedAddress == NULL)
	{
		DbgPrint("[%ws::%d] MmMapIoSpace failed: %08x\n", __FUNCTIONW__, __LINE__, STATUS_INSUFFICIENT_RESOURCES);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlCopyMemory(mappedAddress, patch, szSize);

	MmUnmapIoSpace(mappedAddress, 0x60);

	return STATUS_SUCCESS;
}


/// <summary>
/// Unhooks the region of memory specified. This function does the same thing as 
/// detour::hook, but copies the original bytes instead putting the function into its
/// original state. 
/// </summary>
/// <param name="BaseAddress"></param>
/// <param name="Source"></param>
/// <param name="szSize"></param>
/// <returns>STATUS_SUCCESS/STATUS_INSUFFICIENT_RESOURCES</returns>
NTSTATUS detour::unhook(PVOID BaseAddress, PUINT8 Source, UINT8 szSize)
{
	PHYSICAL_ADDRESS pa = MmGetPhysicalAddress(BaseAddress);

	PVOID mappedAddress = MmMapIoSpace(pa, 0x60, MmNonCached);
	if (mappedAddress == NULL)
	{
		DbgPrint("[%ws::%d] MmMapIoSpace failed: %08x\n", __FUNCTIONW__, __LINE__, STATUS_INSUFFICIENT_RESOURCES);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlCopyMemory(mappedAddress, Source, szSize);

	MmUnmapIoSpace(mappedAddress, 0x60);

	return STATUS_SUCCESS;
}


/// EOF 