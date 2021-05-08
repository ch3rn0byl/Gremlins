#pragma once
#ifndef FUZZ_ENGINE_H
#define FUZZ_ENGINE_H
#include "Helpers.h"

class FuzzEngine
{
	public:
		FuzzEngine();
		void SetChaos(DWORD dwNewChaos);
		void SetSeed(PVOID pBuffer, DWORD dwBufferLength);
		void GetMutation(PVOID* pBuffer, DWORD* dwBufferLength);
		void Mutate();
		BOOL HasSeed();
	private:
		void MutatorFlip();
		void MutatorSwap();
		void MutatorReplicate();
		void MutatorInsert();
		void MutatorTruncate();

	private:
		HANDLE hHeap;
		PVOID pSeedBuffer;
		DWORD dwSeedBuffer;

		DWORD dwChaosAmount;

		PVOID pMutationBuffer;
		DWORD dwMutationBufferLength;
};

#endif