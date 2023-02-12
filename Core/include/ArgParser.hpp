#ifndef ARGPARSER_HPP
#define ARGPARSER_HPP

#include <string>
#include <vector>
#include <map>
#include <functional>

enum ArgType{
	no_argument,
	required_argument,
	optional_argument
};

class ArgParser{
	public:
		ArgParser() = default;
		~ArgParser() = default;

		struct Argument{
			Argument(std::string S,std::string L,std::string D,ArgType T) : sname(S), lname(L), description(D), type(T) {};
			std::string sname;
			std::string lname;
			std::string description;
			ArgType type;
			std::string value;
		};
		
		bool AddArgument(std::string,std::string,ArgType,std::string);
		void ParseArgs(int,char*[]);
	private:
		std::vector<ArgParser::Argument*>* GetArguments();
		void ShowUsage(char*);

		std::vector<ArgParser::Argument*> ArgList;
		std::map<std::string,Argument*> ShortArgs;
		std::map<std::string,Argument*> LongArgs;
};

#endif 
