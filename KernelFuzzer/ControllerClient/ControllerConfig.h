#pragma once
#ifndef CONTROLLER_CONFIG_H
#define CONTROLLER_CONFIG_H
#include <Windows.h>
#include <string>

typedef struct
{
	std::string* szControllerPort;
	std::string* szTargetPort;
	std::string* szProcessorCount;
	std::string* szRemoteIP;
} ControllerConfig;

#endif // CONTROLLER_CONFIG_H