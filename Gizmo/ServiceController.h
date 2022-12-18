// ServiceController.h : WIP MAY NOT GET USED AND WILL GET DELETED IF NOT!!
//
#pragma once
#include <Windows.h>
#include <stdexcept>

#include "ErrorHandler.h"

class ServiceController
{
public:
    ServiceController();
    ~ServiceController();

    BOOL IsServiceRunning();
    BOOL StartKernelService();

private:
    std::unique_ptr<ErrorHandler> m_pError;
    const wchar_t* m_wszServiceName;
    SC_HANDLE m_schSCManager;
    SC_HANDLE m_schService;
    BOOL m_bDoesServiceExist;

    void ServiceCleanUp();
};


/// EOF