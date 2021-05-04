#pragma once
#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H
#include <Windows.h>
#include <string>
#include <sstream>

class ArgumentParser
{
	public:
		ArgumentParser();
		std::string GetArgument(std::string szArguments, std::string szArgumentValue);
	private:
};

#endif // ARGUMENT_PARSER_H