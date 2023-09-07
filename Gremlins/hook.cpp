#include "hook.h"

/// <summary>
/// Will check to see if the given function is hooked by its address. It will iterate 
/// through the LIST_ENTRY member of GLOBALS and check if there is a match. It will 
/// then return the "isHooked" member. 
/// </summary>
/// <param name="address"></param>
/// <returns>true/false</returns>
_Use_decl_annotations_
bool
hook::isFunctionHookedByAddress(
	PVOID address
)
{
	KIRQL OldIrql = 0;

	PLIST_ENTRY temp = nullptr;
	PHOOKED_SYSCALLS HookedSyscall = nullptr;

	ExAcquireSpinLock(&g_Globals->kSpinLock, &OldIrql);

	if (!IsListEmpty(&g_Globals->HookedListHead))
	{
		temp = &g_Globals->HookedListHead;

		do
		{
			temp = temp->Flink;
			HookedSyscall = CONTAINING_RECORD(temp, HOOKED_SYSCALLS, entry);

			//
			// If an entry is found and it's our address, return the hook information.
			//
			if (HookedSyscall->address == address)
			{
				ExReleaseSpinLock(&g_Globals->kSpinLock, OldIrql);
				return HookedSyscall->IsHooked;
			}
		} while (temp->Flink != &g_Globals->HookedListHead);
	}

	ExReleaseSpinLock(&g_Globals->kSpinLock, OldIrql);
	return false;
}

_Use_decl_annotations_
bool
hook::isFunctionHookedByIndex(
	UINT16 index
)
{
	KIRQL OldIrql = 0;

	PLIST_ENTRY temp = nullptr;
	PHOOKED_SYSCALLS hooked = nullptr;

	ExAcquireSpinLock(&g_Globals->kSpinLock, &OldIrql);

	if (!IsListEmpty(&g_Globals->HookedListHead))
	{
		temp = &g_Globals->HookedListHead;

		do
		{
			temp = temp->Flink;
			hooked = CONTAINING_RECORD(temp, HOOKED_SYSCALLS, entry);

			//
			// If an entry is found and it's our address, return the hook information.
			// 
			if (hooked->Index == index)
			{
				ExReleaseSpinLock(&g_Globals->kSpinLock, OldIrql);
				return hooked->IsHooked;
			}
		} while (temp->Flink != &g_Globals->HookedListHead);
	}

	ExReleaseSpinLock(&g_Globals->kSpinLock, OldIrql);
	return false;
}

_Use_decl_annotations_
NTSTATUS
hook::unhookFunction(
	UINT16 index
)
{
	KIRQL OldIrql = 0;

	PLIST_ENTRY temp = nullptr;
	PHOOKED_SYSCALLS hooked = nullptr;

	NTSTATUS Status = STATUS_NOT_FOUND;

	ExAcquireSpinLock(&g_Globals->kSpinLock, &OldIrql);

	if (!IsListEmpty(&g_Globals->HookedListHead))
	{
		temp = &g_Globals->HookedListHead;

		do
		{
			temp = temp->Flink;
			hooked = CONTAINING_RECORD(temp, HOOKED_SYSCALLS, entry);

			//
			// If an entry is found and it's our address, check if its hooked. If it is hooked,
			// unhook it.
			// 
			if (hooked->Index == index && hooked->IsHooked)
			{
				Status = detour::unhook(
					hooked->address,
					hooked->original,
					sizeof(hooked->original)
				);

				//
				// Since we are unhooking this entry, we no longer need it inside our list.
				// Safe to remove it.
				// 
				RemoveEntryList(temp);

				//
				// Now the entry is removed, zero this region of memory and then free it.
				// 
				RtlSecureZeroMemory(hooked, sizeof(HOOKED_SYSCALLS));

#ifdef DBG
				ExFreePoolWithTag(hooked, POOLTAG_DBG);
#else
				ExFreePoolWithTag(hooked, POOLTAG);
#endif // DBG

				ExReleaseSpinLock(&g_Globals->kSpinLock, OldIrql);

				return Status;
			}
		} while (temp->Flink != &g_Globals->HookedListHead);
	}

	ExReleaseSpinLock(&g_Globals->kSpinLock, OldIrql);
	return Status;
}

void
hook::cleanup()
{
	PLIST_ENTRY pe = nullptr;
	PHOOKED_SYSCALLS hooked = nullptr;

	do
	{
		pe = ExInterlockedRemoveHeadList(
			&g_Globals->HookedListHead,
			&g_Globals->kInterlockedSpinLock
		);

		hooked = CONTAINING_RECORD(pe, HOOKED_SYSCALLS, entry);

		if (!NT_SUCCESS(unhookFunction(hooked->Index)))
		{
			LOG_TRACE("[%ws::%d] Something messed up\n", __FUNCTIONW__, __LINE__);
		}

		RtlSecureZeroMemory(hooked, sizeof(HOOKED_SYSCALLS));

#ifdef DBG
		ExFreePoolWithTag(hooked, POOLTAG_DBG);
#else
		ExFreePoolWithTag(hooked, POOLTAG);
#endif // DBG

	} while (!IsListEmpty(&g_Globals->HookedListHead));
}


/// EOF