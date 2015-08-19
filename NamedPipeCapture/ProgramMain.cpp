#include "OptionHandler.h"

#include <iostream>

int main(int ac, char* av[])
{
	OptionHandler options;

	if (!options.parse(ac, av))
		return -1;
	switch (options.getAction())
	{
	case HELP:
		options.help();
		break;
	case LOAD:
		options.load();
		break;
	case UNLOAD:
		options.unload();
		break;
	case START:
		options.start();
		break;
	case STOP:
		options.stop();
		break;
	default:
		std::cout << "Invalid command";
		break;
	}

	std::cout << "Input pipe:" << options.getInputPipe() << std::endl;
	std::cout << "Output pipe:" << options.getOutputPipe() << std::endl;
	std::cout << "Processid:" << options.getProcessID() << std::endl;

	return 0;
}

//class OptionHandler
//{
//public:
//	OptionHandler()
//	{
//		desc("Allowed options");
//		desc.add_options()
//			("help,h", "Produce Help Message")
//			("input,i", po::value<string>(), "Set Input Pipe")
//			("output,o", po::value<string>(), "Set Output Pipe")
//			("processid,p", po::value<string>(), "Set Process ID")
//			("load,l", "Load the application into the DLL")
//			("unload,u", "Unload the application into the DLL")
//			("start", "Start streaming")
//			("stop", "Stop streaming")
//			("dump,d", po::value<string>(), "Dump data to specified file")
//			;
//	}
//
//	void setInputPipe(string ip)
//	{
//		m_inputpipe = ip;
//		b_inputpipe = true;
//	}
//
//	void setOutputPipe(string op)
//	{
//		m_outputpipe = op;
//		b_outputpipe = true;
//	}
//
//	void setProcessID(string pid)
//	{
//		m_processid = pid;
//		b_processid = true;
//	}
//
//	void load()
//	{
//		if (b_inputpipe && b_outputpipe && b_processid)
//		{
//			//check to see if input pipe and processid are valid
//			//load the application
//			b_load = true;
//		}
//		else
//		{
//			if (!b_inputpipe)
//				std::cout << "input pipe ";
//			if (!b_outputpipe)
//				std::cout << "output pipe ";
//			if (!b_processid)
//				std::cout << "process id ";
//			std::cout << "have no value" << endl;
//		}
//	}
//
//	void unload()
//	{
//		if (b_load)
//		{
//			//unload the application
//			b_load = false;
//		}
//		else
//		{
//			std::cout << "The application has not been loaded" << endl;
//		}
//	}
//
//	void start()
//	{
//		if (b_load)
//		{
//			//start the application
//			b_start = true;
//		}
//	}
//
//	void stop()
//	{
//		if (b_start)
//		{
//			//start the application
//			b_start = false;
//		}
//		else
//		{
//			std::cout << "The application has not started" << endl;
//		}
//	}
//
//	void dump(string fileName)
//	{
//		//output to the specified file.
//	}
//
//private:
//	po::options_description desc;
//
//	string m_inputpipe = "";
//	string m_outputpipe = "";
//	string m_processid = "";
//
//	bool b_inputpipe = false;
//	bool b_outputpipe = false;
//	bool b_processid = false;
//
//	bool b_load = false;
//	bool b_start = false;
//	
//};

