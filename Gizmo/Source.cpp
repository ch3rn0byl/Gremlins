// Source.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <Windows.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

//#include "ServiceController.h"
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

void 
PrintBanner()
{
	std::wcout << std::endl;
	std::wcout << "      ,--,   ,---.    ,---.          ,-.    ,-..-. .-.   .---. " << std::endl;
	std::wcout << "    .' .'    | .-.\\   | .-'  |\\    /|| |    |(||  \\| |  ( .-._)" << std::endl;
	std::wcout << "    |  |  __ | `-'/   | `-.  |(\\  / || |    (_)|   | | (_) \\   " << std::endl;
	std::wcout << "    \\  \\ ( _)|   (    | .-'  (_)\\/  || |    | || |\\  | _  \\ \\  " << std::endl;
	std::wcout << "     \\  `-) )| |\\ \\   |  `--.| \\  / || `--. | || | |)|( `-'  ) " << std::endl;
	std::wcout << "     )\\____/ |_| \\)\\  /( __.'| |\\/| ||( __.'`-'/(  (_) `----'  " << std::endl;
	std::wcout << "    (__)         (__)(__)    '-'  '-'(_)      (__)             " << std::endl;
	std::wcout << "                                   A Syscall Introspection Tool" << std::endl;
	std::wcout << std::endl;
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
	INPUT_BUFFER lpInputBuffer{};

	BOOL bIsKdAttached = FALSE;

	PSYSTEM_KERNEL_DEBUGGER_INFORMATION pIsKdAttached = nullptr;

	PrintBanner();

	std::vector<std::string> args(&argv[0], &argv[argc]);

	//
	// Process the arguments given. 
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
	// Now start initializing and interacting with Gremlins.
	//
	try
	{
		pGizmo = std::make_unique<Gizmo>();
	}
	catch (const std::exception&e)
	{
		std::wcerr << "[!] " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	
	//
	// Ensure the target is attached to a kernel debugger. Pause and wait for it to 
	// be attached otherwise data that is intended on being displayed will not show.
	//
	do
	{
		bIsKdAttached = pGizmo->IsBeingKernelDebugged();
		if (!bIsKdAttached)
		{
			std::wcerr << "[!] Not attached to a kernel debugger. Waiting..." << std::endl;
			system("pause");
		}
	} while (!bIsKdAttached);
	
	/*
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
	*/

	//
	// TODO: Use an INI file to store information on what to monitor and what not to.
	//



	if (!HookThese.empty())
	{
		for (const std::string& i : HookThese)
		{
			RtlSecureZeroMemory(&lpInputBuffer, sizeof(lpInputBuffer));

			std::wcout << "[+] Hooking " << i.c_str() << "...";
			if (pGizmo->IsSyscallHooked(i.c_str(), &lpInputBuffer) && !lpInputBuffer.status)
			{
				if (!pGizmo->HookSyscall(&lpInputBuffer))
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
		for (const std::string& i : UnhookThese)
		{
			RtlSecureZeroMemory(&lpInputBuffer, sizeof(lpInputBuffer));

			std::wcout << "[+] Restoring " << i.c_str() << "...";
			if (pGizmo->IsSyscallHooked(i.c_str(), &lpInputBuffer) && lpInputBuffer.status)
			{
				if (!pGizmo->UnhookSyscall(&lpInputBuffer))
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

	std::wcout << "[+] Done." << std::endl;
	return EXIT_SUCCESS;
}


/// EOF