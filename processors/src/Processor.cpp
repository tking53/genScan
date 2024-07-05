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
#include <string>

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
		this->DefaultRegexString += t+":.*|";
		this->console->info("Type : [{}] has been associated with this Processor",t);
		this->AllDefaultRegex[t] = boost::regex(t+":.*",boost::regex_constants::optimize|boost::regex_constants::nosubs);
		this->console->info("Type Regex for {}, has been generated and is available",t);
	}
	if( this->DefaultRegexString.size() > 1 ){
		this->DefaultRegexString.pop_back();
	}
	this->DefaultRegexString += ")";
	this->DefaultRegex = boost::regex(this->DefaultRegexString,boost::regex_constants::optimize|boost::regex_constants::nosubs);
	this->AllDefaultRegex["ALL"] = this->DefaultRegex;
	this->console->info("Default Type Regex established to be {}",this->DefaultRegexString);
}

std::string Processor::GetProcessorName() const{
	return this->ProcessorName;
}

void Processor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
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
		if( this->DefaultRegexString.size() > 1 ){
			this->DefaultRegexString += "|"+t+":.*)";
		}else{
			this->DefaultRegexString += t+":.*)";
		}
		this->DefaultRegex = boost::regex(this->DefaultRegexString,boost::regex_constants::optimize|boost::regex_constants::nosubs);
		this->Types.insert(t);
		this->console->info("Type : [{}] has been associated with this Processor",t);
		this->console->info("Default Type Regex updated to be {}",this->DefaultRegexString);
		this->AllDefaultRegex[t] = boost::regex(t+":.*",boost::regex_constants::optimize|boost::regex_constants::nosubs);
		this->console->info("Type Regex for {}, has been generated and is available",t);
		this->AllDefaultRegex["ALL"] = this->DefaultRegex;
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

[[noreturn]] bool Processor::PreProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]]CUTS::CutRegistry* cutmanager){
	this->console->error("Called Processor::PreProcess(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*), not the overload");
	throw std::runtime_error("Called Processor::PreProcess(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*), not the overload");
}

[[noreturn]] bool Processor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]]CUTS::CutRegistry* cutmanager){
	this->console->error("Called Processor::Process(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*), not the overload");
	throw std::runtime_error("Called Processor::Process(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*), not the overload");
}

