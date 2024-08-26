#include "MtasSSDProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <string>

MtasSSDProcessor::MtasSSDProcessor(const std::string& log) : Processor(log,"MtasSSDProcessor",{"silicon"}){
//	this->h1dsettings = { 
//				{3612 , {16384,0.0,16384}}
//			    };
//
//	this->h2dsettings = {
//				{3900 , {512,0,512,16384,0.0,16384.0}}
//			    };
//	
	this->MaxEvent = nullptr;
}

[[maybe_unused]] bool MtasSSDProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	summary.GetDetectorSummary(this->AllDefaultRegex["silicon"],this->SummaryData);
	this->MaxEvent = summary.GetDetectorMaxEvent(this->SummaryData);

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool MtasSSDProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool MtasSSDProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->Reset();

	return true;
}

void MtasSSDProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void MtasSSDProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void MtasSSDProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}
		
void MtasSSDProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void MtasSSDProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	//MtasSSD diagnostic plots, always want these no matter what
	this->console->info("Finished Declaring Plots");
}

void MtasSSDProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
}

void MtasSSDProcessor::CleanupTree(){
}

void MtasSSDProcessor::Reset(){
	this->MaxEvent = nullptr;
}
