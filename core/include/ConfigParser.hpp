#ifndef __CONFIG_PARSER_HPP__
#define __CONFIG_PARSER_HPP__

#include <string>
#include <vector>

#include <pugixml.hpp>
#include <pugiconfig.hpp>

class ConfigParser{
	public:
		static ConfigParser* Get();
		void SetConfigFile(std::string*);
		void Parse();
	private:
		ConfigParser();
		
		void ParseDescription();
		void ParseAuthor();
		void ParseGlobal();
		void ParseDetectorDriver();
		void ParseMap();

		static ConfigParser* instance;
		std::string* XMLName;
		pugi::xml_document XMLDoc;

		pugi::xml_node Configuration;

		pugi::xml_node Description;
		std::string* DescriptionText;

		pugi::xml_node Author;
		std::string* AuthorNameText;
		std::string* AuthorEmailText;
		std::string* AuthorDateText;

		pugi::xml_node Global;
		double GlobalEventWidthInS;

		pugi::xml_node DetectorDriver;
		std::vector<std::string> DetectorDriverProcessorNames;
		std::vector<std::string> DetectorDriverAnalyzerNames;

		pugi::xml_node Map;
};

#endif
