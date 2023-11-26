#ifndef __YAML_CONFIG_PARSER_HPP__
#define __YAML_CONFIG_PARSER_HPP__

#include <string>
#include <vector>
#include <map>

#include <yaml-cpp/yaml.h>

#include "ConfigParser.hpp"

class YAMLConfigParser : public ConfigParser{
	public:
		YAMLConfigParser(const std::string&);
		virtual void Parse();
	protected:
		virtual void ParseDescription();
		virtual void ParseAuthor();
		virtual void ParseGlobal();
		virtual void ParseDetectorDriver();
		virtual void ParseMap();
	private:
		YAML::Node YAMLDoc;
		YAML::Node Configuration;
		YAML::Node Description;
		YAML::Node Author;
		YAML::Node Global;
		YAML::Node DetectorDriver;
		YAML::Node Map;

		std::map<std::string,YAML::Node> ProcessorNames;
		std::map<std::string,YAML::Node> AnalyzerNames;

};

#endif
