#include "FuzzEngine.h"

FuzzEngine::FuzzEngine()
{
	srand(GetTickCount());
	hHeap = GetProcessHeap();
}

void FuzzEngine::SetChaos(DWORD dwNewChaos)
{
	dwChaosAmount = dwNewChaos;
}

void FuzzEngine::SetSeed(PVOID pBuffer, DWORD dwBufferLength)
{
	pSeedBuffer = HeapAlloc(hHeap, 0, dwBufferLength);
	memcpy(pSeedBuffer, pBuffer, dwBufferLength);
	dwSeedBuffer = dwBufferLength;
}

void FuzzEngine::GetMutation(PVOID* pBuffer, DWORD* dwBufferLength)
{
	*pBuffer = pMutationBuffer;
	*dwBufferLength = dwMutationBufferLength;
}

void FuzzEngine::Mutate()
{
	DWORD dwRandom = 0;
	DWORD dwChaosIndex = 0;

	if (NULL == pSeedBuffer)
	{
		goto end;
	}

	if (NULL != pMutationBuffer)
	{
		HeapFree(hHeap, 0, pMutationBuffer);
		pMutationBuffer = NULL;
	}

	dwMutationBufferLength = dwSeedBuffer;
	pMutationBuffer = HeapAlloc(hHeap, 0, dwMutationBufferLength);
	memcpy(pMutationBuffer, pSeedBuffer, dwSeedBuffer);
	
	

	for (dwChaosIndex = 0; dwChaosIndex < dwChaosAmount; dwChaosIndex++)
	{
		dwRandom = rand() % 5;
		switch (dwRandom)
		{
			case 0:
				MutatorFlip();
				break;

			case 1:
				MutatorSwap();
				break;

			case 2:
				MutatorReplicate();
				break;

			case 3:
				MutatorInsert();
				break;

			case 4:
				MutatorTruncate();
				break;
		}
	}

	end:
	return;
}

void FuzzEngine::MutatorFlip()
{
	CHAR cTargetByte = '\x00';
	DWORD dwRandomByte = 0;
	DWORD dwRandomBit = 0;

	if (0 == dwMutationBufferLength)
	{
		return;
	}

	dwRandomByte = rand() % (dwMutationBufferLength);
	dwRandomBit = rand() % 8;

	cTargetByte = ((CHAR*)pMutationBuffer)[dwRandomByte];
	cTargetByte ^= (cTargetByte & (1 << dwRandomBit));
	((CHAR*)pMutationBuffer)[dwRandomByte] = cTargetByte;

}

void FuzzEngine::MutatorSwap()
{
	CHAR cTempByte = '\x00';
	DWORD dwRandomByte = 0;
	DWORD dwRandomByte2 = 0;

	if (0 == dwMutationBufferLength)
	{
		return;
	}

	dwRandomByte = dwRandomByte2 = rand() % (dwMutationBufferLength);
	while (dwRandomByte == dwRandomByte2 && dwMutationBufferLength > 1)
	{
		dwRandomByte = rand() % (dwMutationBufferLength);
	}

	cTempByte = ((CHAR*)pMutationBuffer)[dwRandomByte];
	((CHAR*)pMutationBuffer)[dwRandomByte] = ((CHAR*)pMutationBuffer)[dwRandomByte2];
	((CHAR*)pMutationBuffer)[dwRandomByte2] = cTempByte;
}

void FuzzEngine::MutatorReplicate()
{
	DWORD dwOldLength = 0;
	CHAR* pNewMutationBuffer = NULL;
	DWORD dwFirstIndex = 0;
	DWORD dwSecondIndex = 0;
	DWORD dwInsert = 0;
	DWORD dwFirstLength = 0;
	DWORD dwSecondLength = 0;
	DWORD dwThirdLength = 0;

	if (0 == dwMutationBufferLength)
	{
		return;
	}

	dwFirstIndex = rand() % (dwMutationBufferLength);
	dwSecondIndex = (dwFirstIndex + rand() % (dwMutationBufferLength - dwFirstIndex));
	dwSecondIndex = dwSecondIndex < dwMutationBufferLength ? dwSecondIndex : dwMutationBufferLength - 1;
	dwInsert = rand() % (dwMutationBufferLength);

	dwOldLength = dwMutationBufferLength;

	dwFirstLength = dwFirstIndex + 1;
	dwSecondLength = dwSecondIndex - dwFirstIndex;
	dwThirdLength = dwOldLength - dwSecondIndex;

	dwMutationBufferLength = dwFirstLength + dwSecondLength + dwThirdLength;

	pNewMutationBuffer = (CHAR*)HeapAlloc(hHeap, 0, dwMutationBufferLength);
	RtlSecureZeroMemory(pNewMutationBuffer, dwMutationBufferLength);
	if (NULL == pNewMutationBuffer)
	{
		return;
	}

	memcpy(pNewMutationBuffer, pMutationBuffer, dwFirstLength);
	memcpy(pNewMutationBuffer + dwFirstLength, (CHAR*)pMutationBuffer + dwFirstIndex, dwSecondLength);
	memcpy(pNewMutationBuffer + dwFirstLength + dwSecondLength,
		(CHAR*)pMutationBuffer + dwSecondIndex, dwThirdLength);

	HeapFree(hHeap, 0, pMutationBuffer);
	pMutationBuffer = pNewMutationBuffer;

}

void FuzzEngine::MutatorInsert()
{
	CHAR* pNewMutationBuffer = NULL;
	DWORD dwNewPosition = 0;
	DWORD dwByteCursor = 0;
	DWORD dwRandomByte = 0;

	if (0 != dwMutationBufferLength)
	{
		dwRandomByte = rand() % (dwMutationBufferLength);
	}

	pNewMutationBuffer = (CHAR*)HeapAlloc(hHeap, 0, dwMutationBufferLength + 1);

	if (NULL == pNewMutationBuffer)
	{
		return;
	}

	for (dwByteCursor = 0; dwByteCursor < dwMutationBufferLength; dwByteCursor++)
	{
		if (dwByteCursor == dwRandomByte)
		{
			pNewMutationBuffer[dwNewPosition++] = rand() % 256;
		}

		dwNewPosition++;
	}

	HeapFree(hHeap, 0, pMutationBuffer);
	pMutationBuffer = pNewMutationBuffer;
	dwMutationBufferLength += 1;


}

void FuzzEngine::MutatorTruncate()
{
	CHAR* pNewMutationBuffer = NULL;
	DWORD dwNewPosition = 0;
	DWORD dwByteCursor = 0;
	DWORD dwRandomByte = 0;
	
	if (dwMutationBufferLength == 0)
	{
		return;
	}

	dwRandomByte = rand() % (dwMutationBufferLength);

	if (0 == dwMutationBufferLength || NULL == pMutationBuffer)
	{
		return;
	}

	pNewMutationBuffer = (CHAR*)HeapAlloc(hHeap, 0, dwMutationBufferLength - 1);
	if (NULL == pNewMutationBuffer)
	{
		return;
	}

	for (dwByteCursor = 0; dwByteCursor < dwMutationBufferLength; dwByteCursor++)
	{
		if (dwByteCursor != dwRandomByte)
		{
			pNewMutationBuffer[dwNewPosition] = ((CHAR*)pMutationBuffer)[dwByteCursor];
			dwNewPosition++;
		}
	}

	HeapFree(hHeap, 0, pMutationBuffer);
	pMutationBuffer = pNewMutationBuffer;
	dwMutationBufferLength -= 1;
}

BOOL FuzzEngine::HasSeed()
{
	if (NULL == pSeedBuffer)
	{
		return FALSE;
	}

	return TRUE;
}