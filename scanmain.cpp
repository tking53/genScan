#include <spdlog/common.h>
#include <string>
#include <iostream>

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "ArgParser.hpp"
#include "ConfigParser.hpp"
#include "ProcessorList.hpp"

int main(int argc, char *argv[]) {
	spdlog::set_level(spdlog::level::debug);
	std::shared_ptr<spdlog::sinks::basic_file_sink_mt> LogFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("genscan.log",true);
	LogFileSink->set_level(spdlog::level::info);

	std::shared_ptr<spdlog::sinks::basic_file_sink_mt> ErrorFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("genscan.err",true);
	ErrorFileSink->set_level(spdlog::level::err);

	std::shared_ptr<spdlog::sinks::basic_file_sink_mt> DebugFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("genscan.dbg",true);
	DebugFileSink->set_level(spdlog::level::debug);

	std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> LogFileConsole = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	LogFileConsole->set_level(spdlog::level::info);

	std::vector<spdlog::sink_ptr> sinks {DebugFileSink,LogFileSink,ErrorFileSink,LogFileConsole};
	auto console = std::make_shared<spdlog::logger>("genscan",sinks.begin(),sinks.end());
	//spdlog::register_logger(console);
	spdlog::initialize_logger(console);

	ArgParser* cmdArgs = new ArgParser();
	cmdArgs->AddArgument("c","configfile",ArgType::required_argument,"config filename");
	cmdArgs->AddArgument("o","outputfile",ArgType::required_argument,"output filename");
	cmdArgs->AddArgument("i","inputfile",ArgType::required_argument,"input filename");
	cmdArgs->AddArgument("e","evtbuild",ArgType::no_argument,"build events");
	cmdArgs->AddArgument("h","help",ArgType::no_argument,"show this message");

	if( argc <= 2 ){
		cmdArgs->ShowUsage(argv[0]);
		exit(EXIT_FAILURE);
	}

	try{
		cmdArgs->ParseArgs(argc,argv);
	}catch( std::runtime_error const& e ){
		console->error(e.what());
		exit(EXIT_FAILURE);
	}

	if( cmdArgs->GetArgumentIsEnabledLongName("help") ){
		cmdArgs->ShowUsage(argv[0]);
		exit(EXIT_SUCCESS);
	}

	std::string* configfile = cmdArgs->GetArgumentValueLongName("configfile");
	std::string* outputfile = cmdArgs->GetArgumentValueLongName("outputfile");
	std::string* inputfile = cmdArgs->GetArgumentValueLongName("inputfile");
	bool evtbuild = cmdArgs->GetArgumentIsEnabledLongName("evtbuild");

	console->info("Begin parsing Config File");
	auto cfgparser = ConfigParser::Get();
	cfgparser->SetConfigFile(configfile);
	try{
		cfgparser->Parse();
	}catch(std::runtime_error const& e){
		console->error(e.what());
	}

	auto processorlist = ProcessorList::Get();
	try{
		processorlist->InitializeProcessors(cfgparser->GetProcessors());
		processorlist->InitializeAnalyzers(cfgparser->GetAnalyzers());
	}catch(std::runtime_error const& e){
		console->error(e.what());
	}

	return 0;
}
