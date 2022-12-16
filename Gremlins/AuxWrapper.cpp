#include "AuxWrapper.h"

AuxWrapper::AuxWrapper() :
	m_LoadedModules(nullptr),
	m_SizeOfModules(NULL),
	m_NumberOfModules(NULL)
{
	NTSTATUS Status = AuxKlibInitialize();
	if (!NT_SUCCESS(Status))
	{
		return;
	}

	//
	// Get the size of the buffer needed to hold all loaded modules on 
	// a system.
	//
	Status = AuxKlibQueryModuleInformation(
		&m_SizeOfModules,
		sizeof(AUX_MODULE_EXTENDED_INFO),
		NULL
	);
	if (!NT_SUCCESS(Status) || m_SizeOfModules == NULL)
	{
		return;
	}

	//
	// Get the number of modules loaded.
	//
	m_NumberOfModules = m_SizeOfModules / sizeof(AUX_MODULE_EXTENDED_INFO);

	m_LoadedModules = static_cast<PAUX_MODULE_EXTENDED_INFO>(
#ifdef DBG
		ExAllocatePool2(POOL_FLAG_NON_PAGED | POOL_FLAG_SPECIAL_POOL, m_SizeOfModules, AUX_POOLTAG_DBG)
#else
		ExAllocatePool2(POOL_FLAG_NON_PAGED, m_SizeOfModules, AUX_POOLTAG)
#endif // DBG
		);
	if (m_LoadedModules == NULL)
	{
		return;
	}

	//
	// m_LoadedModules is now allocated with enough bytes to capture all modules
	// on the system. Query again to populate the structure.
	// 
	Status = AuxKlibQueryModuleInformation(
		&m_SizeOfModules,
		sizeof(AUX_MODULE_EXTENDED_INFO),
		m_LoadedModules
	);
}

AuxWrapper::~AuxWrapper()
{
	if (m_LoadedModules != nullptr)
	{
		RtlSecureZeroMemory(m_LoadedModules, m_SizeOfModules);

#ifdef DBG
		ExFreePoolWithTag(m_LoadedModules, AUX_POOLTAG_DBG);
#else
		ExFreePoolWithTag(m_LoadedModules, AUX_POOLTAG);
#endif // DBG

		m_LoadedModules = nullptr;
		m_SizeOfModules = NULL;
		m_NumberOfModules = NULL;
	}
}
