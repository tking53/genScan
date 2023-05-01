#include <string>
#include <iostream>

#include "genscan.hpp"
#include "ArgParser.hpp"

#ifdef USE_SPDLOG
	#include <spdlog/spdlog.h>
	#include <spdlog/cfg/env.h>
	#include <spdlog/fmt/ostr.h>
	#include <spdlog/sinks/basic_file_sink.h>
	#include <spdlog/sinks/stdout_color_sinks.h>
#endif


int main(int argc, char *argv[]) {
	#ifdef USE_SPDLOG
		auto console = spdlog::stdout_color_mt("console");
		//spdlog::register_logger(console);
	#endif

	ArgParser* cmdArgs = new ArgParser();
	cmdArgs->AddArgument("c","configfile",ArgType::required_argument,"config filename");
	cmdArgs->AddArgument("o","outputfile",ArgType::required_argument,"output filename");
	cmdArgs->AddArgument("i","inputfile",ArgType::required_argument,"input filename");
	cmdArgs->AddArgument("e","evtbuild",ArgType::no_argument,"build events");
	cmdArgs->AddArgument("h","help",ArgType::no_argument,"show this message");

	if( argc <= 2 ){
		cmdArgs->ShowUsage(argv[0]);
		return 0;
	}

	try{
		cmdArgs->ParseArgs(argc,argv);
	}catch( std::string& e ){
		#ifdef USE_SPDLOG 
			console->error(e);
		#else
			std::cout << e << std::endl;
		#endif
		return 1;
	}

	genscan* scan = new genscan();

	if( cmdArgs->GetArgumentIsEnabledLongName("help") ){
		cmdArgs->ShowUsage(argv[0]);
		return 0;
	}
	std::string* configfile = cmdArgs->GetArgumentValueLongName("configfile");
	std::string* outputfile = cmdArgs->GetArgumentValueLongName("outputfile");
	std::string* inputfile = cmdArgs->GetArgumentValueLongName("inputfile");
	bool evtbuild = cmdArgs->GetArgumentIsEnabledLongName("evtbuild");

	scan->SetCfgFile(configfile);
	scan->SetOutputFile(outputfile);
	scan->SetInputFile(inputfile);
	scan->SetEvtBuild(evtbuild);

	scan->DoScan();

	return 0;
}
