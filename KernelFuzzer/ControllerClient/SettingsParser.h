#pragma once
#ifndef SETTINGS_PARSER_H
#define SETTINGS_PARSER_H
#include <Windows.h>
#include <string>
#include <fstream>
#include <sstream>
#include "ControllerConfig.h"

class SettingsParser
{
	public:
		SettingsParser();
		BOOL SettingsFileExists();
		BOOL CreateSettingsFile();
		BOOL OpenSettingsFile();
		BOOL AddSetting(std::string szStringName, std::string* szStringValue);
		BOOL GetSettingsValue(std::string szStringName, std::string** pszStringValue);
		BOOL CloseSettingsFile();
		BOOL LoadSettings(ControllerConfig* pControllerConfig);

	private:
		std::fstream* hSettingsFile;
};

#endif