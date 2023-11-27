#include <stdexcept>
#include <sstream>

#include "ProcessorList.hpp"

#include "Processor.hpp"
#include "GenericProcessor.hpp"

#include "Analyzer.hpp"
#include "GenericAnalyzer.hpp"

ProcessorList::ProcessorList(const std::string& log){
	this->LogName = log;
}

ProcessorList::~ProcessorList() = default;

void ProcessorList::PreAnalyze(){
	for( auto& anal : this->analyzers )
		anal->PreProcess();
}

void ProcessorList::Analyze(){
	for( auto& anal : this->analyzers )
		anal->Process();
}

void ProcessorList::PostAnalyze(){
	for( auto& anal : this->analyzers )
		anal->PostProcess();
}

void ProcessorList::PreProcess(){
	for( auto& proc : this->processors )
		proc->PreProcess();
}

void ProcessorList::Process(){
	for( auto& proc : this->processors )
		proc->Process();
}

void ProcessorList::PostProcess(){
	for( auto& proc : this->processors )
		proc->PostProcess();
}

void ProcessorList::InitializeProcessors(XMLConfigParser* cmap){
	auto procnames = cmap->GetProcessorNames();
	for( auto& name : procnames ){
		if( name.compare("GenericProcessor") == 0 ){
			processors.push_back(std::make_shared<GenericProcessor>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeProcessors() Unknown processor named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		processors.back()->Init(cmap->GetProcessorXMLInfo(name));
	}
}

void ProcessorList::InitializeAnalyzers(XMLConfigParser* cmap){
	auto analnames = cmap->GetAnalyzerNames();
	for( auto& name : analnames ){
		if( name.compare("GenericAnalyzer") == 0 ){
			analyzers.push_back(std::make_shared<GenericAnalyzer>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeAnalyzers() Unknown analyzer named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		analyzers.back()->Init(cmap->GetAnalyzerXMLInfo(name));
	}
}

void ProcessorList::InitializeProcessors(YAMLConfigParser* cmap){
	auto procnames = cmap->GetProcessorNames();
	for( auto& name : procnames ){
		if( name.compare("GenericProcessor") == 0 ){
			processors.push_back(std::make_shared<GenericProcessor>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeProcessors() Unknown processor named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		processors.back()->Init(cmap->GetProcessorYAMLInfo(name));
	}
}

void ProcessorList::InitializeAnalyzers(YAMLConfigParser* cmap){
	auto analnames = cmap->GetAnalyzerNames();
	for( auto& name : analnames ){
		if( name.compare("GenericAnalyzer") == 0 ){
			analyzers.push_back(std::make_shared<GenericAnalyzer>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeAnalyzers() Unknown analyzer named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		analyzers.back()->Init(cmap->GetAnalyzerYAMLInfo(name));
	}
}
void ProcessorList::InitializeProcessors(JSONConfigParser* cmap){
	auto procnames = cmap->GetProcessorNames();
	for( auto& name : procnames ){
		if( name.compare("GenericProcessor") == 0 ){
			processors.push_back(std::make_shared<GenericProcessor>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeProcessors() Unknown processor named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		processors.back()->Init(cmap->GetProcessorJSONInfo(name));
	}
}

void ProcessorList::InitializeAnalyzers(JSONConfigParser* cmap){
	auto analnames = cmap->GetAnalyzerNames();
	for( auto& name : analnames ){
		if( name.compare("GenericAnalyzer") == 0 ){
			analyzers.push_back(std::make_shared<GenericAnalyzer>(this->LogName));
		}else{
			std::stringstream ss;
			ss << "ProcessorList::InitializeAnalyzers() Unknown analyzer named \""
			   << name 
			   << "\"";
			throw std::runtime_error(ss.str());
		}
		analyzers.back()->Init(cmap->GetAnalyzerJSONInfo(name));
	}
}

void ProcessorList::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	for( auto& proc : this->processors )
		proc->DeclarePlots(hismanager);
	for( auto& anal : this->analyzers )
		anal->DeclarePlots(hismanager);
}
