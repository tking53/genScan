#ifndef __YAML_CONFIG_PARSER_HPP__
#define __YAML_CONFIG_PARSER_HPP__

#include <string>
#include <vector>
#include <map>

#include <yaml-cpp/yaml.h>

#include "ChannelMap.hpp"
#include "ConfigParser.hpp"

class YAMLConfigParser : public ConfigParser{
	public:
		YAMLConfigParser(const std::string&);
		virtual void Parse(ChannelMap*);
		~YAMLConfigParser() = default;

		YAML::Node GetProcessorYAMLInfo(const std::string&) const;
		YAML::Node GetAnalyzerYAMLInfo(const std::string&) const;
	protected:
		virtual void ParseDescription();
		virtual void ParseAuthor();
		virtual void ParseGlobal();
		virtual void ParseDetectorDriver();
		virtual void ParseMap(ChannelMap*);
		virtual void ParseCuts();
	private:
		YAML::Node YAMLDoc;
		YAML::Node Configuration;
		YAML::Node Description;
		YAML::Node Author;
		YAML::Node Global;
		YAML::Node DetectorDriver;
		YAML::Node Map;
		YAML::Node Cuts;

		std::map<std::string,YAML::Node> ProcessorNames;
		std::map<std::string,YAML::Node> AnalyzerNames;

};

#endif
