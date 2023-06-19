#ifndef __ARG_PARSER_HPP__
#define __ARG_PARSER_HPP__

#include <memory>
#include <string>
#include <vector>

#include "ArgValue.hpp"

class ArgParser{
	public:
		static ArgParser* Get();	
		static ArgParser* Get(char*);	
		void ShowUsage();
		void ParseArgs(int,char*[]);
		std::string GetConfigFile() const;
		std::string GetOutputFile() const;
		std::vector<std::string> GetInputFiles() const;
		bool GetEvtBuild() const;
	private:
		ArgParser(char*);

		std::string ProgName;

		std::shared_ptr<ArgValue<std::string>> ConfigFile;
		std::shared_ptr<ArgValue<std::string>> OutputFile;
		std::shared_ptr<ArgValue<bool>> EvtBuild;
		std::shared_ptr<ArgValue<std::vector<std::string>>> FileNames;
		std::shared_ptr<ArgValue<bool>> Help;

		static ArgParser* instance;
};

#endif
