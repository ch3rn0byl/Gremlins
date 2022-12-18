#include "ServiceController.h"

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
	if (m_schSCManager != NULL && m_schService == NULL)
	{
		m_schService = OpenService(
			m_schSCManager,
			m_wszServiceName,
			DELETE |
			SERVICE_QUERY_STATUS |
			SERVICE_STOP
		);
	}

	if (m_schSCManager != NULL && m_schService != NULL)
	{
		//
		// Check to see if the service is up and running. If so, 
		// send the signal to stop it.
		// 
		bStatus = ControlService(
			m_schService,
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
						m_schService,
						SC_STATUS_PROCESS_INFO,
						reinterpret_cast<LPBYTE>(&ssp),
						sizeof(SERVICE_STATUS_PROCESS),
						&dwBytesNeeded
					);
					if (!bStatus)
					{
						m_pError = std::make_unique<ErrorHandler>(GetLastError());
						throw std::runtime_error(m_pError->GetLastErrorAsStringA());
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

ServiceController::ServiceController() : 
	m_pError(nullptr),
	m_wszServiceName(L"Gremlins"),
	m_schSCManager(NULL),
	m_schService(NULL),
	m_bDoesServiceExist(TRUE)
{
	//
	// Grab a handle to the service controller manager.
	//
	m_schSCManager = OpenSCManager(
		NULL, NULL,
		SC_MANAGER_ALL_ACCESS
	);
	if (m_schSCManager == NULL)
	{
		m_pError = std::make_unique<ErrorHandler>(GetLastError());
		throw std::runtime_error(m_pError->GetLastErrorAsStringA());
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

	if (m_schService != NULL)
	{
		CloseServiceHandle(m_schService);
		m_schService = NULL;
	}

	if (m_schSCManager != NULL)
	{
		CloseServiceHandle(m_schSCManager);
		m_schSCManager = NULL;
	}
}

BOOL ServiceController::IsServiceRunning()
{
	SERVICE_STATUS Status = { 0 };

	m_schService = OpenService(m_schSCManager, m_wszServiceName, SC_MANAGER_ALL_ACCESS);
	if (m_schService == NULL)
	{
		if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
		{
			m_bDoesServiceExist = FALSE;
		}

		return FALSE;
	}

	if (!QueryServiceStatus(m_schService, &Status))
	{
		return FALSE;
	}

	if (Status.dwCurrentState == SERVICE_RUNNING)
	{
		return TRUE;
	}

	return FALSE;
}

BOOL ServiceController::StartKernelService()
{
	// 
	// Check if the service exists. If it doesn't, create dat hoe.
	//
	if (!m_bDoesServiceExist)
	{
		m_schService = CreateService(
			m_schSCManager,
			m_wszServiceName,
			m_wszServiceName,
			GENERIC_EXECUTE | DELETE,
			SERVICE_KERNEL_DRIVER,
			SERVICE_DEMAND_START,
			SERVICE_ERROR_NORMAL,
			L"C:\\Users\\offensive\\Desktop\\Gremlins.sys", // TODO: dynamically fix dis hoe.
			NULL, NULL, NULL, NULL, NULL
		);
		if (m_schService == NULL)
		{
			return FALSE;
		}
	}
	else
	{
		m_schService = OpenService(m_schSCManager, m_wszServiceName, SC_MANAGER_ALL_ACCESS);
		if (m_schService == NULL)
		{
			return FALSE;
		}
	}


	if (!StartService(m_schService, NULL, NULL))
	{
		return FALSE;
	}

	return TRUE;
}


/// EOF