#ifndef __JSON_CONFIG_PARSER_HPP__
#define __JSON_CONFIG_PARSER_HPP__

#include <string>
#include <vector>
#include <map>
#include <fstream>

#include <json/json.h>

#include "ConfigParser.hpp"
#include "ChannelMap.hpp"

class JSONConfigParser : public ConfigParser{
	public:
		JSONConfigParser(const std::string&);
		virtual void Parse(ChannelMap*);
		~JSONConfigParser() = default;

		Json::Value GetProcessorJSONInfo(const std::string&) const;
		Json::Value GetAnalyzerJSONInfo(const std::string&) const;
	protected:
		virtual void ParseDescription();
		virtual void ParseAuthor();
		virtual void ParseGlobal();
		virtual void ParseDetectorDriver();
		virtual void ParseMap(ChannelMap*);
		virtual void ParseCuts();
	private:
		std::ifstream finput;

		Json::Value JsonDoc;
		Json::Value Configuration;
		Json::Value Description;
		Json::Value Author;
		Json::Value Global;
		Json::Value DetectorDriver;
		Json::Value Map;
		Json::Value Cuts;

		std::map<std::string,Json::Value> ProcessorNames;
		std::map<std::string,Json::Value> AnalyzerNames;

};

#endif
