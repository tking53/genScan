#ifndef __ARG_PARSER_HPP__
#define __ARG_PARSER_HPP__

#include <memory>
#include <string>
#include <vector>
#include <regex>

#include "ArgValue.hpp"

class ArgParser{
	public:
		ArgParser(char*);
		virtual void ShowUsage();
		virtual void ParseArgs(int,char*[]);
	protected:

		std::string ProgName;
		std::string fullargv;
		std::vector<std::smatch> ParseOptions(int,char*[]);
		std::shared_ptr<ArgValue<bool>> Help;
};

#endif
