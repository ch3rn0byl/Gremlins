#pragma once
#ifndef CONTROLLER_CLIENT_H
#define CONTROLLER_CLIENT_H
#include "NetworkClient.h"
#include <Windows.h>
#include <iostream>
#include "SettingsParser.h"
#include "ControllerConfig.h""

#define MESSAGE_NEW_SEED         '\xAA'
#define MESSAGE_CLOSING          '\xAB'
#define MESSAGE_NEW_INPUT        '\xAC'
#define MESSAGE_SUCCESSFUL_CRASH '\xAD'
#define MESSAGE_FAILED_CRASH     '\xAE'

class ControllerClient
{
	public:
		ControllerClient();
		void Startup();
		void Mainloop();
		void SpawnSlave();
		BOOL SendSeedToSlave(DWORD dwSlaveId, PVOID pBuffer, DWORD dwBufferLen);
		BOOL TranslateMessage(DWORD dwSlaveId, PVOID pReceiveBuffer, DWORD dwReceiveBufferLen);

	private:
		SettingsParser* pSettingsParser;
		ControllerConfig* pControllerConfig;
		NetworkClient* pNetworkClient;
		DWORD dwNumInputsSubmitted;
};

#endif // CONTROLLER_CLIENT_H