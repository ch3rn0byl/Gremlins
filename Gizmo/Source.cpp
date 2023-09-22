// Source.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <Windows.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <map>

#include "Gizmo.h"
#include "NtUndoc.h"
#include "ArgumentParser.h"

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
	std::wcout << "  -h, --hook\t\tHooks a syscall by name." << std::endl;
	std::wcout << "  -r, --restore\t\tUnhooks a syscall by name." << std::endl;
	std::wcout << "  -d, --deny\t\tDenies problematic processes from being hooked." << std::endl;
	std::wcout << "  -i, --initial-run\tSets up information in the kernel based on gremlins.ini." << std::endl;
	std::wcout << "  -a, --analyze\t\tAnalyzes the kernel for targeted devices." << std::endl;

	std::wcout << "\nThis application will crash (hopefully) the system. A kernel debugger ";
	std::wcout << "is required to be attached to a system to catch the crash as it occurs. ";
	std::wcout << std::endl;
}

std::map<std::string, std::vector<std::string>> ParseIniFile(const std::string& IniFilename)
{
	std::ifstream IniFile(IniFilename);

	if (!IniFile.is_open())
	{
		return {};
	}

	std::map<std::string, std::vector<std::string>> IniData;
	std::string line;
	std::string CurrentSection;

	//
	// Begin parsing the ini file to extract image names and what not.
	//
	while (std::getline(IniFile, line))
	{
		if (!line.empty())
		{
			line.erase(0, line.find_first_not_of(' '));
			line.erase(line.find_last_not_of(' ') + 1);

			//
			// If it's commented out, ignore it. Not interested in it.
			//
			if (line.front() == ';')
			{
				continue;
			}
			else if (line.front() == '[' && line.back() == ']')
			{
				CurrentSection = line.substr(1, line.size() - 2);
				IniData[CurrentSection] = {};
			}
			else if (!CurrentSection.empty() && !line.empty())
			{
				IniData[CurrentSection].push_back(line);
			}
		}
	}

	return IniData;
}

void 
PrintBanner()
{
	std::wcout << "                                                                                " << std::endl;
	std::wcout << "          (#########(**                                     ,,((((((((((/        " << std::endl;
	std::wcout << "       ##((((((((((((,,,,,****  *********,,,,,,,,,,  ,,,,.....////////////((     " << std::endl;
	std::wcout << "     #((((((((((((((((,,,,***&&&&&&&&/***,,,,,,,,,,,,,,,,....////////////////(   " << std::endl;
	std::wcout << "    #  /((((((((((((((/,,/&&&&&&&&&&&&&&*,,,,,,,,,,,,,,,,,..////////////////  (  " << std::endl;
	std::wcout << "          (((((((((((((//&&&&&#@.**.##&&&,,,,##.**.@#,,,,,,*/////////////        " << std::endl;
	std::wcout << "          ((((((((((((((&&&&&#@.*,.*.#&&%(*,,#.*.,*.@#,,,,,//////////////        " << std::endl;
	std::wcout << "            ((((((((((((&&&&&&&#####&####(((((,(####,,,,,,,////////////          " << std::endl;
	std::wcout << "              (((((((((((&&&&&&&&&#####/////(((((,,,,,,,,,///////////            " << std::endl;
	std::wcout << "                          *&&******&#####((((((&,,,,,,,,,                        " << std::endl;
	std::wcout << "                             *******&&&&&&&&&&&,,,,,,,                           " << std::endl;
	std::wcout << "                             ***,,,,&&&&&&&&%%%,,,.,,,                           " << std::endl;
	std::wcout << "                            *******,&&&&&%%%%%%,,,,,,,,                          " << std::endl;
	std::wcout << "                            &&*,****&&&&&%%%%%%,,,,.,%%                          " << std::endl;
	std::wcout << "                              &,***&&&&&&%%%%%%%,,,,%                            " << std::endl;
	std::wcout << "                                       &&&%%                                     " << std::endl;
	std::wcout << "                            Gremlins: A Syscall Fuzzer" << std::endl;
	std::wcout << "                                                 " << std::endl;
	std::wcout << std::endl;
}

