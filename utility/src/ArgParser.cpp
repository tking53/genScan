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

ArgParser* ArgParser::instance = nullptr;

ArgParser* ArgParser::Get(char* name){
	if( instance == nullptr ){
		instance = new ArgParser(name);
	}
	return instance;
}

ArgParser* ArgParser::Get(){
	if( instance == nullptr ){
		throw std::runtime_error("ArgParser is not initialized");
	}
	return instance;
}

ArgParser::ArgParser(char* name){
	ProgName = std::string(name);

	ConfigFile = std::make_shared<ArgValue<std::string>>("c","configfile","[filename] filename for channel map","default.txt");	
	OutputFile = std::make_shared<ArgValue<std::string>>("o","outputfile","[filename] filename for output","default.txt");	
	EvtBuild = std::make_shared<ArgValue<bool>>("e","evtbuild","event build only",false);
	FileNames = std::make_shared<ArgValue<std::vector<std::string>>>("f","filenames","[file1 file2 file3 ...] list of files used for input",std::vector<std::string>());
	Help = std::make_shared<ArgValue<bool>>("h","help","show this message",false);


}

void ArgParser::ShowUsage(){
	spdlog::get("genscan")->info("{}",ProgName);
	spdlog::get("genscan")->info("{}/{} {}",ConfigFile->GetShortOptName(),ConfigFile->GetLongOptName(),ConfigFile->GetDescription());
	spdlog::get("genscan")->info("{}/{} {}",OutputFile->GetShortOptName(),OutputFile->GetLongOptName(),OutputFile->GetDescription());
	spdlog::get("genscan")->info("{}/{} {}",EvtBuild->GetShortOptName(),EvtBuild->GetLongOptName(),EvtBuild->GetDescription());
	spdlog::get("genscan")->info("{}/{} {}",FileNames->GetShortOptName(),FileNames->GetLongOptName(),FileNames->GetDescription());
	spdlog::get("genscan")->info("{}/{} {}",Help->GetShortOptName(),Help->GetLongOptName(),Help->GetDescription());
}

void ArgParser::ParseArgs(int argc,char* argv[]){
	if( argc < 2){
		ShowUsage();
		exit(EXIT_SUCCESS);
	}

	std::string fullargv;
	for( int ii = 1; ii < argc; ++ii ){
		fullargv += std::string(argv[ii]) + " ";
	}

	std::regex option_regex("(-{1,2}\\w+)");
	std::vector<std::smatch> options;
	for( std::sregex_iterator it =  std::sregex_iterator(fullargv.begin(), fullargv.end(), option_regex); it !=  std::sregex_iterator(); it++) {
		std::smatch match;
		match = *it;
		options.push_back(match);
	}

	for( size_t ii = 0; ii < options.size(); ++ii ){
		auto opt = options.at(ii).str(0);
		auto start = options.at(ii).position(0);
		auto end = fullargv.size();
		if( ii < options.size() - 1 )
			end = options.at(ii+1).position(0);
		auto optarg = fullargv.substr(start+opt.size(),(end-(start+opt.size())));
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
}

std::string ArgParser::GetConfigFile() const{
	return *(ConfigFile->GetValue());
}

std::string ArgParser::GetOutputFile() const{
	return *(OutputFile->GetValue());
}

std::vector<std::string> ArgParser::GetInputFiles() const{
	return *(FileNames->GetValue());
}

bool ArgParser::GetEvtBuild() const{
	return *(EvtBuild->GetValue());
}
