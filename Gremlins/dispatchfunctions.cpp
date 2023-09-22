#include "dispatchfunctions.h"

_Use_decl_annotations_
NTSTATUS
IsModuleInitialized(
	PINPUT_BUFFER& InputBuffer,
	PULONG Information
)
{
	InputBuffer->status = g_Globals->IsInitialized;
	*Information = sizeof(INPUT_BUFFER);

	return STATUS_SUCCESS;
}

NTSTATUS
InitializeModule()
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	UINT64 KiSystemServiceuser = 0;
	UINT64 KeServiceDescriptorTable = 0;

	// 
	// Get the lstar address.
	//
	UINT64 Lstar = __readmsr(IA32_LSTAR_MSR);

	Status = resolve::KiSystemServiceUser(Lstar, &KiSystemServiceuser);
	if (!NT_SUCCESS(Status) || KiSystemServiceuser == NULL)
	{
		return Status;
	}
	else
	{
		LOG_TRACE("[%ws::%d] KiSystemServiceUser: %p.\n", __FUNCTIONW__, __LINE__, KiSystemServiceuser);
	}

	Status = resolve::KeServiceDescriptorTable(KiSystemServiceuser, &KeServiceDescriptorTable);
	if (!NT_SUCCESS(Status) || KiSystemServiceuser == NULL)
	{
		return Status;
	}
	else
	{
		LOG_TRACE("[%ws::%d] KeServiceDescriptorTable: %p.\n", __FUNCTIONW__, __LINE__, KeServiceDescriptorTable);
	}

	g_Globals->ssdt = reinterpret_cast<PKSERVICE_DESCRIPTOR_TABLE>(
		KeServiceDescriptorTable
		);
	g_Globals->IsInitialized = true;

	LOG_TRACE("[%ws::%d] g_Globals has been initialized.\n", __FUNCTIONW__, __LINE__);

	return Status;
}

