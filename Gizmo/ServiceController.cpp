#include "ServiceController.h"
#include <iostream> // delete me
void ServiceController::ServiceCleanUp()
{
	SERVICE_STATUS_PROCESS ssp = { 0 };

	DWORD dwBytesNeeded = NULL;
	DWORD dwWaitTime = NULL;

	bool bStatus = false;

	//
	// We should have the handle for the service controller, but 
	// if the service handle is not populated, open a handle to the
	// service with delete, query, and stopping rights.
	// 
	if (schSCManager != NULL && schService == NULL)
	{
		schService = OpenService(
			schSCManager,
			wszServiceName,
			DELETE |
			SERVICE_QUERY_STATUS |
			SERVICE_STOP
		);
	}

	if (schSCManager != NULL && schService != NULL)
	{
		//
		// Check to see if the service is up and running. If so, 
		// send the signal to stop it.
		// 
		bStatus = ControlService(
			schService,
			SERVICE_CONTROL_STOP,
			reinterpret_cast<LPSERVICE_STATUS>(&ssp)
		);
		if (bStatus)
		{
			// 
			// Query the status of the service. If it is stopped, go ahead and 
			// delete; otherwise, wait for the service to stop.
			// 
			if (ssp.dwCurrentState != SERVICE_STOPPED)
			{
				//
				// Keep querying the state of the service until the service has
				// stopped.
				// 
				while (ssp.dwCurrentState != SERVICE_STOPPED)
				{
					Sleep(ssp.dwWaitHint);

					bStatus = QueryServiceStatusEx(
						schService,
						SC_STATUS_PROCESS_INFO,
						reinterpret_cast<LPBYTE>(&ssp),
						sizeof(SERVICE_STATUS_PROCESS),
						&dwBytesNeeded
					);
					if (!bStatus)
					{
						// handle it
						std::wcout << __FUNCTIONW__ << ":" << __LINE__ << std::endl;
					}
				}
			}

			//
			// Delete the service now that it's stopped.
			// 
			//DeleteService(schService);
		}
		else if (!bStatus && GetLastError() == ERROR_SERVICE_NOT_ACTIVE)
		{
			//
			// If the service is not active, this means the service does exist so 
			// we can delete it.
			// 
			//DeleteService(schService);
		}
	}
}

ServiceController::ServiceController()
{
	schSCManager = NULL;
	schService = NULL;

	bStatus = false;
	bDoesServiceExist = true;

	//
	// Grab a handle to the service controller manager.
	//
	schSCManager = OpenSCManager(
		NULL, NULL,
		SC_MANAGER_ALL_ACCESS
	);
	if (schSCManager != NULL)
	{
		bStatus = true;
	}
}

ServiceController::~ServiceController()
{
	//
	// Check if the service is running and if it is, stop it then
	// delete it.
	// 
	/*
	ServiceCleanUp();
	*/

	if (schService != NULL)
	{
		CloseServiceHandle(schService);
		schService = NULL;
	}

	if (schSCManager != NULL)
	{
		CloseServiceHandle(schSCManager);
		schSCManager = NULL;
	}
}

bool ServiceController::IsServiceRunning()
{
	SERVICE_STATUS Status = { 0 };

	schService = OpenService(schSCManager, wszServiceName, SC_MANAGER_ALL_ACCESS);
	if (schService == NULL)
	{
		if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
		{
			bDoesServiceExist = false;
		}

		return false;
	}

	if (!QueryServiceStatus(schService, &Status))
	{
		return false;
	}

	if (Status.dwCurrentState == SERVICE_RUNNING)
	{
		return true;
	}

	return false;
}

bool ServiceController::StartKernelService()
{
	// 
	// Check if the service exists. If it doesn't, create dat hoe.
	//
	if (!bDoesServiceExist)
	{
		schService = CreateService(
			schSCManager,
			wszServiceName,
			wszServiceName,
			GENERIC_EXECUTE | DELETE,
			SERVICE_KERNEL_DRIVER,
			SERVICE_DEMAND_START,
			SERVICE_ERROR_NORMAL,
			L"C:\\Users\\offensive\\Desktop\\Gremlins.sys", // TODO: dynamically fix dis hoe.
			NULL, NULL, NULL, NULL, NULL
		);
		if (schService == NULL)
		{
			return false;
		}
	}
	else
	{
		schService = OpenService(schSCManager, wszServiceName, SC_MANAGER_ALL_ACCESS);
		if (schService == NULL)
		{
			return false;
		}
	}


	if (!StartService(schService, NULL, NULL))
	{
		return false;
	}

	return true;
}


/// EOF