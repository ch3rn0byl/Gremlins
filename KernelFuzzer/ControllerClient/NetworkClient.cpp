#include "NetworkClient.h"

NetworkClient::NetworkClient()
{
	rgpSlaves = NULL;
	pTarget = NULL;
	hListenSocket = INVALID_SOCKET;
	dwNumConnections = 0;
}

BOOL NetworkClient::Setup(ControllerConfig* pControllerConfig)
{
	WSADATA wsaData;
	BOOL bReturnValue = TRUE;
	int iReturnValue = 0;
	struct addrinfo aiTargetHints;
	struct addrinfo aiListenHints;
	struct addrinfo* aiTargetResult = NULL;
	struct addrinfo* aiListenResult = NULL;

	if (0 == pControllerConfig->szControllerPort->size() ||
		0 == pControllerConfig->szRemoteIP->size() ||
		0 == pControllerConfig->szTargetPort->size())
	{
		DevError("Unable to setup NetworkClient, NULL parameter.");
		bReturnValue = FALSE;
		goto end;
	}

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	RtlSecureZeroMemory(&aiTargetHints, sizeof(struct addrinfo));
	aiTargetHints.ai_family = AF_UNSPEC;
	aiTargetHints.ai_socktype = SOCK_STREAM;
	aiTargetHints.ai_protocol = IPPROTO_TCP;

	RtlSecureZeroMemory(&aiListenHints, sizeof(struct addrinfo));
	aiListenHints.ai_family = AF_INET;
	aiListenHints.ai_socktype = SOCK_STREAM;
	aiListenHints.ai_protocol = IPPROTO_TCP;

	// Target
	getaddrinfo(pControllerConfig->szRemoteIP->c_str(), 
		        pControllerConfig->szTargetPort->c_str(), 
		        &aiTargetHints, 
		        &aiTargetResult);

	// Listen
	getaddrinfo(NULL,  
		        pControllerConfig->szControllerPort->c_str(), 
		        &aiListenHints, 
		        &aiListenResult);

	// Prepare target client information
	pTarget = (TargetClient*)HeapAlloc(GetProcessHeap(), 0, sizeof(TargetClient));

	pTarget->hSocket = socket(aiTargetResult->ai_family, 
		                      aiTargetResult->ai_socktype, 
		                      aiTargetResult->ai_protocol);

	pTarget->pSockAddress = aiTargetResult;

	hListenSocket = socket(aiListenResult->ai_family, 
		                   aiListenResult->ai_socktype, 
		                   aiListenResult->ai_protocol);

	iReturnValue = bind(hListenSocket, aiListenResult->ai_addr, aiListenResult->ai_addrlen);
	iReturnValue = listen(hListenSocket, SOMAXCONN);

	end:
	return bReturnValue;
}

BOOL NetworkClient::ListenForSlaves(DWORD dwSlaveCount)
{
	BOOL bReturnValue = TRUE;
	DWORD dwSlaveCursor = 0;
	SOCKET hClientSocket = INVALID_SOCKET;
	SlaveClient* scSlaveClient = NULL;
	struct sockaddr* saSlaveSockAddr = NULL;
	DWORD iMode = 1;
	int iSockAddrLen = 0;

	rgpSlaves = (SlaveClient**)HeapAlloc(GetProcessHeap(), 0, dwSlaveCount * sizeof(SlaveClient*));
	for (dwSlaveCursor = 0; dwSlaveCursor < dwSlaveCount; dwSlaveCursor++)
	{
		saSlaveSockAddr = (struct sockaddr*)HeapAlloc(GetProcessHeap(), 0, sizeof(struct sockaddr));
		RtlSecureZeroMemory(saSlaveSockAddr, sizeof(struct sockaddr));

		hClientSocket = accept(hListenSocket, NULL, NULL);
		if (SOCKET_ERROR == hClientSocket)
		{
			DevError("Unable to accept on listening socket: 0x%.16llx", WSAGetLastError());
			bReturnValue = FALSE;
			goto end;
		}

		scSlaveClient = (SlaveClient*)HeapAlloc(GetProcessHeap(), 0, sizeof(SlaveClient));

		DevInfo("Slave #%d connected!", dwSlaveCursor);

		scSlaveClient->hSocket = hClientSocket;
		scSlaveClient->pSockAddress = saSlaveSockAddr;
		scSlaveClient->pBuffer = HeapAlloc(GetProcessHeap(), 0, 8096);
		scSlaveClient->dwBufferLen = 0;

		ioctlsocket(hClientSocket, FIONBIO, &iMode);

		rgpSlaves[dwSlaveCursor] = scSlaveClient;
	}

	DevInfo("All slaves have connected.");

	end:
	return TRUE;
}

