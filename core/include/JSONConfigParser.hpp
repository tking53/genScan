#ifndef __JSON_CONFIG_PARSER_HPP__
#define __JSON_CONFIG_PARSER_HPP__

#include <string>
#include <vector>
#include <map>
#include <fstream>

#include <json/json.h>

#include "ConfigParser.hpp"

class JSONConfigParser : public ConfigParser{
	public:
		JSONConfigParser(std::string&);
		virtual void Parse();
	protected:
		virtual void ParseDescription();
		virtual void ParseAuthor();
		virtual void ParseGlobal();
		virtual void ParseDetectorDriver();
		virtual void ParseMap();
	private:
		std::ifstream finput;

		Json::Value JsonDoc;
		Json::Value Configuration;
		Json::Value Description;
		Json::Value Author;
		Json::Value Global;
		Json::Value DetectorDriver;
		Json::Value Map;

		std::map<std::string,Json::Value> ProcessorNames;
		std::map<std::string,Json::Value> AnalyzerNames;

};

#endif
