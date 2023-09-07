#include "exclusions.h"

_Use_decl_annotations_
bool exclusions::IsKernelImageExcluded(const PUNICODE_STRING KernelImage)
{
	KIRQL OldIrql = 0;

	PLIST_ENTRY temp = nullptr;
	PEXCLUDED_KERNEL_IMAGE ExcludedEntry = nullptr;

	ExAcquireSpinLock(&g_Globals->kSpinLock, &OldIrql);

	if (!IsListEmpty(&g_Globals->ExcludeDriverListHead))
	{
		temp = &g_Globals->ExcludeDriverListHead;

		do
		{
			temp = temp->Flink;
			ExcludedEntry = CONTAINING_RECORD(temp, EXCLUDED_KERNEL_IMAGE, entry);

			if (RtlCompareUnicodeString(KernelImage, ExcludedEntry->KernelImageName, TRUE) == 0)
			{
				ExReleaseSpinLock(&g_Globals->kSpinLock, OldIrql);
				return true;
			}
		} while (temp->Flink != &g_Globals->ExcludeDriverListHead);
	}

	ExReleaseSpinLock(&g_Globals->kSpinLock, OldIrql);
	return false;
}

_Use_decl_annotations_
bool exclusions::IsKernelImageForAnalysis(const PUNICODE_STRING KernelImage)
{
	KIRQL OldIrql = 0;

	PLIST_ENTRY temp = nullptr;
	PANALYSIS_KERNEL_IMAGE AnalysisEntry = nullptr;

	ExAcquireSpinLock(&g_Globals->kSpinLock, &OldIrql);

	if (!IsListEmpty(&g_Globals->AnalyzeKernelImageListHead))
	{
		temp = &g_Globals->AnalyzeKernelImageListHead;

		do
		{
			temp = temp->Flink;
			AnalysisEntry = CONTAINING_RECORD(temp, ANALYSIS_KERNEL_IMAGE, entry);

			if (RtlCompareUnicodeString(KernelImage, AnalysisEntry->KernelImageName, TRUE) == 0)
			{
				ExReleaseSpinLock(&g_Globals->kSpinLock, OldIrql);
				return true;
			}
		} while (temp->Flink != &g_Globals->AnalyzeKernelImageListHead);
	}

	ExReleaseSpinLock(&g_Globals->kSpinLock, OldIrql);
	return false;
}

void exclusions::cleanup()
{
	KIRQL OldIrql = 0;

	PLIST_ENTRY temp = nullptr;
	PEXCLUDED_KERNEL_IMAGE image = nullptr;
	PANALYSIS_KERNEL_IMAGE analysis = nullptr;

	ExAcquireSpinLock(&g_Globals->kSpinLock, &OldIrql);

	if (!IsListEmpty(&g_Globals->ExcludeDriverListHead))
	{
		temp = &g_Globals->ExcludeDriverListHead;

		do
		{
			temp = temp->Flink;

			image = CONTAINING_RECORD(temp, EXCLUDED_KERNEL_IMAGE, entry);

			LOG_TRACE("[%ws::%d] Cleaning %wZ\n", __FUNCTIONW__, __LINE__, image->KernelImageName);

			ExFreePoolWithTag(image->KernelImageName->Buffer, POOLTAG);
			ExFreePoolWithTag(image->KernelImageName, POOLTAG);
			
			RemoveEntryList(temp);
			ExFreePoolWithTag(image, POOLTAG);

		} while (temp->Flink != &g_Globals->ExcludeDriverListHead);
	}

	if (!IsListEmpty(&g_Globals->AnalyzeKernelImageListHead))
	{
		do
		{
			temp = &g_Globals->AnalyzeKernelImageListHead;

			analysis = CONTAINING_RECORD(temp, ANALYSIS_KERNEL_IMAGE, entry);

			LOG_TRACE("[%ws::%d] Cleaning %wZ\n", __FUNCTIONW__, __LINE__, analysis->KernelImageName);

			ExFreePoolWithTag(analysis->KernelImageName->Buffer, POOLTAG);
			ExFreePoolWithTag(analysis->KernelImageName, POOLTAG);

			RemoveEntryList(temp);
			ExFreePoolWithTag(analysis, POOLTAG);

		} while (temp->Flink != &g_Globals->ExcludeDriverListHead);
	}

	ExReleaseSpinLock(&g_Globals->kSpinLock, OldIrql);
}


/// EOF