#pragma once
#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H
#define WIN32_LEAN_AND_MEAN
#include <string>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include "Helpers.h"
#include "ControllerConfig.h"

#pragma comment(lib, "ws2_32.lib")

typedef struct
{
	SOCKET hSocket;
	struct sockaddr* pSockAddress;
	PVOID pBuffer;
	DWORD dwBufferLen;
} SlaveClient;

typedef struct
{
	SOCKET hSocket;
	struct addrinfo* pSockAddress;
} TargetClient;

class NetworkClient
{
	public:
		NetworkClient();
		BOOL Setup(ControllerConfig* pControllerConfig);
		BOOL ListenForSlaves(DWORD dwSlaveCount);
		BOOL ConnectToTarget();
		BOOL ReceiveFromSlave(DWORD dwSlaveId);
		BOOL SlaveHasMessage(DWORD dwSlaveId, PVOID* pBuffer, DWORD* dwBufferLen);
		BOOL FlushSlaveBuffer(DWORD dwSlaveId);
		BOOL SendToSlave(DWORD dwSlaveId, PVOID pBuffer, DWORD dwBufferLen);

	private:
		DWORD dwNumConnections;
		SOCKET hListenSocket;
		SlaveClient** rgpSlaves;
		TargetClient* pTarget;
};

#endif