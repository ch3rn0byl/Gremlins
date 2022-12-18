#pragma once
#include <ntddk.h>
#include <aux_klib.h>
#include "AuxTypes.h"

class AuxWrapper
{
public:
	AuxWrapper();

	/// <summary>
	/// The deconstructor is responsible for cleaning up and deallocating
	/// memory.
	/// </summary>
	~AuxWrapper();

protected:
	PAUX_MODULE_EXTENDED_INFO m_LoadedModules;
	UINT32 m_NumberOfModules;

private:
	ULONG m_SizeOfModules;
};


/// EOF