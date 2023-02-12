#include <iostream>

#include "ArgParser.hpp"

bool ArgParser::AddArgument(std::string SName,std::string LName,ArgType Type,std::string Description){
	if( ShortArgs.find(SName) != ShortArgs.end() or LongArgs.find(LName) != LongArgs.end() ){
		//trying to set same arg to multiple names
		return false;
	}else{
		ArgList.push_back( new Argument(SName,LName,Description,Type) );
		ShortArgs[SName] = ArgList.back();
		LongArgs[LName] = ArgList.back();
	}
}

std::vector<ArgParser::Argument*>* ArgParser::GetArguments(){
	return &ArgList;
}

void ArgParser::ShowUsage(char* progname){
	std::cout << progname;
	for( auto& arg : ArgList ){
		if( arg->type == ArgType::no_argument ){
			std::cout << " -" << arg->sname << "/--" << arg->lname << " " << arg->description;
		}else if( arg->type == ArgType::required_argument ){
			std::cout << " -" << arg->sname << "/--" << arg->lname << " [required_argument] " << arg->description;
		}else if( arg->type == ArgType::optional_argument ){
			std::cout << " -" << arg->sname << "/--" << arg->lname << " [optional_argument]" << arg->description;
		}else{
			//how did we get here?
		}
	}
	std::cout << std::endl;
	for( auto& arg : ArgList ){
		if( arg->type == ArgType::no_argument ){
			std::cout << " -" << arg->sname << "/--" << arg->lname << " " << arg->description << std::endl;
		}else if( arg->type == ArgType::required_argument ){
			std::cout << " -" << arg->sname << "/--" << arg->lname << " [required_argument] " << arg->description << std::endl;
		}else if( arg->type == ArgType::optional_argument ){
			std::cout << " -" << arg->sname << "/--" << arg->lname << " [optional_argument]" << arg->description << std::endl;
		}else{
			//how did we get here?
		}
	}
}

void ArgParser::ParseArgs(int argc,char* argv[]){
	//need to parge the arg list and move the values into string rep in the ArgList
}
