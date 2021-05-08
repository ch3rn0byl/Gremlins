#include "ControllerClient.h"
#include "SettingsParser.h"
#include "Helpers.h"
#include <chrono>

ControllerClient::ControllerClient()
{
	pSettingsParser = NULL;
	pControllerConfig = NULL;
	pNetworkClient = NULL;
}

void ControllerClient::Startup()
{
	BOOL bReturnValue = TRUE;

	DevInfo("Starting ControllerClient...");
	pSettingsParser = (SettingsParser *)HeapAlloc(GetProcessHeap(), 0, sizeof(SettingsParser));

	pControllerConfig = (ControllerConfig*)HeapAlloc(GetProcessHeap(), 0, sizeof(ControllerConfig));
	RtlSecureZeroMemory(pControllerConfig, sizeof(ControllerConfig));

	bReturnValue = pSettingsParser->SettingsFileExists();
	if (FALSE == bReturnValue)
	{
		DevError("Settings file does not exist. Generating new one.");

		pSettingsParser->CreateSettingsFile();

		std::cout << "[?] Listening port (Default 5914): ";
		pControllerConfig->szControllerPort = new std::string();
		std::getline(std::cin, *pControllerConfig->szControllerPort);
		if (0 == pControllerConfig->szControllerPort->size())
		{
			*pControllerConfig->szControllerPort = "5914";
		}
		pSettingsParser->AddSetting("LPORT", pControllerConfig->szControllerPort);

		std::cout << "[?] Number of Slave processes (Default 1): ";
		pControllerConfig->szProcessorCount = new std::string();
		std::getline(std::cin, *pControllerConfig->szProcessorCount);
		if (0 == pControllerConfig->szProcessorCount->size())
		{
			*pControllerConfig->szProcessorCount = "1";
		}
		pSettingsParser->AddSetting("SLAVES", pControllerConfig->szProcessorCount);

		pSettingsParser->CloseSettingsFile();
	}

	pSettingsParser->OpenSettingsFile();

	DevInfo("Loading settings file.");
	pSettingsParser->LoadSettings(pControllerConfig);
	
	DevInfo("Startup complete, initializing");

	pNetworkClient = (NetworkClient*)HeapAlloc(GetProcessHeap(), 0, sizeof(NetworkClient));

	return;

}

void ControllerClient::Mainloop()
{
	std::chrono::steady_clock::time_point tpBegin;
	std::chrono::steady_clock::time_point tpEnd;
	std::chrono::duration<double> tpSeconds;
	BOOL bReturnValue = TRUE;
	DWORD dwCycles = 0;
	DWORD dwSlaveCursor = 0;
	DWORD dwSlaveMessageLength = 0;
	PVOID dwSlaveMessage = NULL;
	DWORD dwTotalSeconds = 0;
	DWORD dwSlaveCount = 0;

	pControllerConfig->szRemoteIP = new std::string();
	while (0 == pControllerConfig->szRemoteIP->size())
	{
		std::cout << "[?] Enter the target IP address: ";
		std::getline(std::cin, *pControllerConfig->szRemoteIP);
	}

	std::cout << "[?] Enter the target port (Default 5915): ";
	pControllerConfig->szTargetPort = new std::string();
	std::getline(std::cin, *pControllerConfig->szTargetPort);
	if (0 == pControllerConfig->szTargetPort->size())
	{
		*pControllerConfig->szTargetPort = "5915";
	}

	bReturnValue = pNetworkClient->Setup(pControllerConfig);
	if (bReturnValue == FALSE)
	{
		goto end;
	}

	bReturnValue = pNetworkClient->ConnectToTarget();
	if (bReturnValue == FALSE)
	{
		goto end;
	}

	for (dwSlaveCursor = 0; dwSlaveCursor < std::stoi(*pControllerConfig->szProcessorCount); dwSlaveCursor++)
	{
		SpawnSlave();
	}

	bReturnValue = pNetworkClient->ListenForSlaves(std::stoi(*pControllerConfig->szProcessorCount));
	if (bReturnValue == FALSE)
	{
		goto end;
	}

	for (dwSlaveCursor = 0; dwSlaveCursor < std::stoi(*pControllerConfig->szProcessorCount); dwSlaveCursor++)
	{
		SendSeedToSlave(dwSlaveCursor, (PVOID)"NEWSEED", 7);
	}

	dwSlaveCount = std::stoi(*pControllerConfig->szProcessorCount);

	tpBegin = std::chrono::steady_clock::now();
	while (true)
	{
		for (dwSlaveCursor = 0; dwSlaveCursor < dwSlaveCount; dwSlaveCursor++)
		{
			pNetworkClient->ReceiveFromSlave(dwSlaveCursor);

			if (TRUE == pNetworkClient->SlaveHasMessage(dwSlaveCursor, &dwSlaveMessage, &dwSlaveMessageLength))
			{
				TranslateMessage(dwSlaveCursor, dwSlaveMessage, dwSlaveMessageLength);
				
				// PLACEHOLDER
				pNetworkClient->FlushSlaveBuffer(dwSlaveCursor);
			}
		}


		dwCycles++;
		tpEnd = std::chrono::steady_clock::now();
		tpSeconds = tpEnd - tpBegin;
		if (tpSeconds.count() > 1)
		{
			dwTotalSeconds++;

			tpBegin = std::chrono::steady_clock::now();
			dwCycles = 0;
			printf("[*] Num inputs submitted from slaves: %d (%f/sec)\t\t\t\t\t\t\t\r", 
				dwNumInputsSubmitted, (float)dwNumInputsSubmitted / (float)dwTotalSeconds);

		}

	}

	end:
	return;
}

void ControllerClient::SpawnSlave()
{
	STARTUPINFOA siStartupInfo;
	PROCESS_INFORMATION piProcessInfo;

	RtlSecureZeroMemory(&siStartupInfo, sizeof(STARTUPINFOA));
	RtlSecureZeroMemory(&piProcessInfo, sizeof(PROCESS_INFORMATION));

	std::string szCommandLine = "SlaveClient.exe -p " + *pControllerConfig->szControllerPort;
	CreateProcessA(NULL, (LPSTR)szCommandLine.c_str(), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, 
		NULL, NULL, &siStartupInfo, &piProcessInfo);
}

BOOL ControllerClient::SendSeedToSlave(DWORD dwSlaveId, PVOID pBuffer, DWORD dwBufferLen)
{
	BOOL bReturnValue = TRUE;
	SlaveClient* scSlaveClient = NULL;
	PVOID pBuffer2 = NULL;
	if (NULL == pBuffer || 0 == dwBufferLen)
	{
		bReturnValue = FALSE;
		goto end;
	}

	pBuffer2 = HeapAlloc(GetProcessHeap(), 0, dwBufferLen + 1);
	RtlSecureZeroMemory(pBuffer2, dwBufferLen + 1);
	memcpy((PVOID)((char*)pBuffer2 + 1), pBuffer, dwBufferLen);

	((char*)pBuffer2)[0] = MESSAGE_NEW_SEED;

	bReturnValue = pNetworkClient->SendToSlave(dwSlaveId, pBuffer2, dwBufferLen + 1);

	HeapFree(GetProcessHeap(), 0, pBuffer2);
	
	end:
	return bReturnValue;
}

BOOL ControllerClient::TranslateMessage(DWORD dwSlaveId, PVOID pReceiveBuffer, DWORD dwReceiveBufferLen)
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
		case MESSAGE_NEW_INPUT:
			dwNumInputsSubmitted++;
			break;
	}

end:
	return bReturnValue;
}
