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

	pNetworkClient = (SlaveNetworkClient*)HeapAlloc(GetProcessHeap(), 0, sizeof(SlaveNetworkClient));
}

void SlaveClient::Mainloop()
{
	DWORD dwControllerMessageLength = 0;
	PVOID pControllerMessage = NULL;

	pNetworkClient->Setup(pSlaveConfig);

	pNetworkClient->ConnectToController();

	pNetworkClient->ReceiveFromController();

	while (1)
	{
		pNetworkClient->ReceiveFromController();

		if (TRUE == pNetworkClient->ControllerHasMessage(&pControllerMessage, &dwControllerMessageLength))
		{

			TranslateMessage(pControllerMessage, dwControllerMessageLength);

			// PLACEHOLDER
			pNetworkClient->FlushControllerBuffer();
		}
	}
}

BOOL SlaveClient::TranslateMessage(PVOID pReceiveBuffer, DWORD dwReceiveBufferLen)
{
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
			DevInfo("Received new seed request from controller.");
			break;
	}

	end:
	return bReturnValue;
}