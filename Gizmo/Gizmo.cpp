// Gizmo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <Windows.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

#include "undocumented.h"
#include "banner.h"

wchar_t* what()
{
	LPWSTR lpwMessageBuffer = nullptr;

	DWORD dwSize = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpwMessageBuffer,
		0,
		NULL
	);

	return lpwMessageBuffer;
}

bool ResolveSyscall(LPCSTR lpProcName, uint16_t* ui8SyscallValue)
{
	HMODULE hModule = GetModuleHandle(L"ntdll.dll");
	if (hModule == NULL)
	{
		return false;
	}

	///
	/// The second parameter for GetProcAddress, lpProcName, is case-sensitive!!
	/// 
	FARPROC fpProcName = GetProcAddress(hModule, lpProcName);
	if (fpProcName == NULL)
	{
		FreeLibrary(hModule);
		return false;
	}

	///
	/// Check to see if the function address is hooked. If it is, error because
	/// it's hooked by the host's antivirus
	/// 
	if (*reinterpret_cast<uint8_t*>(fpProcName) == 0xe9)
	{
		SetLastError(ERROR_INVALID_MODULETYPE);

		FreeLibrary(hModule);
		return false;
	}

	if (RtlCompareMemory(fpProcName, SyscallSig, sizeof(SyscallSig)) == sizeof(SyscallSig))
	{
		*ui8SyscallValue = *reinterpret_cast<uint8_t*>(
			reinterpret_cast<uint64_t>(fpProcName) + sizeof(SyscallSig)
			);

		FreeLibrary(hModule);
		return true;
	}

	///
	/// If we reach here, this means everything else passed, but was not able to find the signature or
	/// its just not letting me read memory for whatever reason. Set the last error to point to error 127:
	/// The specified procedure could not be found
	/// 
	SetLastError(ERROR_PROC_NOT_FOUND);

	FreeLibrary(hModule);
	return false;
}

template <typename T>
T GetSystemInfo(SYSTEM_INFORMATION_CLASS InformationClass)
{
	ULONG SystemInformationLength = 0;
	T SystemInformationReturn = nullptr;

	GET_NATIVE(NtQuerySystemInformation);

	do
	{
		PBYTE SystemInformation = new BYTE[SystemInformationLength];

		NTSTATUS Status = f_NtQuerySystemInformation(
			InformationClass,
			SystemInformation,
			SystemInformationLength,
			&SystemInformationLength
		);
		if (Status == STATUS_INFO_LENGTH_MISMATCH)
		{
			delete[] SystemInformation;
		}
		else
		{
			SystemInformationReturn = reinterpret_cast<T>(SystemInformation);
			break;
		}
	} while (true);

	return SystemInformationReturn;
}

bool isServiceRunning(PDWORD dwStatus)
{
	SERVICE_STATUS status = { 0 };

	SC_HANDLE scServiceController = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ENUMERATE_SERVICE);
	if (scServiceController == NULL)
	{
		return false;
	}

	SC_HANDLE hService = OpenService(scServiceController, ServiceName, GENERIC_READ);
	if (hService == NULL)
	{
		CloseServiceHandle(scServiceController);
		return false;
	}

	if (!QueryServiceStatus(hService, &status))
	{
		CloseServiceHandle(hService);
		CloseServiceHandle(scServiceController);
		return false;
	}

	*dwStatus = status.dwCurrentState;

	CloseServiceHandle(hService);
	CloseServiceHandle(scServiceController);

	return true;
}

