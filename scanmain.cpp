#include <cstdlib>
#include <stdexcept>
#include <string>
#include <memory>
#include <iostream>
#include <thread>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>
#include <boost/container/devector.hpp>

#include "Processor.hpp"
#include "StringManipFunctions.hpp"

#include "ConfigParser.hpp"
#include "XMLConfigParser.hpp"
#include "YAMLConfigParser.hpp"
#include "JSONConfigParser.hpp"
#include "ChannelMap.hpp"

#include "Correlator.hpp"

#include "HistogramManager.hpp"

#include "RootFileManager.hpp"

#include "ProcessorList.hpp"

#include "DataParser.hpp"

#include "PhysicsData.hpp"

int main(int argc, char *argv[]) {
	const std::string logname = "genscan";

	std::string configfile;
	std::string outputfile;
	bool evtbuild;
	std::vector<std::string> FileNames;
	int port;
	int limit;
	std::string dataformat;
	
	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("configfile,c",boost::program_options::value<std::string>(&configfile)->default_value("config.xml"),"[filename] filename for channel map")
		("outputfile,o",boost::program_options::value<std::string>(&outputfile)->default_value("out.txt"),"[filename] filename for output")
		("evtbuild,e",boost::program_options::value<bool>(&evtbuild)->default_value(false),"event build only")
		("file,f",boost::program_options::value<std::vector<std::string>>(&FileNames),"[file1 file2 file3 ...] list of files used for input")
		("limit,l",boost::program_options::value<int>(&limit)->default_value(100000),"limit of coincidence queue")
		("format,x",boost::program_options::value<std::string>(&dataformat)->default_value("null"),"[file_format] format of the data file (evt,ldf,pld,caen_root,caen_bin)")
		("port,p",boost::program_options::value<int>(&port)->default_value(9090),"[portid] port to listen/send on for the live histogramming")
		;


	boost::program_options::positional_options_description p;
        p.add("file", -1);

	try{
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
        	notify(vm);
		if( vm.count("help") or argc <= 2 ){
			spdlog::info(cmdline_options);
			exit(EXIT_SUCCESS);
		}
	}catch( std::exception& e){
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}    

	const int lower_limit = 1000;
	
	if( limit < lower_limit ){
		spdlog::error("limit {} passed in is lower than lower_limit of {}",limit,lower_limit);
		exit(EXIT_FAILURE);
	}

	if( FileNames.size() == 0 ){
		spdlog::error("No input files provided");
		exit(EXIT_FAILURE);
	}	

	const int MAX_CRATES = 5;
	const int MAX_CARDS_PER_CRATE = 13;
	const int MAX_BOARDS = MAX_CARDS_PER_CRATE*MAX_CRATES;
	const int MAX_CHANNELS_PER_BOARD = 32;
	const int MAX_CHANNELS = MAX_CHANNELS_PER_BOARD*MAX_BOARDS;
	const int MAX_CAL_PARAMS_PER_CHANNEL = 4;

	const std::string logfilename = (outputfile)+".log";
	const std::string errfilename = (outputfile)+".err";
	const std::string dbgfilename = (outputfile)+".dbg";

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
	console->flush_on(spdlog::level::info);

	if( limit < lower_limit ){
		console->warn("limit of {} is less than lower_limit of {}. Using lower_limit instead",limit,lower_limit);
		limit = lower_limit;
	}

	std::unique_ptr<DataParser> dataparser;
	try{ 
		if( dataformat.compare("evt") == 0 ){
			dataparser.reset(new DataParser(DataParser::DataFileType::EVT_BUILT,logname));
		}else if( dataformat.compare("evt-presort") == 0 ){
			dataparser.reset(new DataParser(DataParser::DataFileType::EVT_PRESORT,logname));
		}else if( dataformat.compare("ldf") == 0 ){
			dataparser.reset(new DataParser(DataParser::DataFileType::LDF,logname));
		}else if( dataformat.compare("pld") == 0 ){
			dataparser.reset(new DataParser(DataParser::DataFileType::PLD,logname));
		}else if( dataformat.compare("caen_root") == 0 ){
			dataparser.reset(new DataParser(DataParser::DataFileType::CAEN_ROOT,logname));
		}else if( dataformat.compare("caen_bin") == 0 ){
			dataparser.reset(new DataParser(DataParser::DataFileType::CAEN_BIN,logname));
		}else{
			throw std::runtime_error("Unknown file format : "+dataformat+", supported types are evt,ldf,pld,caen_root,caen_bin");
		}
	}catch(std::runtime_error const& e){
		console->error(e.what());
		exit(EXIT_FAILURE);
	}
	try{
		dataparser->SetInputFiles(FileNames);
	}catch(std::runtime_error const& e){
		console->error(e.what());
		exit(EXIT_FAILURE);
	}

	console->info("Allocating memory for the ChannelMap");
	std::shared_ptr<ChannelMap> cmap;
	try{
		cmap = std::make_shared<ChannelMap>(MAX_CRATES,MAX_CARDS_PER_CRATE,MAX_CHANNELS_PER_BOARD,MAX_CAL_PARAMS_PER_CHANNEL);
	}catch(std::runtime_error const& e ){
		console->error(e.what());
		exit(EXIT_FAILURE);
	}

	console->info("Begin parsing Config File : {}",configfile);
	std::unique_ptr<ConfigParser> cfgparser;
	auto config_extension = StringManip::GetFileExtension(configfile);
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
		cfgparser->Parse(cmap.get());
	}catch(std::runtime_error const& e){
		console->error(e.what());
		exit(EXIT_FAILURE);
	}
	console->info("Completed parsing Config File : {}",configfile);

	try{
		cmap->FinalizeChannelMap();
	}catch(std::runtime_error const& e ){
		console->error(e.what());
		exit(EXIT_FAILURE);
	}

	dataparser->SetChannelMap(cmap);

	spdlog::info("event width : {} ns",cfgparser->GetGlobalEventWidthInNS());
	//std::shared_ptr<Correlator> correlator = std::make_shared<Correlator>(cfgparser->GetGlobalEventWidth());
	//try{
	//}catch(std::runtime_error const& e){
	//}

	console->info("Generating Plot Registry");
	std::shared_ptr<PLOTS::PlotRegistry> HistogramManager(new PLOTS::PlotRegistry(logname,StringManip::GetFileBaseName(outputfile),port));
	auto ebins = PLOTS::SE;
	auto sbins = PLOTS::SE;
	HistogramManager->Initialize(MAX_CHANNELS,ebins,sbins);
	console->info("Generated Raw, Scalar, and Cal plots for {} Channels, There are {} bins for Raw and Cal, and {} bins for Scalar",MAX_CHANNELS,ebins,sbins);
	
	std::shared_ptr<RootFileManager> RootManager(new RootFileManager(logname,StringManip::GetFileBaseName(outputfile)));
	console->info("Created Root File Manager");

	//Init the processors/analyzers
	std::shared_ptr<ProcessorList> processorlist = std::make_shared<ProcessorList>(logname);
	try{
		if( config_extension == "xml" ){
			processorlist->InitializeProcessors(reinterpret_cast<XMLConfigParser*>(cfgparser.get()));
			processorlist->InitializeAnalyzers(reinterpret_cast<XMLConfigParser*>(cfgparser.get()));
		}else if( config_extension == "yaml" or config_extension == "yml" ){
			processorlist->InitializeProcessors(reinterpret_cast<YAMLConfigParser*>(cfgparser.get()));
			processorlist->InitializeAnalyzers(reinterpret_cast<YAMLConfigParser*>(cfgparser.get()));
		}else if( config_extension == "json" ){
			processorlist->InitializeProcessors(reinterpret_cast<JSONConfigParser*>(cfgparser.get()));
			processorlist->InitializeAnalyzers(reinterpret_cast<JSONConfigParser*>(cfgparser.get()));
		}else{
			console->error("unknown file extension of {}, supported extensions are xml, yaml, json",config_extension);
			exit(EXIT_FAILURE);
		}
		processorlist->DeclarePlots(HistogramManager.get());
		processorlist->RegisterOutputTrees(RootManager.get());
	}catch(std::runtime_error const& e){
		console->error(e.what());
		exit(EXIT_FAILURE);
	}
	
	console->info("Generating {}.list file that contains all the declared histograms",logname);
	HistogramManager->WriteInfo();
	std::thread plotter(&PLOTS::PlotRegistry::HandleSocketHelper,HistogramManager.get());

	#ifdef DEBUG_MODE
		console->info("Beginning plotting");
		std::thread filler(&PLOTS::PlotRegistry::RandFill,HistogramManager.get());
		//std::this_thread::sleep_for(std::chrono::seconds(3));
		for( int ii = 0; ii < 1000000; ++ii ) [[likely]] {
			processorlist->PreProcess();
			processorlist->PreAnalyze();
	
			processorlist->Process();
			processorlist->Analyze();
	
			processorlist->PostProcess();
			processorlist->PostAnalyze();
		}
	#endif

	boost::container::devector<PhysicsData> RawEvents;
	dataparser->Parse(RawEvents);
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
	HistogramManager->KillListen();
	#ifdef DEBUG_MODE
		console->info("Ending plotting");
		filler.join();
	#endif
	plotter.join();

	HistogramManager->WriteAllPlots();
	RootManager->FinalizeTrees();

	return 0;
}
