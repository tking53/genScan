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

ConfigParser::~ConfigParser() = default;

void ConfigParser::SetGlobalEventWidthInS(double width){
	GlobalEventWidthInS = width;
}

void ConfigParser::SetConfigFile(std::string& filename){
	this->ConfigName.reset(new std::string(filename));
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

[[noreturn]] void ConfigParser::Parse([[maybe_unused]] ChannelMap* cmap){
	throw std::runtime_error("ConfigParser::Parse() Should not be called");
}
		
[[noreturn]] void ConfigParser::ParseDescription(){
	throw std::runtime_error("ConfigParser::ParseDescription() Should not be called");
}

[[noreturn]] void ConfigParser::ParseAuthor(){
	throw std::runtime_error("ConfigParser::ParseAuthor() Should not be called");
}

[[noreturn]] void ConfigParser::ParseGlobal(){
	throw std::runtime_error("ConfigParser::ParseGlobal() Should not be called");
}

[[noreturn]] void ConfigParser::ParseDetectorDriver(){
	throw std::runtime_error("ConfigParser::ParseDetectorDriver() Should not be called");
}

[[noreturn]] void ConfigParser::ParseMap([[maybe_unused]] ChannelMap* cmap){
	throw std::runtime_error("ConfigParser::ParseMap() Should not be called");
}
		
double ConfigParser::GetGlobalEventWidth() const{
	return this->GlobalEventWidthInS;
}

double ConfigParser::GetGlobalEventWidthInNS() const{
	return this->GlobalEventWidthInS*1.0e9;
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

std::vector<std::string> ConfigParser::GetProcessorNames() const{
	return ProcessorNames;
}

std::vector<std::string> ConfigParser::GetAnalyzerNames() const{
	return AnalyzerNames;
}

void ConfigParser::AddProcessorName(const std::string& name){
	ProcessorNames.push_back(name);
}
void ConfigParser::AddAnalyzerName(const std::string& name){
	AnalyzerNames.push_back(name);
}

std::string* ConfigParser::GetCorrelationType() const{
	return this->CoincidenceType.get();
}

void ConfigParser::SetCorrelationType(std::string* val){
	this->CoincidenceType.reset(val);
}