int main(int argc, const char* argv[])
{
	INPUT_BUFFER lpInputBuffer;

	uint32_t ui32IsInitialized = 0;

	DWORD dwServiceStatus = 0;

	std::vector<std::string> hookThese;
	std::vector<std::string> unhookThese;

	std::string BinaryName = argv[0];

	std::wcout << banner << std::endl;

	std::vector<std::string> args(&argv[0], &argv[argc]);
	for (std::vector<std::string>::iterator i = args.begin(); i != args.end(); i++)
	{
		std::string argument = *i;

		if (argument == "--hook" || argument == "-h")
		{
			try
			{
				for (std::vector<std::string>::const_iterator params = i + 1; params != args.end(); params++)
				{
					std::string function = *params;

					///
					/// Grab all the arguments up until we reach the unhook argument
					/// 
					if (function == "-r" || function == "--restore")
						break;

					hookThese.push_back(function);
				}
			}
			catch (const std::exception&)
			{
				std::wcerr << "[!] Provide a module's function name(s) to hook." << std::endl;
				return EXIT_FAILURE;
			}
		}
		else if (argument == "--restore" || argument == "-r")
		{
			try
			{
				for (std::vector<std::string>::const_iterator params = i + 1; params != args.end(); params++)
				{
					std::string function = *params;

					///
					/// Grab all the arguments up until we reach the unhook argument
					/// 
					if (function == "-h" || function == "--hook")
						break;

					unhookThese.push_back(function);
				}
			}
			catch (const std::exception&)
			{
				std::wcerr << "[!] Provide a module's function name(s) to hook." << std::endl;
				return EXIT_FAILURE;
			}
		}
	}

	/// 
	/// Validate user input
	/// 
	if (hookThese.empty() && unhookThese.empty())
	{
		std::wcerr << "[!] Usage: " << argv[0] << "-h/--hook function names || -r/--restore function names" << std::endl;
		std::wcerr << "    " << argv[0] << " --hook NtCreateFile NtReadFile NtWriteFile" << std::endl;
		std::wcerr << "    " << argv[0] << " --restore NtWriteFile NtReadFile" << std::endl;
		std::wcerr << "    " << argv[0] << " -h NtCreateFile NtReadFile -r NtWriteFile" << std::endl;
		std::wcerr << "[!] These functions are case sensitive!" << std::endl;
		return EXIT_FAILURE;
	}

	PSYSTEM_KERNEL_DEBUGGER_INFORMATION pIsDebugged = GetSystemInfo<PSYSTEM_KERNEL_DEBUGGER_INFORMATION>(
		SYSTEM_INFORMATION_CLASS::SystemKernelDebuggerInformation
		);
	if (pIsDebugged != NULL)
	{
		if (!pIsDebugged->DebuggerEnabled || pIsDebugged->DebuggerNotPresent)
		{
			std::wcerr << "[!] Not connected to a kernel debugger! Pausing..." << std::endl;
			system("pause");
		}
		delete pIsDebugged;
	}
	else
	{
		std::wcerr << "[!] Unable to retrieve debugging status." << std::endl;
		return EXIT_FAILURE;
	}

	///
	/// Check to see if the service exists or if it is running. 
	/// 
	if (!isServiceRunning(&dwServiceStatus) || dwServiceStatus == 0)
	{
		std::wcerr << "[!] " << what();
		return EXIT_FAILURE;
	}

	if (dwServiceStatus == SERVICE_STOPPED)
	{
		std::wcerr << "[!] Gremlins is not running. Please start it." << std::endl;
		return EXIT_FAILURE;
	}

	std::wcout << "[+] Grabbing handle to gremlins...";
	HANDLE hFile = CreateFile(
		lpFileName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		std::wcerr << "uh-oh!" << std::endl;
		std::wcerr << "[!] " << what();
		return EXIT_FAILURE;
	}
	else
	{
		std::wcout << "got it." << std::endl;
	}

	bool bStatus = DeviceIoControl(
		hFile,
		IOCTL_FUNCTION::IsInitialized,
		&ui32IsInitialized,
		sizeof(ui32IsInitialized),
		&ui32IsInitialized,
		sizeof(ui32IsInitialized),
		NULL,
		NULL
	);
	if (!bStatus)
	{
		std::wcerr << "unable to. " << std::endl;
		std::wcerr << "[!] DeviceIoControl failed: " << GetLastError() << std::endl;
		CloseHandle(hFile);

		return EXIT_FAILURE;
	}

	if (ui32IsInitialized == NULL)
	{
		std::wcout << "[+] Initializing gremlins...";
		if (!DeviceIoControl(
			hFile,
			IOCTL_FUNCTION::Initialize,
			//(LPVOID)argv[0], 
			&BinaryName[0],
			//static_cast<DWORD>(std::strlen(argv[0])), 
			static_cast<DWORD>(BinaryName.size()),
			NULL, 
			NULL, 
			NULL, 
			NULL
		))
		{
			std::wcerr << "unable to. " << std::endl;
			std::wcerr << "[!] DeviceIoControl failed: " << GetLastError() << std::endl;
			CloseHandle(hFile);

			return EXIT_FAILURE;
		}
		else
		{
			std::wcout << "done." << std::endl;
		}
	}

	if (!hookThese.empty())
	{
		for (std::string i : hookThese)
		{
			///
			/// Dynamically resolve ntdll!* syscall
			/// 
			if (!ResolveSyscall(i.c_str(), &lpInputBuffer.syscall) || lpInputBuffer.syscall == 0)
			{
				std::wcerr << "[!] " << what();
				continue;
			}
									
			std::wcout << "[+] Hooking " << i.c_str() << "...";

			bool bStatus = DeviceIoControl(
				hFile, 
				IOCTL_FUNCTION::IsHooked, 
				&lpInputBuffer,
				sizeof(lpInputBuffer),
				&lpInputBuffer,
				sizeof(lpInputBuffer),
				NULL, 
				NULL
			);
			if (bStatus && !lpInputBuffer.status)
			{
				if (!DeviceIoControl(
					hFile,
					IOCTL_FUNCTION::Hook,
					&lpInputBuffer,
					sizeof(lpInputBuffer),
					NULL,
					NULL,
					NULL,
					NULL
				))
				{
					std::wcerr << "uh-oh!" << std::endl;
					std::wcerr << "[!] Unable to hook syscall " << lpInputBuffer.syscall << "." << std::endl;
				}
				else
				{
					std::wcout << "done. Syscall: " << lpInputBuffer.syscall << std::endl;
				}
			}
			else
			{
				std::wcerr << "uh-oh!" << std::endl;
				std::wcerr << "[!] " << i.c_str() << " is already hooked." << std::endl;
			}
		}
	}

	if (!unhookThese.empty())
	{
		for (std::string i : unhookThese)
		{
			///
			/// Dynamically resolve ntdll!* syscall
			/// 
			if (!ResolveSyscall(i.c_str(), &lpInputBuffer.syscall) || lpInputBuffer.syscall == 0)
			{
				std::wcerr << "[!] " << what();
				continue;
			}

			std::wcout << "[+] Restoring " << i.c_str() << "...";
			bool bStatus = DeviceIoControl(
				hFile, 
				IOCTL_FUNCTION::IsHooked,
				&lpInputBuffer,
				sizeof(lpInputBuffer),
				&lpInputBuffer,
				sizeof(lpInputBuffer),
				NULL,
				NULL
			);
			if (bStatus && lpInputBuffer.status)
			{
				if (!DeviceIoControl(
					hFile,
					IOCTL_FUNCTION::Unhook,
					&lpInputBuffer,
					sizeof(lpInputBuffer),
					NULL,
					NULL,
					NULL,
					NULL
				))

				{
					std::wcerr << "uh-oh!" << std::endl;
					std::wcerr << "[!] Unable to restore syscall " << lpInputBuffer.syscall << "." << std::endl;
				}
				else
				{
					std::wcout << "done." << std::endl;
				}
			}
			else
			{
				std::wcerr << "uh-oh!" << std::endl;
				std::wcerr << "[!] " << i.c_str() << " is not hooked. Nothing to do." << std::endl;
			}
		}
	}

	CloseHandle(hFile);

	return EXIT_SUCCESS;
}


/// EOF