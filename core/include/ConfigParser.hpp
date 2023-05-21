#ifndef __CONFIG_PARSER_HPP__
#define __CONFIG_PARSER_HPP__

#include <string>

class ConfigParser{
	public:
		static ConfigParser* Get();
		void SetConfigFile(std::string*);
		void Parse();
	private:
		ConfigParser();
		
		static ConfigParser* instance;
		std::string* XMLName;
};

#endif
