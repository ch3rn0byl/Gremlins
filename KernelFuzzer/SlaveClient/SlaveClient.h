#pragma once
#ifndef SLAVE_CLIENT_H
#define SLAVE_CLIENT_H
#include "SlaveNetworkClient.h"
#include <Windows.h>
#include <iostream>
#include "SlaveConfig.h"
#include "ArgumentParser.h"
#include "FuzzEngine.h"

#define MESSAGE_NEW_SEED         '\xAA'
#define MESSAGE_CLOSING          '\xAB'
#define MESSAGE_NEW_INPUT        '\xAC'
#define MESSAGE_SUCCESSFUL_CRASH '\xAD'
#define MESSAGE_FAILED_CRASH     '\xAE'

class SlaveClient
{
public:
	SlaveClient();
	void Startup(std::string szArguments);
	void Mainloop();
	BOOL TranslateMessage(PVOID pReceiveBuffer, DWORD dwReceiveBufferLen);
	BOOL SendMutationToController();
private:
	SlaveConfig* pSlaveConfig;
	SlaveNetworkClient* pNetworkClient;
	FuzzEngine* pFuzzEngine;
};

#endif // SLAVE_CLIENT_H