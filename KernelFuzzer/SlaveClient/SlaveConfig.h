#pragma once
#ifndef SLAVE_CONFIG_H
#define SLAVE_CONFIG_H
#include <Windows.h>
#include <string>

typedef struct
{
	std::string* szControllerPort;
} SlaveConfig;

#endif // SLAVE_CONFIG_H