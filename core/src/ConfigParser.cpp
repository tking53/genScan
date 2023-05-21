#include <stdexcept>

#include "ConfigParser.hpp"

ConfigParser* ConfigParser::instance = nullptr;

ConfigParser* ConfigParser::Get(){
	if( instance == nullptr ){
		instance = new ConfigParser();
	}
	return instance;
}

ConfigParser::ConfigParser(){
	XMLName = nullptr;
}

void ConfigParser::SetConfigFile(std::string* filename){
	XMLName = filename;
}

void ConfigParser::Parse(){
	if( XMLName == nullptr ){
		throw std::runtime_error("ConfigParser::Parse() SetConfigFile() has not been called");
	}
}
