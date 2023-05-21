#ifndef ARGPARSER_HPP
#define ARGPARSER_HPP

#include <string>
#include <vector>
#include <map>
#include <functional>

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

enum ArgType{
	bad_argument,
	no_argument,
	required_argument,
	optional_argument
};

class ArgParser{
	public:
		ArgParser() = default;
		~ArgParser() = default;

		struct Argument{
			Argument(std::string S,std::string L,std::string D,ArgType T) : sname(S), lname(L), description(D), type(T), value(nullptr), isenabled(false) {};
			std::string sname;
			std::string lname;
			std::string description;
			ArgType type;
			std::string* value;
			bool isenabled;
		};
		
		bool AddArgument(std::string,std::string,ArgType,std::string);

		void ParseArgs(int,char*[]);
		
		void ShowUsage(char*);

		std::string* GetArgumentValueShortName(std::string);
		std::string* GetArgumentValueLongName(std::string);
		
		bool GetArgumentIsEnabledShortName(std::string);
		bool GetArgumentIsEnabledLongName(std::string);

	private:
		ArgParser::Argument* GetArgument(std::string);
		std::vector<ArgParser::Argument*>* GetArguments();

		std::vector<ArgParser::Argument*> ArgList;
		std::map<std::string,Argument*> ShortArgs;
		std::map<std::string,Argument*> LongArgs;
};

#endif 
