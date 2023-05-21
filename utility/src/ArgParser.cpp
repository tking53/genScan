#include <iostream>

#include "ArgParser.hpp"

bool ArgParser::AddArgument(std::string SName,std::string LName,ArgType Type,std::string Description){
	std::string SNameLookup = "-"+SName;
	std::string LNameLookup = "--"+LName;
	if( ShortArgs.find(SNameLookup) != ShortArgs.end() or LongArgs.find(LNameLookup) != LongArgs.end() ){
		//trying to set same arg to multiple names
		return false;
	}else{
		ArgList.push_back( new Argument(SNameLookup,LNameLookup,Description,Type) );
		ShortArgs[SNameLookup] = ArgList.back();
		LongArgs[LNameLookup] = ArgList.back();
		return true;
	}
}

std::vector<ArgParser::Argument*>* ArgParser::GetArguments(){
	return &ArgList;
}

void ArgParser::ShowUsage(char* progname){
	#ifdef USE_SPDLOG
		std::string message(progname);
	#else
		std::cout << progname;
	#endif
	for( auto& arg : ArgList ){
		if( arg->type == ArgType::no_argument ){
			#ifdef USE_SPDLOG
				message +=  " " + arg->sname + "/" + arg->lname + " " + arg->description;
			#else
				std::cout << " " << arg->sname << "/" << arg->lname << " " << arg->description;
			#endif
		}else if( arg->type == ArgType::required_argument ){
			#ifdef USE_SPDLOG
				message +=  " " + arg->sname + "/" + arg->lname + "[required_argument]" + arg->description;
			#else
				std::cout << " " << arg->sname << "/" << arg->lname << " [required_argument] " << arg->description;
			#endif
		}else if( arg->type == ArgType::optional_argument ){
			#ifdef USE_SPDLOG
				message +=  " " + arg->sname + "/" + arg->lname + "[optional_argument]" + arg->description;
			#else
				std::cout << " " << arg->sname << "/" << arg->lname << " [optional_argument]" << arg->description;
			#endif
		}else{
			//how did we get here?
		}
	}
	#ifdef USE_SPDLOG
		spdlog::get("console")->info(message);
	#else
		std::cout << std::endl;
	#endif
	for( auto& arg : ArgList ){
		if( arg->type == ArgType::no_argument ){
			#ifdef USE_SPDLOG
				spdlog::get("console")->info(" {}/{} {}",arg->sname,arg->lname,arg->description);
			#else
				std::cout << arg->sname << "/" << arg->lname << " " << arg->description << std::endl;
			#endif
		}else if( arg->type == ArgType::required_argument ){
			#ifdef USE_SPDLOG
				spdlog::get("console")->info(" {}/{} [required_argument] {}",arg->sname,arg->lname,arg->description);
			#else
				std::cout << arg->sname << "/" << arg->lname << " [required_argument] " << arg->description << std::endl;
			#endif
		}else if( arg->type == ArgType::optional_argument ){
			#ifdef USE_SPDLOG
				spdlog::get("console")->info(" {}/{} [optional_argument] {}",arg->sname,arg->lname,arg->description);
			#else
				std::cout << arg->sname << "/" << arg->lname << " [optional_argument]" << arg->description << std::endl;
			#endif
		}else{
			//how did we get here?
		}
	}
}

void ArgParser::ParseArgs(int argc,char* argv[]){
	//need to parge the arg list and move the values into string rep in the ArgList
	for( int ii = 1; ii < argc;){
		std::string flag(argv[ii]);
		ArgType type = ArgType::bad_argument;
		ArgParser::Argument* arg;

		try{
			arg = GetArgument(flag);
		}catch(...){
			std::string message = "Error Unknown flag : \"" + flag + "\" parsing option position " + std::to_string(ii) + " / " + std::to_string(argc);
			#ifdef USE_SPDLOG
				spdlog::get("console")->error("message");
			#endif
			throw message;
		}
		type = arg->type;

		if( type == ArgType::no_argument ){
			++ii;
			arg->isenabled = true;
		}else if( type == ArgType::required_argument ){
			++ii;
			if( ii < argc ){
				arg->value = new std::string(argv[ii]);
				arg->isenabled = true;
			}
			++ii;
		}else if( type == ArgType::optional_argument ){
			//need to check if this is filled or not
			//easiest done recursively?
			++ii;
			if( ii < argc ){
				std::string val(argv[ii]);
				if( ShortArgs.find(val) == ShortArgs.end() and LongArgs.find(val) == LongArgs.end() ){
					arg->value = new std::string(val);
					arg->isenabled = true;
					++ii;
				}
			}
		}else if( type == ArgType::bad_argument ){
			++ii;
		}else{
			++ii;
		}
	}
	for( auto& arg : ArgList ){
		if( arg->type == ArgType::required_argument and arg->value == nullptr ){
			ShowUsage(argv[0]);
			std::string message = "Missing Required Argument : " + arg->sname + " " + arg->lname;
			#ifdef USE_SPDLOG
				spdlog::get("console")->error("message");
			#endif
			throw message;
		}
	}
}
		
ArgParser::Argument* ArgParser::GetArgument(std::string name){
	auto sname = name;
	auto lname = name;
	auto ssearch = ShortArgs.find(sname);
	auto lsearch = LongArgs.find(lname);

	if( ssearch == ShortArgs.end() and lsearch == LongArgs.end() ){	
		throw "Bad Name";
	}
	if( ssearch != ShortArgs.end() ){
		return ssearch->second;
	}else{
		return lsearch->second;
	}
}

std::string* ArgParser::GetArgumentValueShortName(std::string sname){
	try{
		auto arg = GetArgument("-"+sname);
		return arg->value;
	}catch(...){
		std::string message = "Bad name in ArgParser::GetArgumentValueShortName : " + sname;
		#ifdef USE_SPDLOG
			spdlog::get("console")->error("message");
		#endif
		throw message;
	}
}

std::string* ArgParser::GetArgumentValueLongName(std::string lname){
	try{
		auto arg = GetArgument("--"+lname);
		return arg->value;
	}catch(...){
		std::string message = "Bad name in ArgParser::GetArgumentValueLongName : " + lname;
		#ifdef USE_SPDLOG
			spdlog::get("console")->error("message");
		#endif
		throw message;
	}
}
		
bool ArgParser::GetArgumentIsEnabledShortName(std::string sname){
	try{
		auto arg = GetArgument("-"+sname);
		return arg->isenabled;
	}catch(...){
		std::string message = "Bad name in ArgParser::GetArgumentIsEnabledShortName : " + sname;
		#ifdef USE_SPDLOG
			spdlog::get("console")->error("message");
		#endif
		throw message;
	}
}

bool ArgParser::GetArgumentIsEnabledLongName(std::string lname){
	try{
		auto arg = GetArgument("--"+lname);
		return arg->isenabled;
	}catch(...){
		std::string message = "Bad name in ArgParser::GetArgumentIsEnabledLongName : " + lname;
		#ifdef USE_SPDLOG
			spdlog::get("console")->error("message");
		#endif
		throw message;
	}
}
