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
#include <vector>

#include "ArgParser.hpp"

ArgParser::ArgParser(char* name,const std::string& log){
	ProgName = std::string(name);
	LogName = log; 
	Help = std::make_shared<ArgValue<bool>>("h","help","show this message",false);
}

void ArgParser::ShowUsage(){
	spdlog::get("genscan")->info("{}",ProgName);
	spdlog::get("genscan")->info("{}/{} {}",Help->GetShortOptName(),Help->GetLongOptName(),Help->GetDescription());
}

std::vector<std::smatch> ArgParser::ParseOptions(int argc,char* argv[]){
	fullargv = "";
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
	return options;
}

void ArgParser::ParseArgs(int argc,char* argv[]){
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
		}else{
			std::string msg = "Bad Option : "+opt;
			throw std::runtime_error(msg);
		}
	}

	if( *(Help->GetValue()) ){
		ShowUsage();
		exit(EXIT_SUCCESS);
	}
}
