#include <stdexcept>
#include <sstream>

#include "ProcessorList.hpp"

#include "Processor.hpp"
#include "GenericProcessor.hpp"

#include "Analyzer.hpp"
#include "GenericAnalyzer.hpp"

ProcessorList* ProcessorList::instance = nullptr;

ProcessorList::ProcessorList(){
}

ProcessorList* ProcessorList::Get(){
	if( instance == nullptr ){
		instance = new ProcessorList();
	}
	return instance;
}

void ProcessorList::InitializeProcessors(std::map<std::string,pugi::xml_node>& procnames){
	for( auto& kv : procnames ){
		if( kv.first.compare("GenericProcessor") == 0 ){
			processors.push_back(new GenericProcessor());
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeProcessors() Unknown processor named \""
			   << kv.first 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
	}
}

void ProcessorList::InitializeAnalyzers(std::map<std::string,pugi::xml_node>& analnames){
	for( auto& kv : analnames ){
		if( kv.first.compare("GenericAnalyzer") == 0 ){
			analyzers.push_back(new GenericAnalyzer());
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeAnalyzers() Unknown analyzer named \""
			   << kv.first 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
	}
}
