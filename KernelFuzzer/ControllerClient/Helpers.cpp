#include "Helpers.h"

void DevError(std::string format, ...)
{
	va_list args;
	char    buffer[512] = { 0 };

	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
	va_end(args);

	//strncat(buffer, "\r", (sizeof(buffer) - strlen(buffer)));
	_DevError(buffer);
}

void DevInfo(std::string format, ...)
{
	va_list args;
	char    buffer[512] = { 0 };

	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
	va_end(args);

	//strncat(buffer, "\r", (sizeof(buffer) - strlen(buffer)));
	_DevInfo(buffer);
}