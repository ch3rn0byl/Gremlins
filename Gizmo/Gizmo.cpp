#include "Gizmo.h"
#include <iostream> // delete me
Gizmo::Gizmo()
{
    hFile = INVALID_HANDLE_VALUE;
}

Gizmo::~Gizmo()
{
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }
}

bool Gizmo::Init()
{
    bool bStatus = false;

    INPUT_BUFFER lpInBuffer = { 0 };

    hFile = CreateFile(
        lpFileName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    //
    // Check if driver has been initialized. If it hasn't, initialize
    // dat thang.
    //
    bStatus = DeviceIoControl(
        hFile,
        IsInitialized,
        &lpInBuffer,
        sizeof(lpInBuffer),
        &lpInBuffer,
        sizeof(lpInBuffer),
        NULL,
        NULL
    );
    if (bStatus)
    {
        if (!lpInBuffer.status)
        {
            lpInBuffer.status = false;
            lpInBuffer.syscall = 0;

            bStatus = DeviceIoControl(
                hFile,
                Initialize,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL
            );
            return bStatus;
        }
    }

    return bStatus;
}

bool Gizmo::IsSyscallHooked(LPCSTR lpProcName, PINPUT_BUFFER& InputBuffer)
{
    bool bStatus = ResolveSyscall(lpProcName, &InputBuffer->syscall);
    if (bStatus)
    {
        bStatus = DeviceIoControl(
            hFile,
            IsHooked,
            InputBuffer,
            sizeof(INPUT_BUFFER),
            InputBuffer,
            sizeof(INPUT_BUFFER),
            NULL,
            NULL
        );
    }
    else
    {
        // do something
    }

    return bStatus;
}

bool Gizmo::HookSyscall(PINPUT_BUFFER InputBuffer)
{
    bool bStatus = bStatus = DeviceIoControl(
        hFile,
        Hook,
        InputBuffer,
        sizeof(INPUT_BUFFER),
        NULL,
        NULL,
        NULL,
        NULL
    );
    if (bStatus)
    {

    }
    else
    {

    }

    return bStatus;
}

bool Gizmo::UnhookSyscall(PINPUT_BUFFER InputBuffer)
{
    bool bStatus = DeviceIoControl(
        hFile,
        Unhook,
        InputBuffer,
        sizeof(INPUT_BUFFER),
        NULL,
        NULL,
        NULL,
        NULL
    );
    if (bStatus)
    {

    }
    else
    {

    }

    return bStatus;
}

bool Gizmo::ResolveSyscall(LPCSTR lpProcName, PUINT16 SyscallValue)
{
    HMODULE hModule = NULL;
    FARPROC fpProcName = NULL;

    size_t rcm = 0;

    hModule = GetModuleHandle(L"ntdll.dll");
    if (hModule == NULL)
    {
        return false;
    }

    fpProcName = GetProcAddress(hModule, lpProcName);
    if (fpProcName == NULL)
    {
        FreeLibrary(hModule);
        return false;
    }

    if (*reinterpret_cast<PUINT8>(fpProcName) == 0xe9)
    {
        SetLastError(ERROR_INVALID_MODULETYPE);

        FreeLibrary(hModule);
        return false;
    }

    rcm = RtlCompareMemory(fpProcName, SyscallSig, sizeof(SyscallSig));
    if (rcm == sizeof(SyscallSig))
    {
        *SyscallValue = *reinterpret_cast<PUINT8>(
            reinterpret_cast<PUINT8>(fpProcName) + sizeof(SyscallSig)
            );

        FreeLibrary(hModule);
        hModule = NULL;

        return true;
    }

    FreeLibrary(hModule);
    return false;
}
