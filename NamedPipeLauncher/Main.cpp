#include "OptionHandler.h"
#include <tchar.h>
#include <iostream>
#include "Injlib.hxx"

int main(int ac, char* av[])
{
	OptionHandler options;
	std::string libPath;

	TCHAR szLibFile[MAX_PATH];
	GetModuleFileName(NULL, szLibFile, _countof(szLibFile));
	PTSTR pFilename = _tcsrchr(szLibFile, TEXT('\\')) + 1;
	_tcscpy_s(pFilename, _countof(szLibFile) - (pFilename - szLibFile),
#ifndef WIN64
		TEXT("NamedPipeCapture.DLL"));
#else
		TEXT("NamedPipeCapturex64.DLL"));
#endif

	if (!options.parse(ac, av))
		return -1;
	switch (options.getAction())
	{
	case HELP:
		options.help();
		break;
	case LOAD:
	{
		if (!InjectLib(options.getProcessID(), (PCWSTR)szLibFile))
		{
			std::cout << "Inject Library routine failed." << std::endl;
			return -1;
		}
		DWORD initResult = InitializeProcess(options.getProcessID(), options.getInputPipe(), options.getOutputPipe(), options.getClientPort(), options.getServerPort());

		std::cout << "InitializeProcess returned " << initResult << '\n' <<
			"Warning: Make certain the DLL is unloaded when the test is done. \n"
			"If it is not unloaded, the DLL will remain within the process for the life \n"
			"of the process.\n";
		break;
	}
	case UNLOAD:
      if (!CleanUpEverything(options.getProcessID()))
      {
         std::cout << "Cleanup failed. Was the library loaded?\n";
      }
		if (!EjectLib(options.getProcessID(), (PCWSTR)(szLibFile)))
		{
			std::cout << "Library eject failed!\n";
		}
		break;
	default:
		std::cout << "Invalid command\n";
		break;
	}

	std::cout << "Input pipe: " << options.getInputPipe() << std::endl;
	std::cout << "Output pipe: " << options.getOutputPipe() << std::endl;
	std::cout << "Process ID: " << options.getProcessID() << std::endl;
	std::cout << "Client port: " << options.getClientPort() << std::endl;
	std::cout << "Server port: " << options.getServerPort() << std::endl;

	return 0;
}