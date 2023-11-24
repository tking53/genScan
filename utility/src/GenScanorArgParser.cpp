#include <exception>
#include <pthread.h>
#include <regex>
#include <sstream>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "ArgParser.hpp"
#include "GenScanorArgParser.hpp"

GenScanorArgParser* GenScanorArgParser::instance = nullptr;

GenScanorArgParser* GenScanorArgParser::Get(char* name){
	if( instance == nullptr ){
		instance = new GenScanorArgParser(name);
	}
	return instance;
}

GenScanorArgParser* GenScanorArgParser::Get(){
	if( instance == nullptr ){
		throw std::runtime_error("GenScanorArgParser is not initialized");
	}
	return instance;
}

GenScanorArgParser::GenScanorArgParser(char* name) : ArgParser(name){
	ConfigFile = std::make_shared<ArgValue<std::string>>("c","configfile","[filename] filename for channel map","default.txt");	
	OutputFile = std::make_shared<ArgValue<std::string>>("o","outputfile","[filename] filename for output","default.txt");	
	EvtBuild = std::make_shared<ArgValue<bool>>("e","evtbuild","event build only",false);
	FileNames = std::make_shared<ArgValue<std::vector<std::string>>>("f","filenames","[file1 file2 file3 ...] list of files used for input",std::vector<std::string>());
	Limit = std::make_shared<ArgValue<int>>("l","limit","limit of coincidence queue",100000);
	DataFileType = std::make_shared<ArgValue<std::string>>("x","fileformat","[file_format] format of the data file (evt,ldf,pld,caen_root,caen_bin)","unknown");
}

void GenScanorArgParser::ShowUsage(){
	spdlog::get("genscan")->info("{}",ProgName);
	spdlog::get("genscan")->info("{}/{} {}",ConfigFile->GetShortOptName(),ConfigFile->GetLongOptName(),ConfigFile->GetDescription());
	spdlog::get("genscan")->info("{}/{} {}",OutputFile->GetShortOptName(),OutputFile->GetLongOptName(),OutputFile->GetDescription());
	spdlog::get("genscan")->info("{}/{} {}",EvtBuild->GetShortOptName(),EvtBuild->GetLongOptName(),EvtBuild->GetDescription());
	spdlog::get("genscan")->info("{}/{} {}",FileNames->GetShortOptName(),FileNames->GetLongOptName(),FileNames->GetDescription());
	spdlog::get("genscan")->info("{}/{} {}",Help->GetShortOptName(),Help->GetLongOptName(),Help->GetDescription());
	spdlog::get("genscan")->info("{}/{} {}",Limit->GetShortOptName(),Limit->GetLongOptName(),Limit->GetDescription());
	spdlog::get("genscan")->info("{}/{} {}",DataFileType->GetShortOptName(),DataFileType->GetLongOptName(),DataFileType->GetDescription());
}

void GenScanorArgParser::ParseArgs(int argc,char* argv[]){
	if( argc < 2){
		ShowUsage();
		exit(EXIT_SUCCESS);
	}

	auto options = ParseOptions(argc,argv);

	for( size_t ii = 0; ii < options.size(); ++ii ){
		auto opt = options.at(ii).str(0);
		auto start = options.at(ii).position(0);
		auto end = fullargv.size();
		if( ii < options.size() - 1 )
			end = options.at(ii+1).position(0);
		auto optarg = fullargv.substr(start+opt.size(),(end-(start+opt.size()+1)));
		if( optarg.size() > 0 )
			optarg = optarg.substr(1,optarg.size()-1);
		if( Help->MatchesShortName(opt) or Help->MatchesLongName(opt) ){
			if( optarg.size() > 0 ){
				std::string msg = "Option : "+opt+" doesn't take an argument but, "+optarg+" was passed";
				throw std::runtime_error(msg);
			}else{
				Help->UpdateValue(true);
			}
		}else if( EvtBuild->MatchesShortName(opt) or EvtBuild->MatchesLongName(opt) ){
			if( optarg.size() > 0 ){
				std::string msg = "Option : "+opt+" doesn't take an argument but, "+optarg+" was passed";
				throw std::runtime_error(msg);
			}else{
				EvtBuild->UpdateValue(true);
			}
		}else if( Limit->MatchesShortName(opt) or Limit->MatchesLongName(opt) ){
			if( optarg.size() > 0 ){
				std::string msg = "Option : "+opt+" doesn't take an argument but, "+optarg+" was passed";
				throw std::runtime_error(msg);
			}else{
				auto val = std::stoi(optarg);
				Limit->UpdateValue(val);
			}
		}else if( OutputFile->MatchesShortName(opt) or OutputFile->MatchesLongName(opt) ){
			if( optarg.size() == 0 ){
				std::string msg = "Option : "+opt+" requires an argument but none were passed";
				throw std::runtime_error(msg);
			}else{
				OutputFile->UpdateValue(optarg);
			}
		}else if( DataFileType->MatchesShortName(opt) or DataFileType->MatchesLongName(opt) ){
			if( optarg.size() == 0 ){
				std::string msg = "Option : "+opt+" requires an argument but none were passed";
				throw std::runtime_error(msg);
			}else{
				DataFileType->UpdateValue(optarg);
			}
		}else if( ConfigFile->MatchesShortName(opt) or ConfigFile->MatchesLongName(opt) ){
			if( optarg.size() == 0 ){
				std::string msg = "Option : "+opt+" requires an argument but none were passed";
				throw std::runtime_error(msg);
			}else{
				ConfigFile->UpdateValue(optarg);
			}
		}else if( FileNames->MatchesShortName(opt) or FileNames->MatchesLongName(opt) ){
			if( optarg.size() == 0 ){
				std::string msg = "Option : "+opt+" requires an argument but none were passed";
				throw std::runtime_error(msg);
			}else{
				std::vector<std::string> names;
				std::string curr;
				std::stringstream ss(optarg);
				while( ss >> curr ){
					names.push_back(curr);
				}
				FileNames->UpdateValue(names);
			}
		}else{
			std::string msg = "Bad Option : "+opt;
			throw std::runtime_error(msg);
		}
	}

	if( *(Help->GetValue()) ){
		ShowUsage();
		exit(EXIT_SUCCESS);
	}

	if( FileNames->IsDefault() ){
		std::string msg = "Need FileNames specified";
		throw std::runtime_error(msg);
	}

	if( OutputFile->IsDefault() ){
		std::string msg = "Need OutputFile specified";
		throw std::runtime_error(msg);
	}

	if( ConfigFile->IsDefault() ){
		std::string msg = "Need ConfigFile specified";
		throw std::runtime_error(msg);
	}

	if( DataFileType->IsDefault() ){
		std::string msg = "Need DataFileType specified";
		throw std::runtime_error(msg);
	}
}

std::string* GenScanorArgParser::GetConfigFile() const{
	return ConfigFile->GetValue();
}

std::string* GenScanorArgParser::GetOutputFile() const{
	return OutputFile->GetValue();
}

std::vector<std::string>* GenScanorArgParser::GetInputFiles() const{
	return FileNames->GetValue();
}

std::string* GenScanorArgParser::GetDataFileType() const{
	return DataFileType->GetValue();
}

bool* GenScanorArgParser::GetEvtBuild() const{
	return EvtBuild->GetValue();
}

int* GenScanorArgParser::GetLimit() const{
	return Limit->GetValue();
}
