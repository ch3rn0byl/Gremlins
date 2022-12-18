// NtUndoc.h : This file is used in Gizmo.h for resolving undocumented functions.
//
#pragma once
#include "DllHelper.h"
#include "NtUndocTypes.h"

class NtUndoc
{
private:
    DllHelper m_dll{ L"ntdll.dll" };

public:
    _NtQuerySystemInformation NtQuerySystemInformation = m_dll["NtQuerySystemInformation"];
};


/// EOF