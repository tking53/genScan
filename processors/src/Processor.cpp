#include <chrono>
#include <initializer_list>
#include <stdexcept>

#include "Processor.hpp"

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

Processor::Processor(const std::string& log,const std::string& proc,const std::initializer_list<std::string>& types){
	this->LogName = log;
	this->ProcessorName = proc;
	console = spdlog::get(this->LogName)->clone(this->ProcessorName);
	preprocesscalls = 0;
	processcalls = 0;
	postprocesscalls = 0;
	preprocesstime = 0.0;
	processtime = 0.0;
	postprocesstime = 0.0;
	currstep = STEP::UNKNOWN;
	Types = types;
	console->info("Created Processor : {}",this->ProcessorName);
	for( const auto& t : Types )
		console->info("Type : {} has been associated with this Processor",t);
}

Processor::~Processor(){
	console->info("PreProcessCalls : {} ProcessCalls : {} PostProcessCalls : {}",preprocesscalls,processcalls,postprocesscalls);
	console->info("PreProcess : {:.3f}ms Process: {:.3f}ms PostProcess : {:.3f}ms",preprocesstime,processtime,postprocesstime);
}

std::shared_ptr<Processor> Processor::GetPtr(){
	return shared_from_this();
}

[[nodiscard]] bool Processor::ContainsType(const std::string& type) const{
	if( Types.empty() ){
		return false;
	}else{
		return Types.find(type) != Types.end();
	}
}

[[noreturn]] void Processor::Init([[maybe_unused]] const Json::Value& node){
	console->error("Called Processor::Init(Json::Value& node), not the overload");
	throw std::runtime_error("Called Processor::Init(Json::Value& node), not the overload");
}

[[noreturn]] void Processor::Init([[maybe_unused]] const YAML::Node& node){
	console->error("Called Processor::Init(YAML::Node& node), not the overload");
	throw std::runtime_error("Called Processor::Init(YAML::Node& node), not the overload");
}

[[noreturn]] void Processor::Init([[maybe_unused]] const pugi::xml_node& node){
	console->error("Called Processor::Init(pugi::xml_node& node), not the overload");
	throw std::runtime_error("Called Processor::Init(pugi::xml_node& node), not the overload");
}

[[maybe_unused]] bool Processor::PreProcess(){
	start_time = std::chrono::high_resolution_clock::now();
	currstep = STEP::PREPROCESS;
	++preprocesscalls;
	return true;
}

[[maybe_unused]] bool Processor::Process(){
	start_time = std::chrono::high_resolution_clock::now();
	currstep = STEP::PROCESS;
	++processcalls;
	return true;
}

[[maybe_unused]] bool Processor::PostProcess(){
	start_time = std::chrono::high_resolution_clock::now();
	currstep = STEP::POSTPROCESS;
	++postprocesscalls;
	return true;
}

void Processor::EndProcess(){
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

		
[[noreturn]] void Processor::DeclarePlots([[maybe_unused]] PLOTS::PlotRegistry* hismanager) const{
	console->error("Called Processor::DeclarePlots(PLOTS::PlotRegistry* hismanager), not the overload");
	throw std::runtime_error("Called Processor::DeclarePlots(PLOTS::PlotRegistry* hismanager), not the overload");
}
