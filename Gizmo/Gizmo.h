#pragma once
#include <Windows.h>
#include "types.h"
#include "ServiceController.h"

class Gizmo : public ServiceController
{
public:
	Gizmo();
	~Gizmo();

	bool Init();
	bool IsSyscallHooked(LPCSTR lpProcName, PINPUT_BUFFER& InputBuffer);
	bool HookSyscall(PINPUT_BUFFER InputBuffer);
	bool UnhookSyscall(PINPUT_BUFFER InputBuffer);

protected:
private:
	HANDLE hFile;

	bool ResolveSyscall(LPCSTR lpProcName, PUINT16 SyscallValue);
};


/// EOF