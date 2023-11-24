#ifndef __GENSCANOR_ARG_PARSER_HPP__
#define __GENSCANOR_ARG_PARSER_HPP__

#include <memory>
#include <string>
#include <vector>

#include "ArgValue.hpp"
#include "ArgParser.hpp"

class GenScanorArgParser : private ArgParser{
	public:
		static GenScanorArgParser* Get();	
		static GenScanorArgParser* Get(char*);	
		void ShowUsage();
		void ParseArgs(int,char*[]);
		std::string* GetConfigFile() const;
		std::string* GetOutputFile() const;
		std::string* GetDataFileType() const;
		std::vector<std::string>* GetInputFiles() const;
		bool* GetEvtBuild() const;
		int* GetLimit() const;
	private:
		GenScanorArgParser(char*);

		std::shared_ptr<ArgValue<std::string>> ConfigFile;
		std::shared_ptr<ArgValue<std::string>> OutputFile;
		std::shared_ptr<ArgValue<bool>> EvtBuild;
		std::shared_ptr<ArgValue<std::vector<std::string>>> FileNames;
		std::shared_ptr<ArgValue<int>> Limit;
		std::shared_ptr<ArgValue<std::string>> DataFileType;

		static GenScanorArgParser* instance;
};

#endif