BOOL NetworkClient::ConnectToTarget()
{
	BOOL bReturnValue = TRUE;
	int iReturnValue = 0;
	
	iReturnValue = connect(pTarget->hSocket, pTarget->pSockAddress->ai_addr, pTarget->pSockAddress->ai_addrlen);
	if (ERROR_SUCCESS != iReturnValue)
	{
		DevError("Unable to connect to remote server.");
		bReturnValue = FALSE;
		goto end;
	}

	DevInfo("Connected to remote server!");

	end:
	return bReturnValue;
}

BOOL NetworkClient::ReceiveFromSlave(DWORD dwSlaveId)
{
	BOOL bReturnValue = TRUE;
	SlaveClient* scSlaveClient = NULL;
	DWORD dwMaxLen = 8096;
	DWORD dwRecvLen = 0;

	if (dwSlaveId > dwNumConnections - 1)
	{
		bReturnValue = FALSE;
		goto end;
	}

	scSlaveClient = rgpSlaves[dwSlaveId];

	if (NULL == scSlaveClient)
	{
		bReturnValue = FALSE;
		goto end;
	}

	dwRecvLen = recv(scSlaveClient->hSocket, (char*)scSlaveClient->pBuffer, 8096, 0);
	if (SOCKET_ERROR == dwRecvLen)
	{
		bReturnValue = FALSE;
		goto end;
	}

	scSlaveClient->dwBufferLen = dwRecvLen;

	end:
	return bReturnValue;
}

BOOL NetworkClient::SlaveHasMessage(DWORD dwSlaveId, PVOID* pBuffer, DWORD* dwBufferLen)
{
	BOOL bReturnValue = TRUE;
	SlaveClient* scSlaveClient = NULL;

	if (NULL == pBuffer || NULL == dwBufferLen)
	{
		bReturnValue = FALSE;
		goto end;
	}

	if (dwSlaveId > dwNumConnections - 1)
	{
		bReturnValue = FALSE;
		goto end;
	}

	scSlaveClient = rgpSlaves[dwSlaveId];

	if (scSlaveClient->dwBufferLen == 0)
	{
		bReturnValue = FALSE;
		goto end;
	}

	*pBuffer = scSlaveClient->pBuffer;
	*dwBufferLen = scSlaveClient->dwBufferLen;


end:
	return bReturnValue;
}

BOOL NetworkClient::FlushSlaveBuffer(DWORD dwSlaveId)
{
	BOOL bReturnValue = TRUE;
	SlaveClient* scSlaveClient = NULL;

	if (dwSlaveId > dwNumConnections - 1)
	{
		bReturnValue = FALSE;
		goto end;
	}

	scSlaveClient = rgpSlaves[dwSlaveId];

	scSlaveClient->dwBufferLen = 0;
	RtlSecureZeroMemory(scSlaveClient->pBuffer, 8096);

	end:
	return bReturnValue;
}

BOOL NetworkClient::SendToSlave(DWORD dwSlaveId, PVOID pBuffer, DWORD dwBufferLen)
{
	BOOL bReturnValue = TRUE;
	SlaveClient* scSlaveClient = NULL;

	if (dwSlaveId > dwNumConnections - 1)
	{
		bReturnValue = FALSE;
		goto end;
	}

	scSlaveClient = rgpSlaves[dwSlaveId];

	send(scSlaveClient->hSocket, (char*)pBuffer, dwBufferLen, 0);

	end:
	return bReturnValue;
}
