#include <chrono>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <memory>
#include <iostream>
#include <thread>
#include <csignal>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>
#include <boost/container/devector.hpp>

#include "AnalogCorrelator.hpp"
#include "RollingTriggerCorrelator.hpp"
#include "RollingWindowCorrelator.hpp"
#include "StatsTracker.hpp"
#include "StringManipFunctions.hpp"

#include "ConfigParser.hpp"
#include "Translator.hpp"
#include "ChannelMap.hpp"

#include "Correlator.hpp"

#include "HistogramManager.hpp"

#include "CutManager.hpp"

#include "RootFileManager.hpp"

#include "ProcessorList.hpp"

#include "DataParser.hpp"

#include "EventHistoryManager.hpp"

volatile bool ctrlCPressed = false;

void signalHandler(int signum) {
	if (signum == SIGINT) {
		spdlog::critical("Ctrl-C pressed, Cleaning up and finalizing files");
		ctrlCPressed = true;
	}
}

int main(int argc, char *argv[]) {
	std::chrono::time_point<std::chrono::high_resolution_clock> global_start_time = std::chrono::high_resolution_clock::now();
	const std::string logname = "genscan";

	std::string configfile;
	std::string outputfile;
	bool enabletree;
	std::vector<std::string> FileNames;
	int port;
	int limit;
	std::string dataformat;
	
	int MAX_CRATES = 2;
	int MAX_CARDS_PER_CRATE = 13;
	int MAX_CHANNELS_PER_BOARD = 16;
	int MAX_CAL_PARAMS_PER_CHANNEL = 4;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("configfile,c",boost::program_options::value<std::string>(&configfile)->default_value("config.xml"),"[filename] filename for channel map")
		("outputfile,o",boost::program_options::value<std::string>(&outputfile)->default_value("out"),"[filename] filename for output")
		("enabletree,t",boost::program_options::value<bool>(&enabletree)->default_value(true),"enable root tree output or disable it and only generate histograms")
		("file,f",boost::program_options::value<std::vector<std::string>>(&FileNames),"[file1 file2 file3 ...] list of files used for input")
		("limit,l",boost::program_options::value<int>(&limit)->default_value(10),"number of events to keep in history [0 -> current, 1 -> prev., ... N-1]")
		("format,x",boost::program_options::value<std::string>(&dataformat)->default_value("null"),"[file_format] format of the data file (evt,evt-presort,ldf,pacman_ldf,pld,caen_root,caen_bin)")
		("port,p",boost::program_options::value<int>(&port)->default_value(9090),"[portid] port to listen/send on for the live histogramming, -1 disables for batch scanning")
		("max_crates,i",boost::program_options::value<int>(&MAX_CRATES)->default_value(1),"[MAX_CRATES] Number of crates to expect in data stream")
		("max_slots,j",boost::program_options::value<int>(&MAX_CARDS_PER_CRATE)->default_value(13),"[MAX_CARDS_PER_CRATE] Number of cards per crate to expect in data stream")
		("max_channels,k",boost::program_options::value<int>(&MAX_CHANNELS_PER_BOARD)->default_value(16),"[MAX_CHANNELS_PER_BOARD] Number of channels per board to expect in data stream")
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

	const int upper_limit = 1000;
	
	if( FileNames.size() == 0 ){
		spdlog::error("No input files provided");
		exit(EXIT_FAILURE);
	}	

	if( MAX_CRATES < 1 ){
		spdlog::error("Can't have fewer than 1 crate, supplied with {} max_crates option",MAX_CRATES);
		exit(EXIT_FAILURE);
	}

	if( MAX_CARDS_PER_CRATE < 1 or MAX_CARDS_PER_CRATE > 13 ){
		spdlog::error("Can't have fewer than 1 card per crate or more than 13 cards per crate, supplied with {} max_slots option",MAX_CARDS_PER_CRATE);
		exit(EXIT_FAILURE);
	}

	if( MAX_CHANNELS_PER_BOARD != 16 and MAX_CHANNELS_PER_BOARD != 32 and MAX_CHANNELS_PER_BOARD != 8 and MAX_CHANNELS_PER_BOARD != 4 and MAX_CHANNELS_PER_BOARD != 64 ){
		spdlog::error("Can only have either 4, 8, 16, 32, or 64 channels per board, supplied with {} max_channels option",MAX_CHANNELS_PER_BOARD);
		exit(EXIT_FAILURE);
	}
	
	int MAX_BOARDS = MAX_CARDS_PER_CRATE*MAX_CRATES;
	int MAX_CHANNELS = MAX_CHANNELS_PER_BOARD*MAX_BOARDS;

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

	if( limit > upper_limit ){
		console->warn("limit of {} is greater than upper_limit of {}. Using upper_limit instead",limit,upper_limit);
		limit = upper_limit;
	}

	std::unique_ptr<DataParser> dataparser;
	try{ 
		if( dataformat.compare("evt") == 0 ){
			dataparser.reset(new DataParser(DataParser::DataFileType::EVT_BUILT,logname));
		}else if( dataformat.compare("evt-presort") == 0 ){
			dataparser.reset(new DataParser(DataParser::DataFileType::EVT_PRESORT,logname));
		}else if( dataformat.compare("ldf") == 0 ){
			dataparser.reset(new DataParser(DataParser::DataFileType::LDF_PIXIE,logname));
		}else if( dataformat.compare("pacman_ldf") == 0 ){
			dataparser.reset(new DataParser(DataParser::DataFileType::PACMAN_LDF_PIXIE,logname));
		}else if( dataformat.compare("pld") == 0 ){
			dataparser.reset(new DataParser(DataParser::DataFileType::PLD,logname));
		}else if( dataformat.compare("caen_root") == 0 ){
			dataparser.reset(new DataParser(DataParser::DataFileType::CAEN_ROOT,logname));
		}else if( dataformat.compare("caen_bin") == 0 ){
			dataparser.reset(new DataParser(DataParser::DataFileType::CAEN_BIN,logname));
		}else{
			throw std::runtime_error("Unknown file format : "+dataformat+", supported types are evt,evt-presort,ldf,pld,caen_root,caen_bin");
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
		cfgparser.reset(new ConfigParser(logname));
	}else{
		console->error("unknown file extension of {}, supported extensions are xml",config_extension);
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

	std::shared_ptr<Correlator> correlator;
	try{
		if( cfgparser->GetCorrelationType()->compare("rolling-window") == 0 ){
			correlator = std::make_shared<RollingWindowCorrelator>(logname,cfgparser->GetGlobalEventWidthInNS());
		}else if( cfgparser->GetCorrelationType()->compare("rolling-trigger") == 0 ){
			correlator = std::make_shared<RollingTriggerCorrelator>(logname,cfgparser->GetGlobalEventWidthInNS());
		}else if( cfgparser->GetCorrelationType()->compare("analog") == 0 ){
			correlator = std::make_shared<AnalogCorrelator>(logname,cfgparser->GetGlobalEventWidthInNS());
		}else{
			throw std::runtime_error("Unknown Correlation Type : "+(*(cfgparser->GetCorrelationType()))+", supported types are rolling-window, rolling,trigger, analog");
		}
	}catch(std::runtime_error const& e){
		console->error(e.what());
		exit(EXIT_FAILURE);
	}
	dataparser->SetCorrelator(correlator);
	console->info("event width : {:.0f} ns Correlation type : {}",cfgparser->GetGlobalEventWidthInNS(),(*(cfgparser->GetCorrelationType())));

	console->info("Generating CutRegistry if any cuts listed within the config file");
	std::shared_ptr<CUTS::CutRegistry> CutManager(new CUTS::CutRegistry(logname));
	try{
		for( const auto& details : cfgparser->GetCutDetails() ){
			CutManager->AddCut(details.first,details.second);
		}
	}catch(std::runtime_error const& e){
		console->error(e.what());
		exit(EXIT_FAILURE);
	}

	console->info("Generating Plot Registry");
	std::shared_ptr<PLOTS::PlotRegistry> HistogramManager(new PLOTS::PlotRegistry(logname,StringManip::StripFileExtension(outputfile),port));
	auto ebins = PLOTS::SG;
	auto sbins = PLOTS::SE;
	auto wbins = PLOTS::SE;
	auto zbins = PLOTS::SA;
	auto rbins = PLOTS::S4;
	HistogramManager->Initialize(MAX_CHANNELS,ebins,sbins,wbins,zbins,rbins);
	console->info("Generated Raw, Scalar, and Cal plots for {} Channels, There are {} bins for Raw and Cal, and {} bins for Scalar",MAX_CHANNELS,ebins,sbins);
	
	std::shared_ptr<RootFileManager> RootManager(new RootFileManager(logname,StringManip::StripFileExtension(outputfile),enabletree));
	console->info("Created Root File Manager");
	
	//Init the processors/analyzers
	std::shared_ptr<ProcessorList> processorlist = std::make_shared<ProcessorList>(logname);
	try{
		if( config_extension == "xml" ){
			processorlist->InitializeProcessors(cfgparser.get());
			processorlist->InitializeAnalyzers(cfgparser.get());
		}else{
			console->error("unknown file extension of {}, supported extensions are xml",config_extension);
			exit(EXIT_FAILURE);
		}
		processorlist->DeclarePlots(HistogramManager.get());
		processorlist->RegisterCuts(CutManager.get());
		processorlist->RegisterOutputTrees(RootManager.get());
		processorlist->Finalize();
	}catch(std::runtime_error const& e){
		console->error(e.what());
		exit(EXIT_FAILURE);
	}
	
	console->info("Generating {}.list file that contains all the declared histograms",StringManip::GetFileBaseName(outputfile));
	HistogramManager->WriteInfo();

	console->info("Generating Statistics Manager");
	std::shared_ptr<StatsTracker> StatsManager(new StatsTracker(logname));
	StatsManager->Init(cmap.get());
	console->info("Created Statistics Manager");

	std::thread plotter(&PLOTS::PlotRegistry::HandleSocketHelper,HistogramManager.get());
	if( port <= 0 ){ 
		HistogramManager->KillListen();
		plotter.join();
	}

	std::signal(SIGINT, signalHandler);

	std::shared_ptr<EventHistoryManager> EvtManager( new EventHistoryManager(logname,limit));
	EvtManager->InitMappedUIDs(cmap.get(),processorlist.get());
	Translator::TRANSLATORSTATE CurrState = Translator::TRANSLATORSTATE::UNKNOWN;
	try{
		do{
			EvtManager->RotateBuffer();
			CurrState = dataparser->Parse(EvtManager->GetCurrentEventSummary()->GetRawEvents());

			if( not EvtManager->IsCurrentEventSummaryEmpty() ) [[likely]] {
				processorlist->ThreshAndCal(EvtManager->GetCurrentEventSummary()->GetRawEvents(),cmap.get());
				processorlist->ProcessRaw(EvtManager->GetCurrentEventSummary()->GetRawEvents(),HistogramManager.get());
				StatsManager->IncrementStats(EvtManager->GetCurrentEventSummary()->GetRawEvents());

				EvtManager->BuildCurrentEventDetectorSummary();

				processorlist->PreAnalyze(EvtManager.get(),HistogramManager.get(),CutManager.get());
				processorlist->PreProcess(EvtManager.get(),HistogramManager.get(),CutManager.get());

				processorlist->Analyze(EvtManager.get(),HistogramManager.get(),CutManager.get());
				processorlist->Process(EvtManager.get(),HistogramManager.get(),CutManager.get());

				processorlist->PostAnalyze(EvtManager.get(),HistogramManager.get(),CutManager.get());
				processorlist->PostProcess(EvtManager.get(),HistogramManager.get(),CutManager.get());

				if( enabletree ){
					RootManager->Fill();
				}
				processorlist->CleanupTrees();
			}else [[unlikely]] {
				if( CurrState != Translator::TRANSLATORSTATE::COMPLETE ){
					console->critical("CurrState : {} RawEvents : {}",CurrState,EvtManager->GetCurrentEventSummary()->GetRawEvents().size());
					throw std::runtime_error("Read data but nothing decoded to allow for correlation");
				}
			}
		}while( CurrState == Translator::TRANSLATORSTATE::PARSING and not ctrlCPressed );
	}catch(std::runtime_error const& e){
		console->error(e.what());
	}
	
	if( port > 0 ){ 
		HistogramManager->KillListen();
		plotter.join();
	}

	//Write correlated events to disk
	HistogramManager->WriteAllPlots();
	RootManager->WriteTNamed("MAX_CRATES",std::to_string(MAX_CRATES));
	RootManager->WriteTNamed("MAX_CARDS_PER_CRATE",std::to_string(MAX_CARDS_PER_CRATE));
	RootManager->WriteTNamed("MAX_CHANNELS_PER_BOARD",std::to_string(MAX_CHANNELS_PER_BOARD));
	RootManager->WriteTNamed("ConfigFile",configfile);
	RootManager->FinalizeTrees();
	std::chrono::time_point<std::chrono::high_resolution_clock> global_stop_time = std::chrono::high_resolution_clock::now();
	auto global_run_time = global_stop_time - global_start_time;
	const auto hrs = std::chrono::duration_cast<std::chrono::hours>(global_run_time);
	const auto mins = std::chrono::duration_cast<std::chrono::minutes>(global_run_time - hrs);
	const auto secs = std::chrono::duration_cast<std::chrono::seconds>(global_run_time - hrs - mins);
	const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(global_run_time - hrs - mins - secs);
	console->info("Finished running in {} hours {} minutes {} seconds {} milliseconds",hrs.count(),mins.count(),secs.count(),ms.count());
	console->critical("All data has been written to {}.root",outputfile);

	return 0;
}
