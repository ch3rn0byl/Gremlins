#pragma once
#ifndef CONTROLLER_CLIENT_H
#define CONTROLLER_CLIENT_H
#include "NetworkClient.h"
#include <Windows.h>
#include <iostream>
#include "SettingsParser.h"
#include "ControllerConfig.h""

#define MESSAGE_NEW_SEED '\xAA'

class ControllerClient
{
	public:
		ControllerClient();
		void Startup();
		void Mainloop();
		void SpawnSlave();
		BOOL SendSeedToSlave(DWORD dwSlaveId, PVOID pBuffer, DWORD dwBufferLen);

	private:
		SettingsParser* pSettingsParser;
		ControllerConfig* pControllerConfig;
		NetworkClient* pNetworkClient;
};

#endif // CONTROLLER_CLIENT_H