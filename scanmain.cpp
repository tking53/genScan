#include <cstdlib>
#include <string>
#include <memory>
#include <iostream>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "StringManipFunctions.hpp"

#include "GenScanorArgParser.hpp"

#include "ConfigParser.hpp"
#include "XMLConfigParser.hpp"
#include "YAMLConfigParser.hpp"
#include "JSONConfigParser.hpp"
#include "ChannelMap.hpp"

#include "HistogramManager.hpp"

#include "ProcessorList.hpp"

#include "DataParser.hpp"

int main(int argc, char *argv[]) {
	const int MAX_CRATES = 5;
	const int MAX_CARDS_PER_CRATE = 13;
	const int MAX_BOARDS = MAX_CARDS_PER_CRATE*MAX_CRATES;
	const int MAX_CHANNELS_PER_BOARD = 32;
	const int MAX_CHANNELS = MAX_CHANNELS_PER_BOARD*MAX_BOARDS;
	const int MAX_CAL_PARAMS_PER_CHANNEL = 4;

	std::string logname = "genscan";
	std::string logfilename = logname+".log";
	std::string errfilename = logname+".err";
	std::string dbgfilename = logname+".dbg";

	spdlog::set_level(spdlog::level::debug);
	std::shared_ptr<spdlog::sinks::basic_file_sink_mt> LogFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfilename,true);
	LogFileSink->set_level(spdlog::level::info);

	std::shared_ptr<spdlog::sinks::basic_file_sink_mt> ErrorFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(errfilename,true);
	ErrorFileSink->set_level(spdlog::level::err);

	std::shared_ptr<spdlog::sinks::basic_file_sink_mt> DebugFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(dbgfilename,true);
	DebugFileSink->set_level(spdlog::level::debug);

	std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> LogFileConsole = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	LogFileConsole->set_level(spdlog::level::info);

	std::vector<spdlog::sink_ptr> sinks {DebugFileSink,LogFileSink,ErrorFileSink,LogFileConsole};
	auto console = std::make_shared<spdlog::logger>(logname,sinks.begin(),sinks.end());
	spdlog::initialize_logger(console);

	auto cmdArgs = GenScanorArgParser::Get(argv[0]);
	try{
		cmdArgs->ParseArgs(argc,argv);
	}catch( std::runtime_error const& e ){
		console->error(e.what());
		exit(EXIT_FAILURE);
	}

	auto configfile = cmdArgs->GetConfigFile();
	auto outputfile = cmdArgs->GetOutputFile();
	auto limit = *(cmdArgs->GetLimit());
	auto FileNames = cmdArgs->GetInputFiles();
	auto evtbuild = *(cmdArgs->GetEvtBuild());
	auto dataformat = cmdArgs->GetDataFileType();

	const int lower_limit = 10000;

	if( limit < lower_limit ){
		console->warn("limit of {} is less than lower_limit of {}. Using lower_limit instead",limit,lower_limit);
		limit = lower_limit;
	}

	auto parser = DataParser::Get();
	try{ 
		if( dataformat->compare("evt") == 0 ){
			parser = DataParser::Get(DataParser::DataFileType::EVT);
		}else if( dataformat->compare("ldf") == 0 ){
			parser = DataParser::Get(DataParser::DataFileType::LDF);
		}else if( dataformat->compare("pld") == 0 ){
			parser = DataParser::Get(DataParser::DataFileType::PLD);
		}else if( dataformat->compare("caen_root") == 0 ){
			parser = DataParser::Get(DataParser::DataFileType::CAEN_ROOT);
		}else if( dataformat->compare("caen_bin") == 0 ){
			parser = DataParser::Get(DataParser::DataFileType::CAEN_BIN);
		}else{
			throw std::runtime_error("Unknown file format, supported types are evt,ldf,pld,caen_root,caen_bin");
		}
	}catch(std::runtime_error const& e){
		console->error(e.what());
		exit(EXIT_FAILURE);
	}

	console->info("Allocating memory for the ChannelMap");
	try{
		auto cmap = ChannelMap::Get(MAX_CRATES,MAX_CARDS_PER_CRATE,MAX_CHANNELS_PER_BOARD,MAX_CAL_PARAMS_PER_CHANNEL);
	}catch(std::runtime_error const& e ){
		console->error(e.what());
		exit(EXIT_FAILURE);
	}

	console->info("Begin parsing Config File");
	std::unique_ptr<ConfigParser> cfgparser;
	auto config_extension = StringManip::GetFileExtension(*configfile);
	if( config_extension == "xml" ){
		cfgparser.reset(new XMLConfigParser(logname));
	}else if( config_extension == "yaml" or config_extension == "yml" ){
		cfgparser.reset(new YAMLConfigParser(logname));
	}else if( config_extension == "json" ){
		cfgparser.reset(new JSONConfigParser(logname));
	}else{
		console->error("unknown file extension of {}, supported extensions are xml, yaml, json",config_extension);
		exit(EXIT_FAILURE);
	}
	cfgparser->SetConfigFile(configfile);
	try{
		cfgparser->Parse();
	}catch(std::runtime_error const& e){
		console->error(e.what());
		exit(EXIT_FAILURE);
	}

	std::shared_ptr<PLOTS::PlotRegistry> HistogramManager(new PLOTS::PlotRegistry(StringManip::GetFileBaseName(*outputfile)));
	HistogramManager->Initialize(MAX_CHANNELS,PLOTS::SE,PLOTS::SE);

	//auto processorlist = ProcessorList::Get();
	//try{
	//	processorlist->InitializeProcessors(cfgparser->GetProcessors());
	//	processorlist->InitializeAnalyzers(cfgparser->GetAnalyzers());
	//}catch(std::runtime_error const& e){
	//	console->error(e.what());
	//	exit(EXIT_FAILURE);
	//}
	
	//Init the processors/analyzers
	
	HistogramManager->WriteInfo();

	sleep(30);
	//try{
	// 	while(Parse(inputfile_list)){
	// 		Correlate();
	//
	// 		try{
	// 			processorlist->PreAnalyze();
	// 			processorlist->PreProcess();
	//
	// 			processorlist->Analyze();
	// 			processorlist->Process();
	//
	// 			processorlist->PostAnalyze();
	// 			processorlist->PostProcess();
	// 		}catch(std::runtime_error const& e){
	// 			console->error(e.what());
	// 		}
	// 	}
	//}catch(std::runtime_error const& e){
	// 	console->error(e.what());
	//}
	//
	//void ProcessorList::(Pre//Post)(Analyze/Process)(){
	// 	for( auto& (a/p) : (Analyzers/Processors) )
	// 		(a/p)->(Pre//Post)(Analyze/Process)();
	//}

	//parse portion of the data, 
	// 	either set a limit of how much memory is being used, 
	// 	read in certain number of entries, or 
	// 	read until outside correlation window
	//
	//Correlate events
	//
	//Run correlated events through Analyzers
	//Run correlated events through Processors 
	//
	//Write correlated events to disk

	return 0;
}
