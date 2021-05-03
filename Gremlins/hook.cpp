#include "hook.h"

/// <summary>
/// Will check to see if the given function is hooked by its address. It will iterate 
/// through the LIST_ENTRY member of GLOBALS and check if there is a match. It will 
/// then return the "isHooked" member. 
/// </summary>
/// <param name="address"></param>
/// <returns>true/false</returns>
bool hook::isFunctionHookedByAddress(PVOID address)
{
	KIRQL OldIrql;

	ExAcquireSpinLock(&g_Globals->kSpinLock, &OldIrql);

	if (!IsListEmpty(&g_Globals->ListHead))
	{
		PLIST_ENTRY temp = &g_Globals->ListHead;

		do
		{
			temp = temp->Flink;

			PHOOKED_NT_FUNCTION hooked = CONTAINING_RECORD(temp, HOOKED_NT_FUNCTION, entry);

			DbgPrint("[%ws::%d] Syscall index: %d\n", __FUNCTIONW__, __LINE__, hooked->index);
			DbgPrint("[%ws::%d] Hooked address: %p\n", __FUNCTIONW__, __LINE__, hooked->address);
			DbgPrint("[%ws::%d] Is it hooked: %d\n", __FUNCTIONW__, __LINE__, hooked->isHooked);
			DbgPrint("[%ws::%d] Original bytes: %p\n", __FUNCTIONW__, __LINE__, hooked->original);

			///
			/// If an entry is found and it's our address, return the hook information.
			/// 
			if (hooked->address == address)
			{
				DbgPrint("[%ws::%d] Found an existing entry. Returning hook information.\n", __FUNCTIONW__, __LINE__);

				ExReleaseSpinLock(&g_Globals->kSpinLock, OldIrql);

				return hooked->isHooked;
			}
		} while (temp->Flink != &g_Globals->ListHead);
	}

	ExReleaseSpinLock(&g_Globals->kSpinLock, OldIrql);
	return false;
}

/// <summary>
/// Will check to see if the given function is hooked by its address. It will iterate 
/// through the LIST_ENTRY member of GLOBALS and check if there is a match. It will 
/// then return the "isHooked" member. 
/// </summary>
/// <param name="address"></param>
/// <returns>true/false</returns>
bool hook::isFunctionHookedByIndex(UINT16 index)
{
	KIRQL OldIrql;

	ExAcquireSpinLock(&g_Globals->kSpinLock, &OldIrql);

	if (!IsListEmpty(&g_Globals->ListHead))
	{
		PLIST_ENTRY temp = &g_Globals->ListHead;

		do
		{
			temp = temp->Flink;

			PHOOKED_NT_FUNCTION hooked = CONTAINING_RECORD(temp, HOOKED_NT_FUNCTION, entry);

			DbgPrint("[%ws::%d] Syscall index: %d\n", __FUNCTIONW__, __LINE__, hooked->index);
			DbgPrint("[%ws::%d] Hooked address: %p\n", __FUNCTIONW__, __LINE__, hooked->address);
			DbgPrint("[%ws::%d] Is it hooked: %d\n", __FUNCTIONW__, __LINE__, hooked->isHooked);
			DbgPrint("[%ws::%d] Original bytes: %p\n", __FUNCTIONW__, __LINE__, hooked->original);

			///
			/// If an entry is found and it's our address, return the hook information.
			/// 
			if (hooked->index == index)
			{
				DbgPrint("[%ws::%d] Found an existing entry. Returning hook information.\n", __FUNCTIONW__, __LINE__);

				ExReleaseSpinLock(&g_Globals->kSpinLock, OldIrql);

				return hooked->isHooked;
			}
		} while (temp->Flink != &g_Globals->ListHead);
	}

	ExReleaseSpinLock(&g_Globals->kSpinLock, OldIrql);
	return false;
}

/// <summary>
/// Responsible for unhooking a given function.
/// This works by iterating through the LIST_ENTRY member inside GLOBALS. If the address is found,
/// it will then use the detour namespace to overwrite our hook in memory. After it is done, it 
/// will then remove the entry from the list. 
/// </summary>
/// <param name="address"></param>
/// <returns>STATUS_SUCCESS/STATUS_NOT_FOUND/STATUS_INSUFFICIENT_RESOURCES</returns>
NTSTATUS hook::unhookFunction(UINT16 index)
{
	KIRQL OldIrql;

	ExAcquireSpinLock(&g_Globals->kSpinLock, &OldIrql);

	if (!IsListEmpty(&g_Globals->ListHead))
	{
		PLIST_ENTRY temp = &g_Globals->ListHead;

		do
		{
			temp = temp->Flink;

			PHOOKED_NT_FUNCTION hooked = CONTAINING_RECORD(temp, HOOKED_NT_FUNCTION, entry);

			DbgPrint("[%ws::%d] Syscall index: %d\n", __FUNCTIONW__, __LINE__, hooked->index);
			DbgPrint("[%ws::%d] Hooked address: %p\n", __FUNCTIONW__, __LINE__, hooked->address);
			DbgPrint("[%ws::%d] Is it hooked: %d\n", __FUNCTIONW__, __LINE__, hooked->isHooked);
			DbgPrint("[%ws::%d] Original bytes: %p\n", __FUNCTIONW__, __LINE__, hooked->original);

			///
			/// If an entry is found and it's our address, check if its hooked. If it is hooked,
			/// unhook it.
			/// 
			if (hooked->index == index && hooked->isHooked)
			{
				DbgPrint("[%ws::%d] Found an entry!\n", __FUNCTIONW__, __LINE__);

				DbgPrint("[%ws::%d] Going to start copying %p to %p with size: %d\n", __FUNCTIONW__, __LINE__, hooked->original, hooked->address, sizeof(hooked->original));

				NTSTATUS Status = detour::unhook(
					hooked->address, 
					hooked->original, 
					sizeof(hooked->original)
				);

				///
				/// Since we are unhooking this entry, we no longer need it inside our list.
				/// Safe to remove it.
				/// 
				RemoveEntryList(temp);

				///
				/// Now the entry is removed, zero this region of memory and then free it.
				/// 
				RtlSecureZeroMemory(hooked, sizeof(HOOKED_NT_FUNCTION));
				ExFreePoolWithTag(hooked, POOLTAG);


				ExReleaseSpinLock(&g_Globals->kSpinLock, OldIrql);

				return Status;
			}

		} while (temp->Flink != &g_Globals->ListHead);
	}

	ExReleaseSpinLock(&g_Globals->kSpinLock, OldIrql);
	return STATUS_NOT_FOUND;
}

/// <summary>
/// Responsible for clean up. Will iterate through each entry in the list and then zero
/// the memory by using RtlSecureZeroMemory and then freeing that region of memory. 
/// </summary>
void hook::cleanup()
{
	do
	{
		PLIST_ENTRY pe = ExInterlockedRemoveHeadList(
			&g_Globals->ListHead, 
			&g_Globals->kInterlockedSpinLock
		);

		PHOOKED_NT_FUNCTION hooked = CONTAINING_RECORD(pe, HOOKED_NT_FUNCTION, entry);

		RtlSecureZeroMemory(hooked, sizeof(HOOKED_NT_FUNCTION));

		ExFreePoolWithTag(hooked, POOLTAG);
	} while (!IsListEmpty(&g_Globals->ListHead));
}


/// EOF