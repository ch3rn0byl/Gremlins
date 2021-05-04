#include "SlaveClient.h"

int main(int argc, char** argv)
{
	std::string szArguments;
	SlaveClient scNewClient;

	szArguments = argv[0];
	for (int iArgument = 1; iArgument < argc; iArgument++)
	{
		szArguments += " " + std::string(argv[iArgument]);
	}

	scNewClient.Startup(szArguments);
	scNewClient.Mainloop();

	return 0;
}