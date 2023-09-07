#pragma once
#include "typesndefs.h"
//#include <ntddk.h>

namespace exclusions
{
	_Success_(return != 0) 
	bool IsKernelImageExcluded(const PUNICODE_STRING KernelImage);

	_Success_(return != 0)
	bool IsKernelImageForAnalysis(const PUNICODE_STRING KernelImage);

	void cleanup();
}