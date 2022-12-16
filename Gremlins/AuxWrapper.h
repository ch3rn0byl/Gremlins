#pragma once
#include <ntddk.h>
#include <aux_klib.h>

#define AUX_POOLTAG_DBG	'dWxA'
#define AUX_POOLTAG		'rGxA'

class AuxWrapper
{
public:
	AuxWrapper();
	~AuxWrapper();

protected:
	PAUX_MODULE_EXTENDED_INFO m_LoadedModules;
	UINT32 m_NumberOfModules;

private:
	ULONG m_SizeOfModules;
};


/// EOF