_Use_decl_annotations_
NTSTATUS
IsSyscallHooked(
	PINPUT_BUFFER& InputBuffer,
	PULONG Information
)
{
	//
	// Validate the user's input.
	//
	if (InputBuffer->syscall > g_Globals->ssdt->NumberOfServices ||
		InputBuffer->syscall < 0)
	{
		return STATUS_INVALID_PARAMETER;
	}

	// TODO: Fix status type or add status type
	InputBuffer->status = hook::isFunctionHookedByIndex(InputBuffer->syscall);
	*Information = sizeof(INPUT_BUFFER);

	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS
HookSyscall(
	PINPUT_BUFFER& InputBuffer
)
{
	//
	// Validate the user provided syscall.
	//
	if (InputBuffer->syscall > g_Globals->ssdt->NumberOfServices ||
		InputBuffer->syscall < 0)
	{
		return STATUS_INVALID_PARAMETER;
	}

	UINT64 fnSyscall = 0;
	UINT64 fn_hSyscall = 0;

	NTSTATUS Status = STATUS_SUCCESS;

	//
	// The syscall number is used as an offset to the System Service Descriptor Table. 
	// To resolve the function of the syscall, one must do the following:
	//   1. Reference the index with the syscall
	//   2. Shift it right by four. 
	//   3. ??
	//   4. Profit.
	//
	fnSyscall = reinterpret_cast<UINT64>(g_Globals->ssdt->ServicetableBase);
	fnSyscall += g_Globals->ssdt->ServicetableBase[InputBuffer->syscall] >> 4;

	switch (InputBuffer->syscall)
	{
	case NtCreateFile:
		if (!hook::isFunctionHookedByAddress(reinterpret_cast<PVOID>(fnSyscall)))
		{
			g_Globals->hooked = static_cast<PHOOKED_SYSCALLS>(
#ifdef DBG
				ExAllocatePool2(POOL_FLAG_NON_PAGED | POOL_FLAG_SPECIAL_POOL, sizeof(GLOBALS), POOLTAG_DBG)
#else
				ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(GLOBALS), POOLTAG)
#endif // DBG
				);
			if (g_Globals->hooked == NULL)
			{
				Status = STATUS_MEMORY_NOT_ALLOCATED;
				LOG_ERR("[%ws::%d] Failed with status 0x%08x.\n", __FUNCTIONW__, __LINE__, Status);
				break;
			}

			LOG_TRACE("[%ws::%d] g_Globals->hooked allocated at %p.\n", __FUNCTIONW__, __LINE__, g_Globals->hooked);

			//
			// Save the original address of the NT function.
			//
			g_Globals->hooked->address = reinterpret_cast<PVOID>(fnSyscall);
			g_Globals->hooked->Index = InputBuffer->syscall;

			//
			// NtCreateFile has an internal function it hits. Will need to resolve IopCreateFile. 
			//
			Status = resolve::IopCreateFile(
				reinterpret_cast<UINT64>(g_Globals->hooked->address),
				reinterpret_cast<PUINT64>(&g_Globals->internal.IopCreateFile)
			);
			if (!NT_SUCCESS(Status) || g_Globals->internal.IopCreateFile == NULL)
			{
				LOG_ERR("[%ws::%d] %p was not found.\n", __FUNCTIONW__, __LINE__, g_Globals->internal.IopCreateFile);
				Status = STATUS_NOT_FOUND;
				break;
			}

			fn_hSyscall = reinterpret_cast<UINT64>(fn_hNtCreateFile);

			//
			// Copy enough memory of the function to restore afterwards. 
			//
			RtlCopyMemory(
				g_Globals->hooked->original,
				g_Globals->hooked->address,
				sizeof(g_Globals->hooked->original)
			);

			//
			// Patch the temp bytes that way RAX points to something useful.
			// 
			RtlCopyMemory(HookingStub + 2, &fn_hSyscall, sizeof(fn_hSyscall));

			Status = detour::hook(reinterpret_cast<PVOID>(fnSyscall), HookingStub, sizeof(HookingStub));
			if (!NT_SUCCESS(Status))
			{
				LOG_ERR("[%ws::%d] Failed with status 0x%08x.\n", __FUNCTIONW__, __LINE__, Status);
				break;
			}
			else
			{
				g_Globals->hooked->IsHooked = true;
			}

			//
			// Keep track of the functions that are hooked. Insert dem hoes into
			// the list entry. 
			// 
			ExInterlockedInsertTailList(
				&g_Globals->HookedListHead,
				&g_Globals->hooked->entry,
				&g_Globals->kInterlockedSpinLock
			);

			LOG_TRACE("[%ws::%d] NtCreateFile has been hooked.\n", __FUNCTIONW__, __LINE__);
		}
		break;
	case NtDeviceIoControlFile:
		//
		// Check to make sure the function is not already hooked by querying 
		// the address.
		//
		if (!hook::isFunctionHookedByAddress(reinterpret_cast<PVOID>(fnSyscall)))
		{
			g_Globals->hooked = static_cast<PHOOKED_SYSCALLS>(
#ifdef DBG
				ExAllocatePool2(POOL_FLAG_NON_PAGED | POOL_FLAG_SPECIAL_POOL, sizeof(GLOBALS), POOLTAG_DBG)
#else
				ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(GLOBALS), POOLTAG)
#endif // DBG
				);
			if (g_Globals->hooked == NULL)
			{
				Status = STATUS_MEMORY_NOT_ALLOCATED;
				LOG_ERR("[%ws::%d] Failed with status 0x%08x.\n", __FUNCTIONW__, __LINE__, Status);
				break;
			}

			LOG_TRACE("[%ws::%d] g_Globals->hooked allocated at %p.\n", __FUNCTIONW__, __LINE__, g_Globals->hooked);

			//
			// Save the original address of the NT function.
			//
			g_Globals->hooked->address = reinterpret_cast<PVOID>(fnSyscall);
			g_Globals->hooked->Index = InputBuffer->syscall;

			//
			// NtDeviceIoControlFile has an internal function it hits. Will need to 
			// resolve IopXxxControlFile. 
			//
			Status = resolve::IopXxxControlFile(
				reinterpret_cast<UINT64>(g_Globals->hooked->address),
				reinterpret_cast<PUINT64>(&g_Globals->internal.IopXxxControlFile)
			);
			if (!NT_SUCCESS(Status) || g_Globals->internal.IopXxxControlFile == NULL)
			{
				LOG_ERR("[%ws::%d] %p was not found.\n", __FUNCTIONW__, __LINE__, g_Globals->internal.IopXxxControlFile);
				Status = STATUS_NOT_FOUND;
				break;
			}

			fn_hSyscall = reinterpret_cast<UINT64>(fn_hNtDeviceIoControlFile);

			//
			// Copy enough memory of the function to restore afterwards. 
			//
			RtlCopyMemory(
				g_Globals->hooked->original,
				g_Globals->hooked->address,
				sizeof(g_Globals->hooked->original)
			);

			//
			// Patch the temp bytes that way RAX points to something useful.
			// 
			RtlCopyMemory(HookingStub + 2, &fn_hSyscall, sizeof(fn_hSyscall));

			Status = detour::hook(reinterpret_cast<PVOID>(fnSyscall), HookingStub, sizeof(HookingStub));
			if (!NT_SUCCESS(Status))
			{
				LOG_ERR("[%ws::%d] Failed with status 0x%08x.\n", __FUNCTIONW__, __LINE__, Status);
				break;
			}
			else
			{
				g_Globals->hooked->IsHooked = true;
			}

			//
			// Keep track of the functions that are hooked. Insert dem hoes into
			// the list entry. 
			// 
			ExInterlockedInsertTailList(
				&g_Globals->HookedListHead,
				&g_Globals->hooked->entry,
				&g_Globals->kInterlockedSpinLock
			);

			LOG_TRACE("[%ws::%d] NtDeviceIoControlFile has been hooked.\n", __FUNCTIONW__, __LINE__);
		}
		break;
	default:
		Status = STATUS_NOT_IMPLEMENTED;
		break;
	}

	return Status;
}

