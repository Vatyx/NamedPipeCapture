#include "OptionHandler.h"
#include "Injlib.hxx"
#include <tchar.h>
#include <iostream>

OptionHandler::OptionHandler()
	: desc("Allowed Options")
{
	desc.add_options()
		("help,h", "Produce Help Message")
		("input,i", value<std::string>(&m_inputpipe), "Pipe to scan for (In the form \\[Pipe Name Here])")
		("output,o", value<std::string>(&m_outputpipe), "Set Output Pipe (In the form \\\\.\\pipe\\[Pipe Name Here])")
		("processid,p", value<DWORD>(&m_processid), "Set Process ID")
		("clientport,c", value<unsigned short>(&m_clientPort), "Set Client Port (Default is 0)")
		("serverport,s", value<unsigned short>(&m_serverPort), "Set Server Port (Default is 0)")
		("load,l", "Load the DLL into the application and open the output named pipe")
		("unload,u", "Unload the DLL from the application and close the output named pipe")
		;
}

bool OptionHandler::parse(int ac, char* av[])
{
	variables_map vm;
	try
	{
		store(parse_command_line(ac, av, desc), vm);
		notify(vm);

		if (vm.count("help"))
		{
			if (m_action == ACTION::NONE)
				m_action = ACTION::HELP;
			else
				throw IncorrectNumOfOptionsException();
		}
		if (vm.count("load"))
		{
			if (m_action == ACTION::NONE)
				m_action = ACTION::LOAD;
			else
				throw IncorrectNumOfOptionsException();
		}
		if (vm.count("unload"))
		{
			if (m_action == ACTION::NONE)
				m_action = ACTION::UNLOAD;
			else
				throw IncorrectNumOfOptionsException();
		}
		if (vm.count("start"))
		{
			if (m_action == ACTION::NONE)
				m_action = ACTION::START;
			else
				throw IncorrectNumOfOptionsException();
		}
		if (vm.count("stop"))
		{
			if (m_action == ACTION::NONE)
				m_action = ACTION::STOP;
			else
				throw IncorrectNumOfOptionsException();
		}
		return (m_action == ACTION::HELP) ? true : validate();
	}
	catch (std::exception& e)
	{
		std::cout << "Parsing error: " << e.what();
		std::cout << desc;
		return false;
	}
}

bool OptionHandler::validate()
{
	if (m_inputpipe.empty() || m_outputpipe.empty() || m_processid == 0)
	{
		std::cout << "Input pipe, output pipe or process id are not defined."
			<< std::endl;
		;
		return false;
	}
	return true;
}

void OptionHandler::help() { std::cout << desc << std::endl; }

std::string OptionHandler::getInputPipe() { return m_inputpipe; }

std::string OptionHandler::getOutputPipe() { return m_outputpipe; }

DWORD OptionHandler::getProcessID() { return m_processid; }

ACTION OptionHandler::getAction() { return m_action; }

unsigned short OptionHandler::getClientPort() { return m_clientPort; }

unsigned short OptionHandler::getServerPort() { return m_serverPort; }