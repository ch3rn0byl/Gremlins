// Gizmo.h : This file is the driver for Gremlins. All Gremlin interaction happens here. Used in main().
//
#pragma once
#include <Windows.h>
#include <stdexcept>

#include "types.h"
#include "ErrorHandler.h"
#include "BaseDriverclient.h"
#include "NtUndoc.h"

class Gizmo : 
	public BaseDriverClient
{
public:
	/// <summary>
	/// The constructor is responsible for grabbing the handle to Gremlins
	/// to interact with the kernel. Once that is done, it will then 
	/// initialize Gremlins to begin introspection and/or fuzzing (when implemented).
	/// </summary>
	Gizmo();
	~Gizmo();

	/// <summary>
	/// This method will query the system for the machine's kernel debugging 
	/// information via NtQuerySystemInformation. Two things will be checked:
	///   1. Is debugging enabled.
	///   2. Is a debugger present.
	/// </summary>
	/// <returns>TRUE. If debugging is not enabled or present, will return FALSE.</returns>
	BOOL IsBeingKernelDebugged();

	/// <summary>
	/// This method will check if a chosen syscall is hooked.
	/// </summary>
	/// <param name="lpProcName"></param>
	/// <param name="lpInputBuffer"></param>
	/// <returns>true. If this method fails, reference GetLastError().</returns>
	bool IsSyscallHooked(
		_In_z_ LPCSTR lpProcName, 
		_Inout_ PINPUT_BUFFER lpInputBuffer
	);

	/// <summary>
	/// This method will hook the syscall via Gremlins.
	/// </summary>
	/// <param name="lpInputBuffer"></param>
	/// <returns>true. If this method fails, reference GetLastError().</returns>
	bool HookSyscall(
		_In_ PINPUT_BUFFER lpInputBuffer
	);

	/// <summary>
	/// This method gives the capability of unhooking the syscall.
	/// </summary>
	/// <param name="lpInputBuffer"></param>
	/// <returns>true. If this method fails, reference GetLastError().</returns>
	bool UnhookSyscall(
		_In_ PINPUT_BUFFER lpInputBuffer
	);

	// TODO: Make this method do all the work for conversion. intake the ascii version!
	bool ExcludeDriverImage(
		_In_ PUNICODE_STRING DriverName
	);

	// TODO: Make this method robust!
	bool AnalyzeDriverImage(
		_In_ PUNICODE_STRING DriverName
	);

	/// <summary>
	/// If an error should occur, this method will return what happened
	/// in a std::wstring format.
	/// </summary>
	/// <returns>std::wstring</returns>
	std::wstring what();

protected:
	std::unique_ptr<NtUndoc> m_pNtUndoc;

	/// <summary>
	/// This method will resolve the syscall value by using GetProcAddress
	/// to resolve a function. It will scan for the syscall signature and if
	/// it was found, outputs it into the SyscallValue var.
	/// 
	/// This method will only fail if the module was not found or if the syscall
	/// is already hooked. If it is hooked already, this indicates an AV is 
	/// present on the system.
	/// </summary>
	/// <param name="lpProcName"></param>
	/// <param name="SyscallValue"></param>
	/// <returns>true. May return ERROR_MOD_NOT_FOUND, ERROR_INVALID_MODULETYPE, or 
	/// another value that can be checked via GetLastError() upon failure.</returns>
	bool ResolveSyscall(
		_In_z_ LPCSTR lpProcName, 
		_Out_ PUINT16 SyscallValue
	);

private:
	const wchar_t* m_wszServiceName;
	std::unique_ptr<ErrorHandler> m_pError;
};


/// EOF