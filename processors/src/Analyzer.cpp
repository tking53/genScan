#include <initializer_list>
#include <stdexcept>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>


#include "Analyzer.hpp"

Analyzer::Analyzer(const std::string& log,const std::string& analyzer,const std::initializer_list<std::string>& types){
	this->AnalyzerName = analyzer;
	this->LogName = log;
	this->console = spdlog::get(this->LogName)->clone(this->AnalyzerName);
	this->preprocesscalls = 0;
	this->processcalls = 0;
	this->postprocesscalls = 0;
	this->preprocesstime = 0.0;
	this->processtime = 0.0;
	this->postprocesstime = 0.0;
	this->Types = types;
	this->console->info("Created Analyzer : {}",this->AnalyzerName);
	this->DefaultRegexString = "(";
	for( const auto& t : this->Types ){
		this->DefaultRegexString += t+"|";
		this->console->info("Type : {} has been associated with this Analyzer",t);
	}
	if( this->Types.size() == 1 ){
		this->DefaultRegexString.pop_back();
	}
	this->DefaultRegexString += ")";
	this->DefaultRegex = std::regex(this->DefaultRegexString);
	this->console->info("Default Type Regex established to be {}",this->DefaultRegexString);
}

Analyzer::~Analyzer(){
	this->console->info("PreProcessCalls : {} ProcessCalls : {} PostProcessCalls : {}",this->preprocesscalls,this->processcalls,this->postprocesscalls);
	this->console->info("PreProcess : {:.3f}s Process: {:.3f}s PostProcess : {:.3f}s",this->preprocesstime/1000.0,this->processtime/1000.0,this->postprocesstime/1000.0);
}

std::shared_ptr<Analyzer> Analyzer::GetPtr(){
	return shared_from_this();
}

std::string Analyzer::GetAnalyzerName() const{
	return this->AnalyzerName;
}

[[nodiscard]] bool Analyzer::ContainsType(const std::string& type) const{
	if( this->Types.empty() ){
		return false;
	}else{
		return this->Types.find(type) != this->Types.end();
	}
}

[[nodiscard]] bool Analyzer::ContainsAnyType(const std::set<std::string>& type) const{
	if( this->Types.empty() ){
		return false;
	}else{
		for( const auto& t : type ){
			if( this->Types.find(t) != this->Types.end() ){
				return true;
			}
		}
		return false;
	}
}

[[noreturn]] void Analyzer::Init([[maybe_unused]] const Json::Value& node){
	console->error("Called Analyzer::Init(Json::Value& node), not the overload");
	throw std::runtime_error("Called Analyzer::Init(Json::Value& node), not the overload");
}

[[noreturn]] void Analyzer::Init([[maybe_unused]] const YAML::Node& node){
	console->error("Called Analyzer::Init(YAML::Node& node), not the overload");
	throw std::runtime_error("Called Analyzer::Init(YAML::Node& node), not the overload");
}

[[noreturn]] void Analyzer::Init([[maybe_unused]] const pugi::xml_node& node){
	console->error("Called Analyzer::Init(pugi::xml_node& node), not the overload");
	throw std::runtime_error("Called Analyzer::Init(pugi::xml_node& node), not the overload");
}

void Analyzer::AssociateType(const std::string& t){
	if( this->Types.find(t) == this->Types.end() ){
		this->DefaultRegexString.pop_back();
		this->DefaultRegexString += "|"+t+")";
		this->Types.insert(t);
		this->DefaultRegex = std::regex(this->DefaultRegexString);
		this->console->info("Type : {} has been associated with this Analyzer",t);
		this->console->info("Default Type Regex updated to be {}",this->DefaultRegexString);
	}else{
		this->console->critical("Type : {} is already associated with this Analyzer",t);
	}
}

[[noreturn]] void Analyzer::Finalize(){
	this->console->error("Called Analyzer::Finalize(), not the overload");
	throw std::runtime_error("Called Analyzer::Finalize(), not the overload");
}


[[maybe_unused]] bool Analyzer::PreProcess(){
	this->start_time = std::chrono::high_resolution_clock::now();
	this->currstep = STEP::PREPROCESS;
	++(this->preprocesscalls);
	return true;
}

[[maybe_unused]] bool Analyzer::Process(){
	this->start_time = std::chrono::high_resolution_clock::now();
	this->currstep = STEP::PROCESS;
	++(this->processcalls);
	return true;
}

[[maybe_unused]] bool Analyzer::PostProcess(){
	this->start_time = std::chrono::high_resolution_clock::now();
	this->currstep = STEP::POSTPROCESS;
	++(this->postprocesscalls);
	return true;
}

[[noreturn]] bool Analyzer::PreProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager){
	this->console->error("Called Analyzer::PreProcess(EventSummary&,PLOTS::PlotRegistry*), not the overload");
	throw std::runtime_error("Called Analyzer::PreProcess(EventSummary&,PLOTS::PlotRegistry*), not the overload");
}

[[noreturn]] bool Analyzer::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager){
	this->console->error("Called Analyzer::Process(EventSummary&,PLOTS::PlotRegistry*), not the overload");
	throw std::runtime_error("Called Analyzer::Process(EventSummary&,PLOTS::PlotRegistry*), not the overload");
}

[[noreturn]] bool Analyzer::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager){
	this->console->error("Called Analyzer::PostProcess(EventSummary&,PLOTS::PlotRegistry*), not the overload");
	throw std::runtime_error("Called Analyzer::PostProcess(EventSummary&,PLOTS::PlotRegistry*), not the overload");
}

void Analyzer::EndProcess(){
	this->stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double,std::milli> dur = this->stop_time - this->start_time;
	switch (this->currstep) {
		case STEP::PREPROCESS:
			this->preprocesstime += dur.count();
			break;
		case STEP::PROCESS:
			this->processtime += dur.count();
			break;
		case STEP::POSTPROCESS:
			this->postprocesstime += dur.count();
			break;
		[[unlikely]] default:
			break;
	}
}

[[noreturn]] void Analyzer::DeclarePlots([[maybe_unused]] PLOTS::PlotRegistry* hismanager) const{
	this->console->error("Called Analyzer::DeclarePlots(PLOTS::PlotRegistry* hismanager), not the overload");
	throw std::runtime_error("Called Analyzer::DeclarePlots(PLOTS::PlotRegistry* hismanager), not the overload");
}
