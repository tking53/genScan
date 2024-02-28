#ifndef __CONFIG_PARSER_HPP__
#define __CONFIG_PARSER_HPP__

#include <string>
#include <vector>
#include <memory>

#include <boost/container/flat_set.hpp>

#include <pugiconfig.hpp>
#include <pugixml.hpp>

#include <yaml-cpp/yaml.h>

#include <json/json.h>

#include "ChannelMap.hpp"

class ConfigParser{
	public:
		ConfigParser(const std::string&);
		[[noreturn]] virtual void Parse([[maybe_unused]] ChannelMap*);
		virtual ~ConfigParser();

		virtual void SetConfigFile(std::string&);
		virtual void SetDescriptionText(std::string*);
		virtual void SetAuthorNameText(std::string*);
		virtual void SetAuthorDateText(std::string*);
		virtual void SetAuthorEmailText(std::string*);

		virtual double GetGlobalEventWidth() const;
		virtual double GetGlobalEventWidthInNS() const;
		virtual void SetGlobalEventWidthInS(double);

		virtual std::string* GetConfigName() const;
		virtual std::string* GetDescriptionText() const;
		virtual std::string* GetAuthorNameText() const;
		virtual std::string* GetAuthorDateText() const;
		virtual std::string* GetAuthorEmailText() const;

		virtual std::vector<std::string> GetProcessorNames() const;
		virtual std::vector<std::string> GetAnalyzerNames() const;

		virtual void AddProcessorName(const std::string&);
		virtual void AddAnalyzerName(const std::string&);

		virtual std::string* GetCorrelationType() const;
		virtual void SetCorrelationType(std::string*);

		virtual std::vector<std::pair<std::string,std::string>> GetCutDetails() const final;
	protected:
		[[noreturn]] virtual void ParseDescription();
		[[noreturn]] virtual void ParseAuthor();
		[[noreturn]] virtual void ParseGlobal();
		[[noreturn]] virtual void ParseDetectorDriver();
		[[noreturn]] virtual void ParseCuts();
		[[noreturn]] virtual void ParseMap([[maybe_unused]] ChannelMap*);

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
};

#endif
