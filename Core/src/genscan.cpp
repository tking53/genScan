#include <iostream>

#include "genscan.hpp"

#include "CompassROOTUnpacker.hpp"

genscan::genscan(){
	#ifdef USE_SPDLOG
		LogFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("genscan.log",true);
		LogFileConsole = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		std::vector<spdlog::sink_ptr> sinks {LogFileSink,LogFileConsole};
		auto LogFile = std::make_shared<spdlog::logger>("genscan",sinks.begin(),sinks.end());
		spdlog::register_logger(LogFile);
	#endif
}

void genscan::SetCfgFile(std::string* cfg){
	cfgFile = cfg;
}

void genscan::SetInputFile(std::string* infile){
	inputFile = infile;
}

void genscan::SetOutputFile(std::string* outfile){
	outputFile = outfile;
}

void genscan::SetEvtBuild(bool evtbuild){
	evtBuild = evtbuild;
}

void genscan::DoScan(){
	//need to determine which unpacker to use
	//either do this based on file extension of by command line option?
	//default to a specific file extension?
	//probably safest to just read the file extension
	
	unpacker = new CompassROOTUnpacker();
	unpacker->SetCurrFile(inputFile);
	#ifdef USE_SPDLOG
		spdlog::get("genscan")->info("Current File : {}",*(unpacker->GetCurrFile()));
		//LogFile->info("Current File : {}",*(unpacker->GetCurrFile()));
	#else
		std::cout << "Current File : " << *(unpacker->GetCurrFile()) << std::endl;
	#endif
	
	try{
		configParser = configXMLparser::get(*cfgFile);
	}catch(std::invalid_argument const& e){
		spdlog::get("genscan")->error(e.what());
		exit(EXIT_FAILURE);
	}
}
