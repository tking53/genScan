#include <stdexcept>
#include <sstream>

#include "ConfigParser.hpp"

ConfigParser* ConfigParser::instance = nullptr;

ConfigParser* ConfigParser::Get(){
	if( instance == nullptr ){
		instance = new ConfigParser();
	}
	return instance;
}

ConfigParser::ConfigParser(){
	this->XMLName = nullptr;
}

void ConfigParser::SetConfigFile(std::string* filename){
	this->XMLName = filename;
}

void ConfigParser::Parse(){
	if( this->XMLName == nullptr ){
		throw std::runtime_error("ConfigParser::Parse() SetConfigFile() has not been called");
	}

	pugi::xml_parse_result result = this->XMLDoc.load_file(this->XMLName->c_str());

	if( !result ){
		std::stringstream ss;
		ss << "ConfigParser::Parse() : Unable to open xml file named\""
		   << *(this->XMLName) 
		   << "\". pugixml reports : "
		   << result.description();
		throw std::runtime_error(ss.str());
	}
	this->Configuration = this->XMLDoc.child("Configuration");
	if( !this->Configuration ){
		std::stringstream ss;
		ss << "ConfigParser::Parse() : config file named \""
		   << *(this->XMLName) 
		   << "\" is malformed and there is no Configuration node.";
		throw std::runtime_error(ss.str());
	}

	ParseDescription();
	ParseAuthor();
	ParseGlobal();
	ParseDetectorDriver();
	ParseMap();
}
		
void ConfigParser::ParseDescription(){
	this->Description = this->Configuration.child("Description");
	if( this->Description ){
		this->DescriptionText = new std::string(this->Description.text().get());
	}
}

void ConfigParser::ParseAuthor(){
	this->Author = this->Configuration.child("Author");
	if( this->Author ){
		if( this->Author.child("Name") )
			AuthorNameText = new std::string(this->Author.child("Name").text().get());
		if( this->Author.child("Email") )
			AuthorEmailText = new std::string(this->Author.child("Email").text().get());
		if( this->Author.child("Date") )
			AuthorDateText = new std::string(this->Author.child("Date").text().get());
	}
}

void ConfigParser::ParseGlobal(){
	this->Global = this->Configuration.child("Global");
	if( this->Global ){
		GlobalEventWidthInS = this->Global.attribute("EventWidth").as_double(-1.0);
		if( GlobalEventWidthInS < 0.0 ){
			std::stringstream ss;
			ss << "ConfigParser::ParseGlobal() : config file named \""
		   	   << *(this->XMLName) 
		   	   << "\" is malformed and Global node is missing EventWidth attribute. expect positive number";
			throw std::runtime_error(ss.str());
		}

		std::string GlobalEventWidthUnit = this->Global.attribute("EventWidthUnit").as_string("x");
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
				ss << "ConfigParser::ParseGlobal() : config file named \""
		   		   << *(this->XMLName) 
		   		   << "\" is malformed and Global node is missing EventWidthUnit attribute. (s,ns,us,ms) are valid";
				throw std::runtime_error(ss.str());
		}
	}else{
		std::stringstream ss;
		ss << "ConfigParser::ParseGlobal() : config file named \""
		   << *(this->XMLName) 
		    << "\" is malformed and Global node is missing.";
		throw std::runtime_error(ss.str());
	}
}

void ConfigParser::ParseDetectorDriver(){
	this->DetectorDriver = this->Configuration.child("DetectorDriver");
	if( this->DetectorDriver ){
		//loop through list of analyzers and processors
	}else{
		//just make raw coincidences but warn/critical????
	}
}

void ConfigParser::ParseMap(){
	this->Map = this->Configuration.child("Map");
	if( this->Map ){
		//create the channel map object that will be used for lookups. Make it global instanced
	}else{
		//throw error
	}
}
