#include "SettingsParser.h"

SettingsParser::SettingsParser()
{

}

BOOL SettingsParser::SettingsFileExists()
{
	BOOL bReturnValue = TRUE;

	std::ifstream hTempHandle("./settings.ini");

	if (FALSE == hTempHandle.is_open())
	{
		bReturnValue = FALSE;
		goto end;
	}

	hTempHandle.close();

	end:
	return bReturnValue;

}

BOOL SettingsParser::CreateSettingsFile()
{
	BOOL bReturnValue = TRUE;

	hSettingsFile = new std::fstream("./settings.ini", std::fstream::in | std::fstream::out | std::fstream::app);

	if (FALSE == hSettingsFile->is_open())
	{
		bReturnValue = FALSE;
		goto end;
	}

	end:
	return bReturnValue;
}

BOOL SettingsParser::OpenSettingsFile()
{
	return CreateSettingsFile();
}

BOOL SettingsParser::AddSetting(std::string szStringName, std::string* szStringValue)
{
	BOOL bReturnValue = TRUE;

	std::string szConcatString = szStringName + ":" + *szStringValue + "\r\n";

	if (0 == szStringName.size() || 0 == szStringValue->size())
	{
		bReturnValue = FALSE;
		goto end;
	}

	*hSettingsFile << szConcatString;

	end:
	return bReturnValue;
}

BOOL SettingsParser::CloseSettingsFile()
{
	hSettingsFile->close();

	return !hSettingsFile->is_open();
}

BOOL SettingsParser::LoadSettings(ControllerConfig* pControllerConfig)
{

	GetSettingsValue("LPORT", &pControllerConfig->szControllerPort);
	GetSettingsValue("SLAVES", &pControllerConfig->szProcessorCount);

	return TRUE;
}

BOOL SettingsParser::GetSettingsValue(std::string szStringName, std::string** pszStringValue)
{
	std::string szBufferString;
	std::string lszStringName;
	std::string lszStringValue;

	while (std::getline(*hSettingsFile, szBufferString))
	{
		std::stringstream ssBuffer(szBufferString);
		std::getline(ssBuffer, lszStringName, ':');
		std::getline(ssBuffer, lszStringValue, ':');
		if (szStringName == lszStringName)
		{
			*pszStringValue = new std::string(lszStringValue);
			(*pszStringValue)->erase((*pszStringValue)->size() - 1);
			break;
		}

	}

	return TRUE;
}
