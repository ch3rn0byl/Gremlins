#pragma once
#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H
#define WIN32_LEAN_AND_MEAN
#include <string>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include "Helpers.h"
#include "SlaveConfig.h"

#pragma comment(lib, "ws2_32.lib")

class SlaveNetworkClient
{
	public:
		SlaveNetworkClient();
		BOOL Setup(SlaveConfig* pControllerConfig);
		BOOL ConnectToController();
		BOOL ReceiveFromController();
		BOOL ControllerHasMessage(PVOID* pBuffer, DWORD* dwBufferLen);
		BOOL FlushControllerBuffer();

	private:
		DWORD dwNumConnections;
		SOCKET hControllerSocket;
		PVOID pReceiveBuffer;
		DWORD dwReceiveBufferLen;
		struct addrinfo* pSockAddress;
};

#endif