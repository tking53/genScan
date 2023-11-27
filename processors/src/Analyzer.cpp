#include <stdexcept>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>


#include "Analyzer.hpp"

Analyzer::Analyzer(const std::string& log,const std::string& analyzer){
	this->AnalyzerName = analyzer;
	this->LogName = log;
	console = spdlog::get(this->LogName)->clone(this->AnalyzerName);
	preprocesscalls = 0;
	processcalls = 0;
	postprocesscalls = 0;
	preprocesstime = 0.0;
	processtime = 0.0;
	postprocesstime = 0.0;
	console->info("Created AnalyzerName : {}",this->AnalyzerName);
}

Analyzer::~Analyzer(){
	console->info("PreProcessCalls : {} ProcessCalls : {} PostProcessCalls : {}",preprocesscalls,processcalls,postprocesscalls);
	console->info("PreProcess : {:.3f}ms Process: {:.3f}ms PostProcess : {:.3f}ms",preprocesstime,processtime,postprocesstime);
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

bool Analyzer::PreProcess(){
	start_time = std::chrono::high_resolution_clock::now();
	currstep = STEP::PREPROCESS;
	++preprocesscalls;
	return true;
}

bool Analyzer::Process(){
	start_time = std::chrono::high_resolution_clock::now();
	currstep = STEP::PROCESS;
	++processcalls;
	return true;
}

bool Analyzer::PostProcess(){
	start_time = std::chrono::high_resolution_clock::now();
	currstep = STEP::POSTPROCESS;
	++postprocesscalls;
	return true;
}

void Analyzer::EndProcess(){
	stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double,std::milli> dur = stop_time - start_time;
	switch (currstep) {
		case STEP::PREPROCESS:
			preprocesstime += dur.count();
			break;
		case STEP::PROCESS:
			processtime += dur.count();
			break;
		case STEP::POSTPROCESS:
			postprocesstime += dur.count();
			break;
		[[unlikely]] default:
			break;
	}
}

[[noreturn]] void Analyzer::DeclarePlots([[maybe_unused]] PLOTS::PlotRegistry* hismanager) const{
	console->error("Called Analyzer::DeclarePlots(PLOTS::PlotRegistry* hismanager), not the overload");
	throw std::runtime_error("Called Analyzer::DeclarePlots(PLOTS::PlotRegistry* hismanager), not the overload");
}