[[noreturn]] bool Processor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]]CUTS::CutRegistry* cutmanager){
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

[[nodiscard]] const boost::regex& Processor::GetDefaultRegex() const{
	return this->DefaultRegex;
}

[[nodiscard]] const std::unordered_map<std::string,boost::regex>& Processor::GetAllDefaultRegex() const{
	return this->AllDefaultRegex;
}
		
[[noreturn]] void Processor::DeclarePlots([[maybe_unused]] PLOTS::PlotRegistry* hismanager) const{
	this->console->error("Called Processor::DeclarePlots(PLOTS::PlotRegistry* hismanager), not the overload");
	throw std::runtime_error("Called Processor::DeclarePlots(PLOTS::PlotRegistry* hismanager), not the overload");
}

[[nodiscard]] std::set<std::string> Processor::GetKnownTypes() const{
	return this->Types;
}

void Processor::CleanupTree(){
}

void Processor::LoadHistogramSettings(const pugi::xml_node& config){
	for( pugi::xml_node histogram = config.child("Histogram"); histogram; histogram = histogram.next_sibling("Histogram") ){
		int id = histogram.attribute("id").as_int(0);
		auto currh1d = this->h1dsettings.find(id);
		auto currh2d = this->h2dsettings.find(id);
		if( currh1d != this->h1dsettings.end()){
			this->console->info("Found Different his settings for 1D histogram : {}",id);
			this->console->info("original X: {} {} {}",currh1d->second.nbinsx,currh1d->second.xlow,currh1d->second.xhigh);
			this->h1dsettings[id].nbinsx = histogram.attribute("nbinsx").as_int(currh1d->second.nbinsx);
			this->h1dsettings[id].xlow = histogram.attribute("xlow").as_double(currh1d->second.xlow);
			this->h1dsettings[id].xhigh = histogram.attribute("xhigh").as_double(currh1d->second.xhigh);
			this->console->info("X: {} {} {}",this->h1dsettings[id].nbinsx,this->h1dsettings[id].xlow,this->h1dsettings[id].xhigh);
		}else if( currh2d != this->h2dsettings.end() ){
			this->console->info("Found Different his settings for 2D histogram : {}",id);
			this->console->info("original X: {} {} {}",currh2d->second.nbinsx,currh2d->second.xlow,currh2d->second.xhigh);
			this->console->info("original Y: {} {} {}",currh2d->second.nbinsy,currh2d->second.ylow,currh2d->second.yhigh);
			this->h2dsettings[id].nbinsx = histogram.attribute("nbinsx").as_int(currh2d->second.nbinsx);
			this->h2dsettings[id].xlow = histogram.attribute("xlow").as_double(currh2d->second.xlow);
			this->h2dsettings[id].xhigh = histogram.attribute("xhigh").as_double(currh2d->second.xhigh);
			this->h2dsettings[id].nbinsy = histogram.attribute("nbinsy").as_int(currh2d->second.nbinsy);
			this->h2dsettings[id].ylow = histogram.attribute("ylow").as_double(currh2d->second.ylow);
			this->h2dsettings[id].yhigh = histogram.attribute("yhigh").as_double(currh2d->second.yhigh);
			this->console->info("New his settings for {}",id);
			this->console->info("X: {} {} {}",this->h2dsettings[id].nbinsx,this->h2dsettings[id].xlow,this->h2dsettings[id].xhigh);
			this->console->info("Y: {} {} {}",this->h2dsettings[id].nbinsy,this->h2dsettings[id].ylow,this->h2dsettings[id].yhigh);
		}else{
			std::string mess = "Unknown histogram id: "+std::to_string(id)+" for "+this->ProcessorName;
			throw std::runtime_error(mess);
		}
	}
}

void Processor::LoadHistogramSettings(const YAML::Node& config){
	YAML::Node histogram = config["Histogram"];
	for( size_t ii = 0; ii < histogram.size(); ++ii ){
		int id = histogram[ii]["id"].as<int>(0);
		auto currh1d = this->h1dsettings.find(id);
		auto currh2d = this->h2dsettings.find(id);
		if( currh1d != this->h1dsettings.end()){
			this->console->info("Found Different his settings for 1D histogram : {}",id);
			this->console->info("original X: {} {} {}",currh1d->second.nbinsx,currh1d->second.xlow,currh1d->second.xhigh);
			this->h1dsettings[id].nbinsx = histogram[ii]["nbinsx"].as<int>(currh1d->second.nbinsx);
			this->h1dsettings[id].xlow = histogram[ii]["xlow"].as<double>(currh1d->second.xlow);
			this->h1dsettings[id].xhigh = histogram[ii]["xhigh"].as<double>(currh1d->second.xhigh);
			this->console->info("X: {} {} {}",this->h1dsettings[id].nbinsx,this->h1dsettings[id].xlow,this->h1dsettings[id].xhigh);
		}else if( currh2d != this->h2dsettings.end() ){
			this->console->info("Found Different his settings for 2D histogram : {}",id);
			this->console->info("original X: {} {} {}",currh2d->second.nbinsx,currh2d->second.xlow,currh2d->second.xhigh);
			this->console->info("original Y: {} {} {}",currh2d->second.nbinsy,currh2d->second.ylow,currh2d->second.yhigh);
			this->h2dsettings[id].nbinsx = histogram[ii]["nbinsx"].as<int>(currh2d->second.nbinsx);
			this->h2dsettings[id].xlow = histogram[ii]["xlow"].as<double>(currh2d->second.xlow);
			this->h2dsettings[id].xhigh = histogram[ii]["xhigh"].as<double>(currh2d->second.xhigh);
			this->h2dsettings[id].nbinsy = histogram[ii]["nbinsy"].as<int>(currh2d->second.nbinsy);
			this->h2dsettings[id].ylow = histogram[ii]["ylow"].as<double>(currh2d->second.ylow);
			this->h2dsettings[id].yhigh = histogram[ii]["yhigh"].as<double>(currh2d->second.yhigh);
			this->console->info("New his settings for {}",id);
			this->console->info("X: {} {} {}",this->h2dsettings[id].nbinsx,this->h2dsettings[id].xlow,this->h2dsettings[id].xhigh);
			this->console->info("Y: {} {} {}",this->h2dsettings[id].nbinsy,this->h2dsettings[id].ylow,this->h2dsettings[id].yhigh);
		}else{
			std::string mess = "Unknown histogram id: "+std::to_string(id)+" for "+this->ProcessorName;
			throw std::runtime_error(mess);
		}
	}
}

void Processor::LoadHistogramSettings(const Json::Value& config){
	Json::Value histogram = config["Histogram"];
	for( const auto& his : histogram ){
		int id = his.get("id",0).asInt();
		auto currh1d = this->h1dsettings.find(id);
		auto currh2d = this->h2dsettings.find(id);
		if( currh1d != this->h1dsettings.end()){
			this->console->info("Found Different his settings for 1D histogram : {}",id);
			this->console->info("original X: {} {} {}",currh1d->second.nbinsx,currh1d->second.xlow,currh1d->second.xhigh);
			this->h1dsettings[id].nbinsx = his.get("nbinsx",currh1d->second.nbinsx).asInt();
			this->h1dsettings[id].xlow = his.get("xlow",currh1d->second.xlow).asDouble();
			this->h1dsettings[id].xhigh = his.get("xhigh",currh1d->second.xhigh).asDouble();
			this->console->info("X: {} {} {}",this->h1dsettings[id].nbinsx,this->h1dsettings[id].xlow,this->h1dsettings[id].xhigh);
		}else if( currh2d != this->h2dsettings.end() ){
			this->console->info("Found Different his settings for {}",id);
			this->console->info("original X: {} {} {}",currh2d->second.nbinsx,currh2d->second.xlow,currh2d->second.xhigh);
			this->console->info("original Y: {} {} {}",currh2d->second.nbinsy,currh2d->second.ylow,currh2d->second.yhigh);
			this->h2dsettings[id].nbinsx = his.get("nbinsx",currh2d->second.nbinsx).asInt();
			this->h2dsettings[id].xlow = his.get("xlow",currh2d->second.xlow).asDouble();
			this->h2dsettings[id].xhigh = his.get("xhigh",currh2d->second.xhigh).asDouble();
			this->h2dsettings[id].nbinsy = his.get("nbinsy",currh2d->second.nbinsy).asInt();
			this->h2dsettings[id].ylow = his.get("ylow",currh2d->second.ylow).asDouble();
			this->h2dsettings[id].yhigh = his.get("yhigh",currh2d->second.yhigh).asDouble();
			this->console->info("New his settings for {}",id);
			this->console->info("X: {} {} {}",this->h2dsettings[id].nbinsx,this->h2dsettings[id].xlow,this->h2dsettings[id].xhigh);
			this->console->info("Y: {} {} {}",this->h2dsettings[id].nbinsy,this->h2dsettings[id].ylow,this->h2dsettings[id].yhigh);
		}else{
			std::string mess = "Unknown histogram id: "+std::to_string(id)+" for "+this->ProcessorName;
			throw std::runtime_error(mess);
		}
	}
}
