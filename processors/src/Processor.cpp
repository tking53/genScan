#include <chrono>
#include <initializer_list>
#include <stdexcept>

#include "Processor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

Processor::Processor(const std::string& log,const std::string& proc,const std::initializer_list<std::string>& types){
	this->LogName = log;
	this->ProcessorName = proc;
	this->console = spdlog::get(this->LogName)->clone(this->ProcessorName);
	this->preprocesscalls = 0;
	this->processcalls = 0;
	this->postprocesscalls = 0;
	this->preprocesstime = 0.0;
	this->processtime = 0.0;
	this->postprocesstime = 0.0;
	this->currstep = STEP::UNKNOWN;
	this->Types = types;
	this->console->info("Created Processor : {}",this->ProcessorName);
	this->DefaultRegexString = "(";
	for( const auto& t : this->Types ){
		this->DefaultRegexString += t+"|";
		this->console->info("Type : [{}] has been associated with this Processor",t);
	}
	if( this->Types.size() == 1 ){
		this->DefaultRegexString.pop_back();
	}
	this->DefaultRegexString += ")";
	this->DefaultRegex = boost::regex(this->DefaultRegexString);
	this->console->info("Default Type Regex established to be {}",this->DefaultRegexString);
}

std::string Processor::GetProcessorName() const{
	return this->ProcessorName;
}

TTree* Processor::RegisterTree(){
	this->OutputTree = nullptr;
	return this->OutputTree;
}

Processor::~Processor(){
	this->console->info("PreProcessCalls : {} ProcessCalls : {} PostProcessCalls : {}",this->preprocesscalls,this->processcalls,this->postprocesscalls);
	this->console->info("PreProcess : {:.3f}s Process: {:.3f}s PostProcess : {:.3f}s",this->preprocesstime/1000.0,this->processtime/1000.0,this->postprocesstime/1000.0);
}

std::shared_ptr<Processor> Processor::GetPtr(){
	return shared_from_this();
}

[[nodiscard]] bool Processor::ContainsType(const std::string& type) const{
	if( this->Types.empty() ){
		return false;
	}else{
		return this->Types.find(type) != this->Types.end();
	}
}

[[nodiscard]] bool Processor::ContainsAnyType(const std::set<std::string>& type) const{
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

[[noreturn]] void Processor::Init([[maybe_unused]] const Json::Value& node){
	this->console->error("Called Processor::Init(Json::Value& node), not the overload");
	throw std::runtime_error("Called Processor::Init(Json::Value& node), not the overload");
}

[[noreturn]] void Processor::Init([[maybe_unused]] const YAML::Node& node){
	this->console->error("Called Processor::Init(YAML::Node& node), not the overload");
	throw std::runtime_error("Called Processor::Init(YAML::Node& node), not the overload");
}

[[noreturn]] void Processor::Init([[maybe_unused]] const pugi::xml_node& node){
	this->console->error("Called Processor::Init(pugi::xml_node& node), not the overload");
	throw std::runtime_error("Called Processor::Init(pugi::xml_node& node), not the overload");
}

void Processor::AssociateType(const std::string& t){
	if( this->Types.find(t) == this->Types.end() ){
		this->DefaultRegexString.pop_back();
		this->DefaultRegexString += "|"+t+")";
		this->DefaultRegex = boost::regex(this->DefaultRegexString);
		this->Types.insert(t);
		this->console->info("Type : [{}] has been associated with this Processor",t);
		this->console->info("Default Type Regex updated to be {}",this->DefaultRegexString);
	}else{
		this->console->critical("Type : [{}] is already associated with this Processor",t);
	}
}

[[noreturn]] void Processor::Finalize(){
	this->console->error("Called Processor::Finalize(), not the overload");
	throw std::runtime_error("Called Processor::Finalize(), not the overload");
}

[[maybe_unused]] bool Processor::PreProcess(){
	this->start_time = std::chrono::high_resolution_clock::now();
	this->currstep = STEP::PREPROCESS;
	++(this->preprocesscalls);
	return true;
}

[[maybe_unused]] bool Processor::Process(){
	this->start_time = std::chrono::high_resolution_clock::now();
	this->currstep = STEP::PROCESS;
	++(this->processcalls);
	return true;
}

[[maybe_unused]] bool Processor::PostProcess(){
	this->start_time = std::chrono::high_resolution_clock::now();
	this->currstep = STEP::POSTPROCESS;
	++(this->postprocesscalls);
	return true;
}

[[noreturn]] bool Processor::PreProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,CUTS::CutRegistry* cutmanager){
	this->console->error("Called Processor::PreProcess(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*), not the overload");
	throw std::runtime_error("Called Processor::PreProcess(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*), not the overload");
}

[[noreturn]] bool Processor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,CUTS::CutRegistry* cutmanager){
	this->console->error("Called Processor::Process(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*), not the overload");
	throw std::runtime_error("Called Processor::Process(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*), not the overload");
}

[[noreturn]] bool Processor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,CUTS::CutRegistry* cutmanager){
	this->console->error("Called Processor::PostProcess(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*), not the overload");
	throw std::runtime_error("Called Processor::PostProcess(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*), not the overload");
}

void Processor::EndProcess(){
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

		
[[noreturn]] void Processor::DeclarePlots([[maybe_unused]] PLOTS::PlotRegistry* hismanager) const{
	this->console->error("Called Processor::DeclarePlots(PLOTS::PlotRegistry* hismanager), not the overload");
	throw std::runtime_error("Called Processor::DeclarePlots(PLOTS::PlotRegistry* hismanager), not the overload");
}

void Processor::CleanupTree(){
}
