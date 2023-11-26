#ifndef __XML_CONFIG_PARSER_HPP__
#define __XML_CONFIG_PARSER_HPP__

#include <string>
#include <vector>
#include <map>

#include <pugixml.hpp>
#include <pugiconfig.hpp>

#include "ConfigParser.hpp"
#include "ChannelMap.hpp"

class XMLConfigParser : public ConfigParser{
	public:
		XMLConfigParser(const std::string&);
		virtual void Parse(ChannelMap*);
		std::map<std::string,pugi::xml_node>& GetAnalyzers();
		std::map<std::string,pugi::xml_node>& GetProcessors();
	protected:
		virtual void ParseDescription();
		virtual void ParseAuthor();
		virtual void ParseGlobal();
		virtual void ParseDetectorDriver();
		virtual void ParseMap(ChannelMap*);
	private:
		pugi::xml_document XMLDoc;

		pugi::xml_node Configuration;
		pugi::xml_node Description;
		pugi::xml_node Author;
		pugi::xml_node Global;
		pugi::xml_node DetectorDriver;
		pugi::xml_node Map;

		std::map<std::string,pugi::xml_node> ProcessorNames;
		std::map<std::string,pugi::xml_node> AnalyzerNames;
};

#endif
