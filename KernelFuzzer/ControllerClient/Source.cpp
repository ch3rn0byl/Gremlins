#include "ControllerClient.h"

int main()
{
	ControllerClient ccNewClient;
	ccNewClient.Startup();
	ccNewClient.Mainloop();

	return 0;
}