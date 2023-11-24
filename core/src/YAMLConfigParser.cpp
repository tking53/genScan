#include <stdexcept>
#include <sstream>
#include <limits>
#include <regex>

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "YAMLConfigParser.hpp"
#include "ChannelMap.hpp"
#include "yaml-cpp/exceptions.h"
#include "yaml-cpp/node/detail/iterator_fwd.h"
#include "yaml-cpp/node/type.h"
#include "yaml-cpp/parser.h"

YAMLConfigParser::YAMLConfigParser(std::string& log) : ConfigParser(log){
}

void YAMLConfigParser::Parse(){
	if( this->ConfigName == nullptr ){
		throw std::runtime_error("YAMLConfigParser::Parse() SetConfigFile() has not been called");
	}

	try{
		YAMLDoc = YAML::LoadFile(*(this->ConfigName));;
	}catch( YAML::ParserException& e ){
		throw std::runtime_error(e.what());
	}

	this->Configuration = YAMLDoc["Configuration"];
	if( !this->Configuration ){
		std::stringstream ss;
		ss << "YAMLConfigParser::Parse() : config file named \""
		   << *(this->ConfigName) 
		   << "\" is malformed and there is no Configuration node.";
		throw std::runtime_error(ss.str());
	}

	spdlog::get(this->LogName)->info("Parsing Description tag");
	ParseDescription();
	spdlog::get(this->LogName)->info("Parsing Author tag");
	ParseAuthor();
	spdlog::get(this->LogName)->info("Parsing Global tag");
	ParseGlobal();
	spdlog::get(this->LogName)->info("Parsing DetectorDriver tag");
	ParseDetectorDriver();
	spdlog::get(this->LogName)->info("Parsing Map tag");
	ParseMap();
}
		
void YAMLConfigParser::ParseDescription(){
	this->Description = this->Configuration["Description"];
	if( this->Description ){
		this->DescriptionText.reset(new std::string(this->Description.as<std::string>()));
	}
}

void YAMLConfigParser::ParseAuthor(){
	this->Author = this->Configuration["Author"];
	if( this->Author ){
		if( this->Author["Name"] )
			this->AuthorNameText.reset(new std::string(this->Author["Name"].as<std::string>()));
		if( this->Author["Email"] )
			this->AuthorEmailText.reset(new std::string(this->Author["Email"].as<std::string>()));
		if( this->Author["Date"] )
			this->AuthorDateText.reset(new std::string(this->Author["Date"].as<std::string>()));
	}

}

void YAMLConfigParser::ParseGlobal(){
	this->Global = this->Configuration["Global"];
	if( this->Global ){
		GlobalEventWidthInS = this->Global["EventWidth"].as<double>(-1.0);
		if( GlobalEventWidthInS < 0.0 ){
			std::stringstream ss;
			ss << "YAMLConfigParser::ParseGlobal() : config file named \""
		   	   << *(this->ConfigName) 
		   	   << "\" is malformed and Global node is missing EventWidth attribute. expect positive number";
			throw std::runtime_error(ss.str());
		}

		std::string GlobalEventWidthUnit = this->Global["EventWidthUnit"].as<std::string>("x");
		if( GlobalEventWidthUnit.compare("ns") == 0 ){
			GlobalEventWidthInS *= 1.0e-9;
		}else if( GlobalEventWidthUnit.compare("us") == 0 ){
			GlobalEventWidthInS *= 1.0e-6;
		}else if( GlobalEventWidthUnit.compare("ms") == 0 ){
			GlobalEventWidthInS *= 1.0e-3;
		}else if( GlobalEventWidthUnit.compare("s") == 0 ){
			GlobalEventWidthInS *= 1.0;
		}else{
				std::stringstream ss;
				ss << "YAMLConfigParser::ParseGlobal() : config file named \""
		   		   << *(this->ConfigName) 
		   		   << "\" is malformed and Global node is missing EventWidthUnit attribute. (s,ns,us,ms) are valid";
				throw std::runtime_error(ss.str());
		}
	}else{
		std::stringstream ss;
		ss << "YAMLConfigParser::ParseGlobal() : config file named \""
		   << *(this->ConfigName) 
		   << "\" is malformed and Global node is missing.";
		throw std::runtime_error(ss.str());
	}
}

void YAMLConfigParser::ParseDetectorDriver(){
	this->DetectorDriver = this->Configuration["DetectorDriver"];
	if( this->DetectorDriver ){

		YAML::Node processor = this->DetectorDriver["Processor"];
		YAML::Node analyzer = this->DetectorDriver["Analyzer"];
		if( (!processor) and (!analyzer) ){
			std::stringstream ss;
			ss << "XMLConfigParser::ParseDetectorDriver() : config file named \""
			   << *(this->ConfigName) 
			   << "\" is malformed. No Processors or Analyzers listed.";
			throw std::runtime_error(ss.str());
		}

		for( size_t ii = 0; ii < processor.size(); ++ii ){
			std::string name = processor[ii]["name"].as<std::string>();
			this->ProcessorNames[name] = processor[ii];
		}

		for( size_t ii = 0; ii < analyzer.size(); ++ii ){
			std::string name = analyzer[ii]["name"].as<std::string>();
			this->AnalyzerNames[name] = analyzer[ii];
		}


	}else{
		std::stringstream ss;
		ss << "YAMLConfigParser::ParseDetectorDriver() : config file named \""
		   << *(this->ConfigName) 
		   << "\" is malformed because DetectorDriver tag is missing.";
		throw std::runtime_error(ss.str());
	}

}

void YAMLConfigParser::ParseMap(){
	this->Map = this->Configuration["Map"];
}
