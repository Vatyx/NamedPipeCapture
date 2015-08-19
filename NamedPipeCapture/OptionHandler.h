#ifndef OPTION_HANDLER_H
#define OPTION_HANDLER_H

#include <boost/program_options.hpp>

#include <string>
#include <Windows.h>

using namespace boost::program_options;

enum ACTION
{
	NONE,
	HELP,
	LOAD,
	UNLOAD,
	START,
	STOP
};

class OptionHandler
{
public:
	OptionHandler();

	bool parse(int ac, char* av[]);
	bool validate();

	void help();
	void load();
	void unload();
	void start();
	void stop();

	std::string getInputPipe();
	std::string getOutputPipe();
	DWORD getProcessID();
	ACTION getAction();

private:
	options_description desc;

	std::string m_inputpipe = "";
	std::string m_outputpipe = "";
	DWORD m_processid = 0;

	ACTION m_action = NONE;
};

class IncorrectNumOfOptionsException
	: public std::exception
{
	virtual const char* what() const throw()
	{
		return "An incorrect number of options was entered. Enter only one instance of --load, --unload, --start, --stop.";
	}
};

#endif