_Use_decl_annotations_
NTSTATUS
UnhookSyscall(
	PINPUT_BUFFER InputBuffer
)
{
	//
	// Validate the user's provided syscall.
	//
	if (InputBuffer->syscall > g_Globals->ssdt->NumberOfServices ||
		InputBuffer->syscall < 0)
	{
		return STATUS_INVALID_PARAMETER;
	}

	return hook::unhookFunction(InputBuffer->syscall);
}

_Use_decl_annotations_
NTSTATUS ExcludeDriver(PVOID InputBuffer)
{
	if (InputBuffer == nullptr)
	{
		return STATUS_INVALID_PARAMETER_1;
	}

	PUNICODE_STRING ImageName = static_cast<PUNICODE_STRING>(InputBuffer);

	if (exclusions::IsKernelImageExcluded(ImageName))
	{
		return STATUS_ALREADY_INITIALIZED;
	}

	g_Globals->ExcludedKernelImages = static_cast<PEXCLUDED_KERNEL_IMAGE>(
		ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(EXCLUDED_KERNEL_IMAGE), POOLTAG)
		);
	if (g_Globals->ExcludedKernelImages == nullptr)
	{
		return STATUS_MEMORY_NOT_ALLOCATED;
	}
	
	PUNICODE_STRING ExcludedImageName = static_cast<PUNICODE_STRING>(
		ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(UNICODE_STRING), POOLTAG)
		);
	if (ExcludedImageName == nullptr)
	{
		return STATUS_MEMORY_NOT_ALLOCATED;
	}

	ExcludedImageName->Buffer = static_cast<PWCH>(
		ExAllocatePool2(POOL_FLAG_NON_PAGED, ImageName->MaximumLength, POOLTAG)
		);
	if (ExcludedImageName->Buffer == nullptr)
	{
		return STATUS_MEMORY_NOT_ALLOCATED;
	}

	ExcludedImageName->Length = ImageName->Length;
	ExcludedImageName->MaximumLength = ImageName->MaximumLength;

	RtlCopyUnicodeString(ExcludedImageName, ImageName);

	g_Globals->ExcludedKernelImages->KernelImageName = ExcludedImageName;

	ExInterlockedInsertTailList(
		&g_Globals->ExcludeDriverListHead,
		&g_Globals->ExcludedKernelImages->entry,
		&g_Globals->kInterlockedSpinLock
	);

	LOG_TRACE("[%ws::%d] %wZ has been excluded.\n", __FUNCTIONW__, __LINE__, ExcludedImageName);

	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS AnalyzeImage(PVOID InputBuffer)
{
	if (InputBuffer == nullptr)
	{
		return STATUS_INVALID_PARAMETER_1;
	}

	PUNICODE_STRING ImageName = static_cast<PUNICODE_STRING>(InputBuffer);
	LOG_TRACE("[%ws::%d] going to attempt to insert %wZ\n", __FUNCTIONW__, __LINE__, ImageName);

	if (exclusions::IsKernelImageForAnalysis(ImageName))
	{
		LOG_TRACE("[%ws::%d] %wZ is already being analyzed.\n", __FUNCTIONW__, __LINE__, ImageName);
		return STATUS_ALREADY_INITIALIZED;
	}

	g_Globals->AnalyzeKernelImages = static_cast<PANALYSIS_KERNEL_IMAGE>(
		ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(ANALYSIS_KERNEL_IMAGE), POOLTAG)
		);
	if (g_Globals->AnalyzeKernelImages == nullptr)
	{
		return STATUS_MEMORY_NOT_ALLOCATED;
	}

	PUNICODE_STRING AnalyzeImageName = static_cast<PUNICODE_STRING>(
		ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(UNICODE_STRING), POOLTAG)
		);
	if (AnalyzeImageName == nullptr)
	{
		return STATUS_MEMORY_NOT_ALLOCATED;
	}

	AnalyzeImageName->Buffer = static_cast<PWCH>(
		ExAllocatePool2(POOL_FLAG_NON_PAGED, ImageName->MaximumLength, POOLTAG)
		);
	if (AnalyzeImageName->Buffer == nullptr)
	{
		return STATUS_MEMORY_NOT_ALLOCATED;
	}

	AnalyzeImageName->Length = ImageName->Length;
	AnalyzeImageName->MaximumLength = ImageName->MaximumLength;


	RtlCopyUnicodeString(AnalyzeImageName, ImageName);

	LOG_TRACE("[%ws::%d] Copied %wZ to %wZ\n", __FUNCTIONW__, __LINE__, ImageName, AnalyzeImageName);

	g_Globals->AnalyzeKernelImages->KernelImageName = AnalyzeImageName;

	ExInterlockedInsertTailList(
		&g_Globals->AnalyzeKernelImageListHead,
		&g_Globals->AnalyzeKernelImages->entry,
		&g_Globals->kInterlockedSpinLock
	);

	LOG_TRACE("[%ws::%d] %wZ has been set for analysis.\n", __FUNCTIONW__, __LINE__, AnalyzeImageName);

	return STATUS_SUCCESS;
}


/// EOF