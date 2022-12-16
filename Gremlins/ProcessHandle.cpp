#include "ProcessHandle.h"

ProcessHandle::ProcessHandle() :
	m_FileHandle(reinterpret_cast<HANDLE>(-1)),
	m_FileObject(nullptr),
	m_NameInformation(nullptr),
	m_ReturnLength(NULL)
{
}

ProcessHandle::~ProcessHandle()
{
	//
	// Clean up the buffers that have been created.
	// 
	if (m_NameInformation != nullptr && m_ReturnLength != NULL)
	{
		RtlSecureZeroMemory(m_NameInformation, m_ReturnLength);

#ifdef DBG
		ExFreePoolWithTag(m_NameInformation, phPoolTagDbg);
#else
		ExFreePoolWithTag(m_NameInformation, phPoolTag);
#endif // DBG

		m_NameInformation = nullptr;
		m_ReturnLength = NULL;
	}

	//
	// If the file object was created, decrement the reference count since
	// we are finished using the object. 
	//
	if (m_FileObject != nullptr)
	{
#ifdef DBG
		ObDereferenceObjectWithTag(m_FileObject, ObReferenceTag);
#else
		ObDereferenceObject(m_FileObject);
#endif // DBG		
		m_FileObject = nullptr;
	}
}

_Use_decl_annotations_
ProcessHandle::ProcessHandle(HANDLE hFileHandle) :
	m_FileHandle(hFileHandle),
	m_FileObject(nullptr),
	m_NameInformation(nullptr),
	m_ReturnLength(NULL)
{
}

NTSTATUS ProcessHandle::AnalyzeHandle()
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	//
	// Get the pointer to the object that the handle belongs to. 
	// ObReferenceObjectByHandleWithTag increments the reference count of the
	// object that is identified. If debugging is needed, use dat hoe
	// !obtrace.
	// 
#ifdef DBG
	Status = ObReferenceObjectByHandleWithTag(
		m_FileHandle,
		NULL,
		NULL,
		UserMode,
		ObReferenceTag,
		reinterpret_cast<PVOID*>(&m_FileObject),
		NULL
	);
#else
	Status = ObReferenceObjectByHandle(
		m_FileHandle,
		NULL,
		NULL,
		UserMode,
		reinterpret_cast<PVOID*>(&m_FileObject),
		NULL
	);
#endif // DBG
	if (!NT_SUCCESS(Status) || m_FileObject == nullptr)
	{
		return Status;
	}

	//
	// Now start pulling information from the object. The DeviceObject will be 
	// retrieved first to read from the flags. If the device object has the flag
	// DO_DEVICE_HAS_NAME, this means there's a name to get. 
	//
	if (m_FileObject->DeviceObject->Flags & DO_DEVICE_HAS_NAME)
	{
		//
		// Resolve the size needed to allocate for the name of the device object.
		// The device name is of type PUNICODE_STRING.
		// 
		do
		{
			Status = ZwQueryObject(
				m_FileHandle,
				ObjectNameInformation,
				m_NameInformation,
				m_ReturnLength,
				&m_ReturnLength
			);
			if (!NT_SUCCESS(Status) && Status == STATUS_INFO_LENGTH_MISMATCH)
			{
				if (m_NameInformation != nullptr)
				{
#ifdef DBG
					ExFreePoolWithTag(m_NameInformation, phPoolTagDbg);
#else
					ExFreePoolWithTag(m_NameInformation, phPoolTag);
#endif // DBG
					m_NameInformation = nullptr;
				}

				m_NameInformation = static_cast<PUNICODE_STRING>(
#ifdef DBG
					ExAllocatePool2(POOL_FLAG_NON_PAGED | POOL_FLAG_SPECIAL_POOL, m_ReturnLength, phPoolTagDbg)
#else
					ExAllocatePool2(POOL_FLAG_NON_PAGED, m_ReturnLength, phPoolTag)
#endif // DBG
					);
				if (m_NameInformation == NULL)
				{
					return STATUS_MEMORY_NOT_ALLOCATED;
				}
			}
		} while (!NT_SUCCESS(Status));
	}

	return Status;
}

PSECURITY_DESCRIPTOR ProcessHandle::GetFileObjectSD()
{
	return m_FileObject->DeviceObject->SecurityDescriptor;
}


/// EOF