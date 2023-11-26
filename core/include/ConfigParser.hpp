#ifndef __CONFIG_PARSER_HPP__
#define __CONFIG_PARSER_HPP__

#include <string>
#include <vector>
#include <map>
#include <memory>

class ConfigParser{
	public:
		ConfigParser(const std::string&);
		virtual void Parse();

		void SetGlobalEventWidthInS(double);
		void SetConfigFile(std::string*);
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

	protected:
		virtual void ParseDescription();
		virtual void ParseAuthor();
		virtual void ParseGlobal();
		virtual void ParseDetectorDriver();
		virtual void ParseMap();
		
		std::string LogName;

		std::unique_ptr<std::string> ConfigName;

		std::unique_ptr<std::string> DescriptionText;

		std::unique_ptr<std::string> AuthorNameText;
		std::unique_ptr<std::string> AuthorEmailText;
		std::unique_ptr<std::string> AuthorDateText;

		double GlobalEventWidthInS;
};

#endif
