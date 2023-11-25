#ifndef __GENCONFIG_ARG_PARSER_HPP__
#define __GENCONFIG_ARG_PARSER_HPP__

#include <memory>
#include <string>
#include <vector>

#include "ArgValue.hpp"
#include "ArgParser.hpp"

class GenConfigArgParser : private ArgParser{
	public:
		static GenConfigArgParser* Get();	
		static GenConfigArgParser* Get(char*,const std::string&);	
		void ShowUsage();
		void ParseArgs(int,char*[]);
		std::string* GetConfigFile() const;
		std::string* GetOutputFile() const;
	private:
		GenConfigArgParser(char*,const std::string&);

		std::shared_ptr<ArgValue<std::string>> ConfigFile;
		std::shared_ptr<ArgValue<std::string>> OutputFile;

		static GenConfigArgParser* instance;
};

#endif
