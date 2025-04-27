#ifndef __CONFIG_PARSER_HPP__
#define __CONFIG_PARSER_HPP__

#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <set>
#include <map>

#include <boost/container/flat_set.hpp>

#include <pugiconfig.hpp>
#include <pugixml.hpp>

#include "ChannelMap.hpp"

class ConfigParser{
	public:
		ConfigParser(const std::string&);
		void Parse(ChannelMap*);
		~ConfigParser();
		
		pugi::xml_node GetAnalyzerXMLInfo(const std::string&) const;
		pugi::xml_node GetProcessorXMLInfo(const std::string&) const;

		void SetConfigFile(std::string&);
		void SetDescriptionText(std::string*);
		void SetAuthorNameText(std::string*);
		void SetAuthorDateText(std::string*);
		void SetAuthorEmailText(std::string*);

		double GetGlobalEventWidth() const;
		double GetGlobalEventWidthInNS() const;
		void SetGlobalEventWidthInS(double);

		std::string* GetConfigName() const;
		std::string* GetDescriptionText() const;
		std::string* GetAuthorNameText() const;
		std::string* GetAuthorDateText() const;
		std::string* GetAuthorEmailText() const;

		std::vector<std::string> GetProcessorNames() const;
		std::vector<std::string> GetAnalyzerNames() const;

		void AddProcessorName(const std::string&);
		void AddAnalyzerName(const std::string&);

		std::string* GetCorrelationType() const;
		void SetCorrelationType(std::string*);

		std::vector<std::pair<std::string,std::string>> GetCutDetails() const;

		void GetConfigFileStr(std::stringstream&) const;
	protected:
		void ParseDescription();
		void ParseAuthor();
		void ParseGlobal();
		void ParseDetectorDriver();
		void ParseCuts();
		void ParseMap(ChannelMap*);
		void RecursiveNameCheck(pugi::xml_node&,std::set<std::string>&) const;

		std::vector<std::string> ProcessorNames;
		std::vector<std::string> AnalyzerNames;
		
		std::string LogName;

		std::unique_ptr<std::string> ConfigName;

		std::unique_ptr<std::string> DescriptionText;

		std::unique_ptr<std::string> AuthorNameText;
		std::unique_ptr<std::string> AuthorEmailText;
		std::unique_ptr<std::string> AuthorDateText;

		std::unique_ptr<std::string> CoincidenceType;

		std::vector<std::pair<std::string,std::string>> CutFiles;

		double GlobalEventWidthInS;

		boost::container::flat_set<int> KnownCrates;
		boost::container::flat_set<int> KnownBoardsInCrate;
		boost::container::flat_set<int> KnownChannelsInBoard;
	
		pugi::xml_document XMLDoc;

		pugi::xml_node Configuration;
		pugi::xml_node Description;
		pugi::xml_node Author;
		pugi::xml_node Global;
		pugi::xml_node DetectorDriver;
		pugi::xml_node Cuts;
		pugi::xml_node Map;

		std::map<std::string,pugi::xml_node> ProcessorNodes;
		std::map<std::string,pugi::xml_node> AnalyzerNodes;

};

#endif
