#include "Leaks.h"

_Use_decl_annotations_
Leaks::Leaks(PUINT8 Buffer, ULONG BufferLen) :
	m_Buffer(Buffer),
	m_BufferLen(BufferLen)
{
}

Leaks::~Leaks()
{
}

_Use_decl_annotations_
UINT64
Leaks::DidKernelAddressLeak()
{
	VIRTUAL_ADDR va = { 0 };
	ULARGE_INTEGER temp = { 0 };

	if (m_Buffer == NULL)
	{
		return NULL;
	}

	//
	// Need to ensure the buffer is 8 bytes or bigger. If not,
	// an out-of-bounds read is totally possible. Ain't got time fo dat thang o thang.
	//
	if (m_BufferLen < 8)
	{
		return NULL;
	}

	if (m_LoadedModules)
	{
		//
		// Iterate through the entire buffer and check for any types of kernel
		// leaks.
		// 
		for (ULONG i = 0; i < m_BufferLen; i++)
		{
			if (m_BufferLen - i >= 8)
			{
				va.value = *reinterpret_cast<PUINT64>(
					m_Buffer + i
					);

				//
				// Is this value a kernel address?
				// 
				if (va.MSB)
				{
					for (UINT32 index = 0; index < m_NumberOfModules; index++)
					{
						temp.QuadPart = reinterpret_cast<ULONGLONG>(
							m_LoadedModules[index].BasicInfo.ImageBase
							);

						if (va.Address.HighPart == temp.HighPart)
						{
							return va.Address.QuadPart;
						}
					}
				}
			}
		}
	}

	return NULL;
}


/// EOF