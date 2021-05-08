#include "SlaveClient.h"

SlaveClient::SlaveClient()
{

}

void SlaveClient::Startup(std::string szArguments)
{
	ArgumentParser apParser;
	std::string szPortNumber;
	szPortNumber = apParser.GetArgument(szArguments, "-p");

	pSlaveConfig = (SlaveConfig*)HeapAlloc(GetProcessHeap(), 0, sizeof(SlaveConfig));
	pSlaveConfig->szControllerPort = new std::string(szPortNumber);

	pNetworkClient = new SlaveNetworkClient;

	pFuzzEngine = new FuzzEngine;
}

void SlaveClient::Mainloop()
{
	DWORD dwControllerMessageLength = 0;
	PVOID pControllerMessage = NULL;

	pNetworkClient->Setup(pSlaveConfig);

	pNetworkClient->ConnectToController();

	while (1)
	{
		pNetworkClient->ReceiveFromController();

		if (TRUE == pNetworkClient->ControllerHasMessage(&pControllerMessage, &dwControllerMessageLength))
		{
			TranslateMessage(pControllerMessage, dwControllerMessageLength);

			pNetworkClient->FlushControllerBuffer();
		}

		SendMutationToController();
	}
}

BOOL SlaveClient::TranslateMessage(PVOID pReceiveBuffer, DWORD dwReceiveBufferLen)
{
	DWORD dwDataLen = 0;
	BOOL bReturnValue = TRUE;
	CHAR cMessageType;

	if (NULL == pReceiveBuffer || 0 == dwReceiveBufferLen)
	{
		bReturnValue = FALSE;
		goto end;
	}

	cMessageType = ((char*)pReceiveBuffer)[0];
	switch (cMessageType)
	{
		case MESSAGE_NEW_SEED:
			pFuzzEngine->SetSeed((CHAR*)pReceiveBuffer + 1, dwReceiveBufferLen - 1);
			DevInfo("Received new seed request from controller.");
			break;
	}

	end:
	return bReturnValue;
}

BOOL SlaveClient::SendMutationToController()
{
	BOOL bReturnValue = TRUE;
	SlaveClient* scSlaveClient = NULL;
	CHAR* pBuffer2 = NULL;
	PVOID pMutationBuffer = NULL;
	DWORD dwMutationBuffer = 0;
	int i = 0;

	if (FALSE == pFuzzEngine->HasSeed())
	{
		bReturnValue = FALSE;
		goto end;
	}

	pFuzzEngine->SetChaos(10);

	pFuzzEngine->Mutate();

	pFuzzEngine->GetMutation(&pMutationBuffer, &dwMutationBuffer);

	pBuffer2 = (CHAR * )HeapAlloc(GetProcessHeap(), 0, dwMutationBuffer + 1);
	memcpy(pBuffer2 + 1, pMutationBuffer, dwMutationBuffer);

	((char*)pBuffer2)[0] = MESSAGE_NEW_INPUT;

	bReturnValue = pNetworkClient->SendToController(pBuffer2, dwMutationBuffer + 1);

	HeapFree(GetProcessHeap(), 0, pBuffer2);

end:
	return bReturnValue;
}
