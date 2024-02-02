#ifndef __CONFIG_PARSER_HPP__
#define __CONFIG_PARSER_HPP__

#include <string>
#include <vector>
#include <map>
#include <memory>

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

		void SetGlobalEventWidthInS(double);
		void SetConfigFile(std::string&);
		void SetDescriptionText(std::string*);
		void SetAuthorNameText(std::string*);
		void SetAuthorDateText(std::string*);
		void SetAuthorEmailText(std::string*);

		double GetGlobalEventWidth() const;
		std::string* GetConfigName() const;
		std::string* GetDescriptionText() const;
		std::string* GetAuthorNameText() const;
		std::string* GetAuthorDateText() const;
		std::string* GetAuthorEmailText() const;

		std::vector<std::string> GetProcessorNames() const;
		std::vector<std::string> GetAnalyzerNames() const;

		void AddProcessorName(const std::string&);
		void AddAnalyzerName(const std::string&);
	protected:
		[[noreturn]] virtual void ParseDescription();
		[[noreturn]] virtual void ParseAuthor();
		[[noreturn]] virtual void ParseGlobal();
		[[noreturn]] virtual void ParseDetectorDriver();
		[[noreturn]] virtual void ParseMap([[maybe_unused]] ChannelMap*);

		std::vector<std::string> ProcessorNames;
		std::vector<std::string> AnalyzerNames;
		
		std::string LogName;

		std::unique_ptr<std::string> ConfigName;

		std::unique_ptr<std::string> DescriptionText;

		std::unique_ptr<std::string> AuthorNameText;
		std::unique_ptr<std::string> AuthorEmailText;
		std::unique_ptr<std::string> AuthorDateText;

		double GlobalEventWidthInS;
};

#endif
