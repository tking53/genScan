#include <exception>
#include <pthread.h>
#include <regex>
#include <sstream>
#include <iostream>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "ArgParser.hpp"
#include "GenConfigArgParser.hpp"

GenConfigArgParser* GenConfigArgParser::instance = nullptr;

GenConfigArgParser* GenConfigArgParser::Get(char* name,const std::string& log){
	if( instance == nullptr ){
		instance = new GenConfigArgParser(name,log);
	}
	return instance;
}

GenConfigArgParser* GenConfigArgParser::Get(){
	if( instance == nullptr ){
		throw std::runtime_error("GenConfigArgParser is not initialized");
	}
	return instance;
}

GenConfigArgParser::GenConfigArgParser(char* name,const std::string& log) : ArgParser(name,log){
	ConfigFile = std::make_shared<ArgValue<std::string>>("c","configfile","[filename] filename for channel map","default.txt");	
	OutputFile = std::make_shared<ArgValue<std::string>>("o","outputfile","[filename] filename for output","default.txt");	
}

void GenConfigArgParser::ShowUsage(){
	spdlog::info("{}",ProgName);
	spdlog::info("{}/{} {}",ConfigFile->GetShortOptName(),ConfigFile->GetLongOptName(),ConfigFile->GetDescription());
	spdlog::info("{}/{} {}",OutputFile->GetShortOptName(),OutputFile->GetLongOptName(),OutputFile->GetDescription());
	spdlog::info("{}/{} {}",Help->GetShortOptName(),Help->GetLongOptName(),Help->GetDescription());
}

void GenConfigArgParser::ParseArgs(int argc,char* argv[]){
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
		}else if( OutputFile->MatchesShortName(opt) or OutputFile->MatchesLongName(opt) ){
			if( optarg.size() == 0 ){
				std::string msg = "Option : "+opt+" requires an argument but none were passed";
				throw std::runtime_error(msg);
			}else{
				OutputFile->UpdateValue(optarg);
			}
		}else if( ConfigFile->MatchesShortName(opt) or ConfigFile->MatchesLongName(opt) ){
			if( optarg.size() == 0 ){
				std::string msg = "Option : "+opt+" requires an argument but none were passed";
				throw std::runtime_error(msg);
			}else{
				ConfigFile->UpdateValue(optarg);
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

	if( OutputFile->IsDefault() ){
		std::string msg = "Need OutputFile specified";
		throw std::runtime_error(msg);
	}

	if( ConfigFile->IsDefault() ){
		std::string msg = "Need ConfigFile specified";
		throw std::runtime_error(msg);
	}
}

std::string* GenConfigArgParser::GetConfigFile() const{
	return ConfigFile->GetValue();
}

std::string* GenConfigArgParser::GetOutputFile() const{
	return OutputFile->GetValue();
}
