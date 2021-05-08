#include "SlaveNetworkClient.h"

SlaveNetworkClient::SlaveNetworkClient()
{
	dwNumConnections = 0;
	hControllerSocket = INVALID_SOCKET;
	pSockAddress = NULL;
	dwReceiveBufferLen = 0;
	pReceiveBuffer = NULL;
}

BOOL SlaveNetworkClient::Setup(SlaveConfig* pControllerConfig)
{
	WSADATA wsaData;
	BOOL bReturnValue = TRUE;
	int iReturnValue = 0;
	struct addrinfo aiTargetHints;
	struct addrinfo aiListenHints;
	struct addrinfo* aiTargetResult = NULL;
	struct addrinfo* aiListenResult = NULL;

	if (0 == pControllerConfig->szControllerPort->size())
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

	// Target
	getaddrinfo("127.0.0.1",
		pControllerConfig->szControllerPort->c_str(),
		&aiTargetHints,
		&aiTargetResult);

	hControllerSocket = socket(aiTargetResult->ai_family,
		aiTargetResult->ai_socktype,
		aiTargetResult->ai_protocol);

	pSockAddress = aiTargetResult;

	pReceiveBuffer = HeapAlloc(GetProcessHeap(), 0, 8096);
	RtlSecureZeroMemory(pReceiveBuffer, 8096);

end:
	return bReturnValue;
}

BOOL SlaveNetworkClient::ConnectToController()
{
	DWORD iMode = 1;
	BOOL bReturnValue = TRUE;
	int iReturnValue = 0;

	iReturnValue = connect(hControllerSocket, pSockAddress->ai_addr, pSockAddress->ai_addrlen);
	if (ERROR_SUCCESS != iReturnValue)
	{
		DevError("Unable to connect to remote server.");
		bReturnValue = FALSE;
		goto end;
	}

	ioctlsocket(hControllerSocket, FIONBIO, &iMode);

	DevInfo("Connected to remote server!");

end:
	return bReturnValue;
}

BOOL SlaveNetworkClient::ReceiveFromController()
{
	BOOL bReturnValue = TRUE;
	DWORD dwMaxLen = 8096;
	DWORD dwRecvLen = 0;

	dwRecvLen = recv(hControllerSocket, (char*)pReceiveBuffer, 8096, 0);
	if (SOCKET_ERROR == dwRecvLen)
	{
		bReturnValue = FALSE;
		goto end;
	}

	dwReceiveBufferLen = dwRecvLen;

end:
	return bReturnValue;
}

BOOL SlaveNetworkClient::ControllerHasMessage(PVOID* pBuffer, DWORD* dwBufferLen)
{
	BOOL bReturnValue = TRUE;

	if (NULL == pBuffer || NULL == dwBufferLen)
	{
		bReturnValue = FALSE;
		goto end;
	}

	if (dwReceiveBufferLen == 0)
	{
		bReturnValue = FALSE;
		goto end;
	}

	*pBuffer = pReceiveBuffer;
	*dwBufferLen = dwReceiveBufferLen;

	end:
	return bReturnValue;
}

BOOL SlaveNetworkClient::FlushControllerBuffer()
{
	BOOL bReturnValue = TRUE;

	dwReceiveBufferLen = 0;
	RtlSecureZeroMemory(pReceiveBuffer, 8096);

end:
	return bReturnValue;
}

BOOL SlaveNetworkClient::SendToController(PVOID pBuffer, DWORD dwBufferLen)
{
	BOOL bReturnValue = TRUE;
	INT iReturnValue = 0;
	INT IWSAError = 0;

	if (INVALID_SOCKET == hControllerSocket)
	{
		bReturnValue = FALSE;
		goto end;
	}

	iReturnValue = send(hControllerSocket, (char*)pBuffer, dwBufferLen, 0);
	if (SOCKET_ERROR == iReturnValue)
	{
		bReturnValue = FALSE;
	}

end:
	return bReturnValue;
}