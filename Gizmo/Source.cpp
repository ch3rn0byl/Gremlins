// Source.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <Windows.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

#include "undocumented.h"
#include "banner.h"

#include "ServiceController.h"
#include "Gizmo.h"

void
PrintUsage(
	_In_ const char* pszProgramName
)
{
	std::wcout << "Usage: " << pszProgramName << " options syscall(s)" << std::endl;

	std::wcout << pszProgramName << " can be used for hooking all of the syscalls on a ";
	std::wcout << "target. You can hook one syscall at a time, or hook multiple syscalls ";
	std::wcout << "at a time. The syscalls are case-sensitive!" << std::endl;

	std::wcout << "\nOptions: " << std::endl;
	std::wcout << "  -h, --hook\tHooks a syscall by name." << std::endl;
	std::wcout << "  -r, --restore\tUnhooks a syscall by name." << std::endl;

	std::wcout << "\nThis application will crash (hopefully) the system. A kernel debugger ";
	std::wcout << "is required to be attached to a system to catch the crash as it occurs. ";
	std::wcout << std::endl;
}

template <typename T>
T GetKdInformation(
	SYSTEM_INFORMATION_CLASS InformationClass
)
{
	ULONG SystemInformationLength = 0;
	T SystemInformationReturn = nullptr;

	GET_NATIVE(NtQuerySystemInformation);

	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	do
	{
		T SystemInformation = reinterpret_cast<T>(
			new BYTE[SystemInformationLength]()
			);

		Status = f_NtQuerySystemInformation(
			InformationClass,
			SystemInformation,
			SystemInformationLength,
			&SystemInformationLength
		);
		if (!NT_SUCCESS(Status) && Status == STATUS_INFO_LENGTH_MISMATCH)
		{
			if (SystemInformation != nullptr)
			{
				delete[] SystemInformation;
				SystemInformation = nullptr;
			}
		}
		else
		{
			SystemInformationReturn = reinterpret_cast<T>(SystemInformation);
			break;
		}
	} while (Status != STATUS_SUCCESS);

	return SystemInformationReturn;
}

int
main(
	int argc,
	const char* argv[]
)
{
	std::unique_ptr<Gizmo> pGizmo = nullptr;

	std::vector<std::string> HookThese{};
	std::vector<std::string> UnhookThese{};

	PSYSTEM_KERNEL_DEBUGGER_INFORMATION pIsKdAttached = nullptr;
	PINPUT_BUFFER lpInputBuffer = nullptr;

	pGizmo = std::make_unique<Gizmo>();

	std::wcout << banner << std::endl;

	std::vector<std::string> args(&argv[0], &argv[argc]);

	//
	// Iterate through all the arguments. 
	//
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

					//
					// Grab all the arguments up until we reach the unhook argument
					// 
					if (function == "-r" || function == "--restore")
						break;

					HookThese.push_back(function);
				}
			}
			catch (const std::exception&)
			{
				std::wcerr << "[!] Provide a syscall(s) to hook." << std::endl;
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

					//
					// Grab all the arguments up until we reach the unhook argument
					// 
					if (function == "-h" || function == "--hook")
						break;

					UnhookThese.push_back(function);
				}
			}
			catch (const std::exception&)
			{
				std::wcerr << "[!] Provide a module's function name(s) to hook." << std::endl;
				return EXIT_FAILURE;
			}
		}
	}

	//
	// Validate user input.
	//
	if (HookThese.empty() && UnhookThese.empty())
	{
		PrintUsage(argv[0]);
		return EXIT_FAILURE;
	}

	//
	// Ensure the target is attached to a kernel debugger. Pause and wait for it
	// to be attached.
	// 
	do
	{
		pIsKdAttached = GetKdInformation<PSYSTEM_KERNEL_DEBUGGER_INFORMATION>(
			SystemKernelDebuggerInformation
			);
		if (pIsKdAttached != nullptr)
		{
			if (!pIsKdAttached->DebuggerEnabled || pIsKdAttached->DebuggerNotPresent)
			{
				std::wcerr << "[!] Not connected to a kernel debugger!" << std::endl;
				system("pause");
			}
			else
			{
				break;
			}
		}
		else
		{
			std::wcerr << "[!] Unable to retrieve debugging status." << std::endl;
			return EXIT_FAILURE;
		}
	} while (true);

	//
	// The kernel debugger should be attached at this point. Start the driver up.
	// 
	if (!pGizmo->IsServiceRunning())
	{
		if (!pGizmo->StartKernelService())
		{
			std::wcerr << "[!] Unable to start service." << std::endl;
			return EXIT_FAILURE;
		}
	}

	if (!pGizmo->Init())
	{
		std::wcerr << "[!] Unable to initialize gremlins." << std::endl;
		return EXIT_FAILURE;
	}

	if (!HookThese.empty())
	{
		lpInputBuffer = new INPUT_BUFFER();
		if (lpInputBuffer == nullptr)
		{
			//
			// We shouldn't be in here but whatevs. Just in case.
			//
			return EXIT_FAILURE;
		}

		for (const std::string& i : HookThese)
		{
			RtlSecureZeroMemory(lpInputBuffer, sizeof(INPUT_BUFFER));

			std::wcout << "[+] Hooking " << i.c_str() << "...";
			if (pGizmo->IsSyscallHooked(i.c_str(), lpInputBuffer) && !lpInputBuffer->status)
			{
				if (!pGizmo->HookSyscall(lpInputBuffer))
				{
					std::wcerr << "uh-oh!" << std::endl;
					std::wcerr << "[!] Unable to hook syscall " << i.c_str() << std::endl;
				}
				else
				{
					std::wcout << "done." << std::endl;
				}
			}
			else
			{
				std::wcerr << "uh-oh!" << std::endl;
				std::wcerr << "[!] " << i.c_str() << " is hooked already." << std::endl;
			}
		}
	}

	if (!UnhookThese.empty())
	{
		lpInputBuffer = new INPUT_BUFFER();
		if (lpInputBuffer == nullptr)
		{
			//
			// We shouldn't be in here but whatevs. Just in case.
			//
			return EXIT_FAILURE;
		}

		for (const std::string& i : UnhookThese)
		{
			RtlSecureZeroMemory(lpInputBuffer, sizeof(INPUT_BUFFER));

			std::wcout << "[+] Restoring " << i.c_str() << "...";
			if (pGizmo->IsSyscallHooked(i.c_str(), lpInputBuffer) && lpInputBuffer->status)
			{
				if (!pGizmo->UnhookSyscall(lpInputBuffer))
				{
					std::wcerr << "uh-oh!" << std::endl;
					std::wcerr << "[!] Unable to hook syscall " << i.c_str() << std::endl;
				}
				else
				{
					std::wcout << "done." << std::endl;
				}
			}
			else
			{
				std::wcerr << "uh-oh!" << std::endl;
				std::wcerr << "[!] " << i.c_str() << " is not hooked." << std::endl;
			}
		}
	}

	if (pIsKdAttached != nullptr)
	{
		delete pIsKdAttached;
		pIsKdAttached = nullptr;
	}

	std::wcout << "[+] Done." << std::endl;
	return EXIT_SUCCESS;
}


/// EOF