#include "OptionHandler.h"
#include <iostream>

OptionHandler::OptionHandler()
	: desc("Allowed Options")
{
	desc.add_options()
		("help,h", "Produce Help Message")
		("input,i", value<std::string>(&m_inputpipe), "Pipe to scan for")
		("output,o", value<std::string>(&m_outputpipe), "Set Output Pipe")
		("processid,p", value<DWORD>(&m_processid), "Set Process ID")
		("load,l", "Load the application into the DLL")
		("unload,u", "Unload the application into the DLL")
		("start", "Start streaming")
		("stop", "Stop streaming")
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

void OptionHandler::load() { std::cout << "In the load function" << std::endl; }

void OptionHandler::unload() { std::cout << "In the unload function" << std::endl; }

void OptionHandler::start() { std::cout << "In the start function" << std::endl; }

void OptionHandler::stop() { std::cout << "In the stop function" << std::endl; }

std::string OptionHandler::getInputPipe() { return m_inputpipe; }

std::string OptionHandler::getOutputPipe() { return m_outputpipe; }

DWORD OptionHandler::getProcessID() { return m_processid; }

ACTION OptionHandler::getAction() { return m_action; }
