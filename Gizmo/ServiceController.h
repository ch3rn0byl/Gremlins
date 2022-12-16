#pragma once
#include <Windows.h>

constexpr auto wszServiceName = L"Gremlins";

class ServiceController
{
private:
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    bool bStatus;
    bool bDoesServiceExist;

    /// <summary>
    /// This private method will unregister the services created
    /// and delete the driver that was dumped to disk.
    /// </summary>
    void ServiceCleanUp();

public:
    ServiceController();
    ~ServiceController();

    bool IsServiceRunning();

    /// <summary>
    /// This method will create a service on the machine and 
    /// start the driver.
    /// </summary>
    /// <returns>true if successful</returns>
    bool StartKernelService();
};


/// EOF