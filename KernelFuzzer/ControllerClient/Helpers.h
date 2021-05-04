#pragma once
#ifndef HELPERS_H
#define HELPERS_H
#include <Windows.h>
#include <string>
#include <iostream>

#define _DevError(msg); \
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),12); \
	std::cout << "[X] " << msg << std::endl; \
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15); \

#define _DevInfo(msg); \
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),11);  \
	std::cout << "[+] " << msg << std::endl; \
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),15);  \

void DevError(std::string format, ...); //Writes an error message to the console [ERROR]
void DevInfo(std::string format, ...); //Writes an info message to the console [INFO]
void DevDrop(std::string format, ...); //Writes a dropdown message to the console, used to continue info messages [----]

#endif