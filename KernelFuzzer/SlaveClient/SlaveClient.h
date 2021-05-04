#pragma once
#ifndef SLAVE_CLIENT_H
#define SLAVE_CLIENT_H
#include "SlaveNetworkClient.h"
#include <Windows.h>
#include <iostream>
#include "SlaveConfig.h"
#include "ArgumentParser.h"

#define MESSAGE_NEW_SEED '\xAA'

class SlaveClient
{
public:
	SlaveClient();
	void Startup(std::string szArguments);
	void Mainloop();
	BOOL TranslateMessage(PVOID pReceiveBuffer, DWORD dwReceiveBufferLen);
private:
	SlaveConfig* pSlaveConfig;
	SlaveNetworkClient* pNetworkClient;

	PVOID pSeedBuffer;
	DWORD dwSeedBuffer;
};

#endif // SLAVE_CLIENT_H