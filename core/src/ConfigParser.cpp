#include <stdexcept>
#include <sstream>
#include <limits>
#include <regex>

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <string>

#include "ConfigParser.hpp"
#include "ChannelMap.hpp"

ConfigParser::ConfigParser(const std::string& log){
	this->LogName = log;
}

void ConfigParser::SetGlobalEventWidthInS(double width){
	GlobalEventWidthInS = width;
}

void ConfigParser::SetConfigFile(std::string* filename){
	this->ConfigName.reset(filename);
}

void ConfigParser::SetDescriptionText(std::string* txt){
	this->DescriptionText.reset(txt);
}

void ConfigParser::SetAuthorNameText(std::string* txt){
	this->AuthorNameText.reset(txt);
}

void ConfigParser::SetAuthorDateText(std::string* txt){
	this->AuthorDateText.reset(txt);
}

void ConfigParser::SetAuthorEmailText(std::string* txt){
	this->AuthorEmailText.reset(txt);
}

void ConfigParser::Parse(ChannelMap* cmap){
	throw std::runtime_error("ConfigParser::Parse() Should not be called");
	(void) cmap;
}
		
void ConfigParser::ParseDescription(){
	throw std::runtime_error("ConfigParser::ParseDescription() Should not be called");
}

void ConfigParser::ParseAuthor(){
	throw std::runtime_error("ConfigParser::ParseAuthor() Should not be called");
}

void ConfigParser::ParseGlobal(){
	throw std::runtime_error("ConfigParser::ParseGlobal() Should not be called");
}

void ConfigParser::ParseDetectorDriver(){
	throw std::runtime_error("ConfigParser::ParseDetectorDriver() Should not be called");
}

void ConfigParser::ParseMap(ChannelMap* cmap){
	throw std::runtime_error("ConfigParser::ParseMap() Should not be called");
	(void) cmap;
}
		
double ConfigParser::GetGlobalEventWidth() const{
	return this->GlobalEventWidthInS;
}
		
std::string* ConfigParser::GetConfigName() const{
	return this->ConfigName.get();
}

std::string* ConfigParser::GetDescriptionText() const{
	return this->DescriptionText.get();
}

std::string* ConfigParser::GetAuthorNameText() const{
	return this->AuthorNameText.get();
}

std::string* ConfigParser::GetAuthorDateText() const{
	return this->AuthorDateText.get();
}

std::string* ConfigParser::GetAuthorEmailText() const{
	return this->AuthorEmailText.get();
}