int
main(
	int argc,
	char* argv[]
)
{
	std::unique_ptr<Gizmo> pGizmo = nullptr;

	std::vector<std::string> HookThese{};
	std::vector<std::string> UnhookThese{};
	std::vector<std::string> ExcludeTheseDrivers{};
	std::vector<std::string> AnalyzeImages{};

	INPUT_BUFFER lpInputBuffer{};

	BOOL bIsKdAttached = FALSE;
	BOOL bIsInitialRun = FALSE;

	PSYSTEM_KERNEL_DEBUGGER_INFORMATION pIsKdAttached = nullptr;

	//
	// Gizmo!! :D
	//
	PrintBanner();

	ArgumentParser parser(argc, argv);

	
	/*
	ArgumentParser parser(argc, argv);

	if (argc == 1)
	{
		PrintUsage(argv[0]);
		return EXIT_SUCCESS;
	}

	if (parser.Contains("-h") || parser.Contains("--hook"))
	{
		std::wcout << "hooking: ";
		auto values = parser.Get("-h");
		for (const auto& value : values)
		{
			std::wcout << value.c_str() << std::endl;
		}
		std::wcout << std::endl;
	}

	if (parser.Contains("-r") || parser.Contains("--restore"))
	{
		std::wcout << "restoring: ";
		auto values = parser.Get("-r");
		for (const auto& value : values)
		{
			std::wcout << value.c_str() << std::endl;
		}
		std::wcout << std::endl;
	}

	if (parser.Contains("-e") || parser.Contains("--exclude"))
	{
		std::wcout << "excluding: ";
		auto values = parser.Get("-e");
		for (const auto& value : values)
		{
			std::wcout << value.c_str() << std::endl;
		}
		std::wcout << std::endl;
	}

	if (parser.Contains("-a") || parser.Contains("--analyze"))
	{
		std::wcout << "analyzing: ";
		auto values = parser.Get("-a");
		for (const auto& value : values)
		{
			std::wcout << value.c_str() << std::endl;
		}
		std::wcout << std::endl;
	}

	if (parser.Contains("-i") || parser.Contains("--input-run"))
	{
		std::wcout << "initial run" << std::endl;
	}
	*/

	/*
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
		else if (argument == "--initial-run" || argument == "-i")
		{
			std::map<std::string, std::vector<std::string>> GizmoIniData = ParseIniFile("gremlins.ini");

			//
			// If this is empty, this means the ini file was not found or there was an error parsing it.
			//
			if (GizmoIniData.empty())
			{
				std::wcerr << "[!] Failed to parse gremlins.ini. Exiting..." << std::endl;
				return EXIT_FAILURE;
			}

			bIsInitialRun = TRUE; 

			for (const auto& section : GizmoIniData)
			{
				if (section.first == "exclude_drivers")
				{
					for (const auto& data : section.second)
					{
						ExcludeTheseDrivers.push_back(data);
					}
				}
				else if (section.first == "analyze_images")
				{
					for (const auto& data : section.second)
					{
						AnalyzeImages.push_back(data);
					}
				}
				else if (section.first == "hook")
				{
					for (const auto& data : section.second)
					{
						HookThese.push_back(data);
					}
				}
			}
		}
		else if (argument == "-a" || argument == "--analyze")
		{
			try
			{
				for (std::vector<std::string>::const_iterator params = i + 1; params != args.end(); params++)
				{
					std::string image = *params;

					AnalyzeImages.push_back(image);
				}
			}
			catch (const std::exception&)
			{
				std::wcerr << "[!] Provide a module's function name(s) to hook." << std::endl;
				return EXIT_FAILURE;
			}
		}
	}
	*/

	/*
	bIsInitialRun = parser.isFlagSet("--initial-run");

	auto hooks = parser.getFlagArguments("--hook");
	for (const auto& hook : hooks)
	{
		std::wcout << "[+] Going to hook " << hook.c_str() << std::endl;
		HookThese.push_back(hook);
	}

	auto restores = parser.getFlagArguments("--restore");
	for (const auto& restore : restores)
	{
		std::wcout << "[+] Going to unhook " << restore.c_str() << std::endl;
		UnhookThese.push_back(restore);
	}

	auto denies = parser.getFlagArguments("--deny");
	for (const auto& deny : denies)
	{
		std::wcout << "[+] Going to deny " << deny.c_str() << std::endl;
		ExcludeTheseDrivers.push_back(deny);
	}

	auto analyzes = parser.getFlagArguments("--analyze");
	for (const auto& analyze : analyzes)
	{
		std::wcout << "[+] Going to analyze " << analyze.c_str() << std::endl;
		AnalyzeImages.push_back(analyze);
	}
	*/

	//
	// Validate user input.
	//
/*
	if (!bIsInitialRun && HookThese.empty() && UnhookThese.empty())
	{
		PrintUsage(argv[0]);
		return EXIT_FAILURE;
	}
	*/

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

	auto hooks = parser.getFlagArguments("--hook");
	for (const auto& hook : hooks)
	{
		HookThese.push_back(hook);
	}

	auto restores = parser.getFlagArguments("--restore");
	for (const auto& restore : restores)
	{
		UnhookThese.push_back(restore);
	}

	auto denies = parser.getFlagArguments("--deny");
	for (const auto& deny : denies)
	{
		ExcludeTheseDrivers.push_back(deny);
	}

	auto args = parser.getFlagArguments("--analyze");
	for (const auto& arg : args)
	{
		AnalyzeImages.push_back(arg);
	}


	//if (bIsInitialRun)
	if (parser.isFlagSet("--initial-run"))
	{
		std::map<std::string, std::vector<std::string>> GizmoIniData = ParseIniFile("gremlins.ini");

		//
		// If this is empty, this means the ini file was not found or there was an error parsing it.
		//
		if (GizmoIniData.empty())
		{
			std::wcerr << "[!] Failed to parse gremlins.ini. Exiting..." << std::endl;
			return EXIT_FAILURE;
		}

		for (const auto& section : GizmoIniData)
		{
			if (section.first == "exclude_drivers")
			{
				for (const auto& data : section.second)
				{
					ExcludeTheseDrivers.push_back(data);
				}
			}
			else if (section.first == "analyze_images")
			{
				for (const auto& data : section.second)
				{
					AnalyzeImages.push_back(data);
				}
			}
			else if (section.first == "hook")
			{
				for (const auto& data : section.second)
				{
					HookThese.push_back(data);
				}
			}
		}

		//
		// TODO: Fix these and make it cleaner. 
		//
		std::wcout << "[+] Setting up environment." << std::endl;
		if (!ExcludeTheseDrivers.empty())
		{
			std::wcout << "[+] Going to exclude ";
			for (const auto& image : ExcludeTheseDrivers)
			{
				std::wcout << image.c_str() << " ";
				std::wstring DriverName(image.begin(), image.end());

				std::unique_ptr<NtUndoc> pNtUndoc = std::make_unique<NtUndoc>();

				UNICODE_STRING DriverNameUnicode{};

				pNtUndoc->RtlInitUnicodeString(&DriverNameUnicode, DriverName.c_str());

				if (!pGizmo->ExcludeDriverImage(&DriverNameUnicode))
				{
					std::wcout << "[!] " << pGizmo->what() << std::endl;
				}
			}
		}

		if (!AnalyzeImages.empty())
		{
			std::wcout << "[+] Going to analyze ";
			for (const auto& image : AnalyzeImages)
			{
				std::wcout << image.c_str() << " ";
				std::wstring DriverName(image.begin(), image.end());

				std::unique_ptr<NtUndoc> pNtUndoc = std::make_unique<NtUndoc>();

				UNICODE_STRING DriverNameUnicode{};

				pNtUndoc->RtlInitUnicodeString(&DriverNameUnicode, DriverName.c_str());

				if (!pGizmo->AnalyzeDriverImage(&DriverNameUnicode))
				{
					std::wcout << "[!] " << pGizmo->what() << std::endl;
				}
			}
		}
	}

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