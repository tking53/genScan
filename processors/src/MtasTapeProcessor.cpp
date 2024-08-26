#include "MtasTapeProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <string>

MtasTapeProcessor::MtasTapeProcessor(const std::string& log) : Processor(log,"MtasTapeProcessor",{"tape"}){
//	this->h1dsettings = { 
//				{3612 , {16384,0.0,16384}}
//			    };
//
//	this->h2dsettings = {
//				{3900 , {512,0,512,16384,0.0,16384.0}}
//			    };
//	
	this->CycleCount = 0;
	this->CurrState = TapeCycleState::UNKNOWN;
}

[[maybe_unused]] bool MtasTapeProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	summary.GetDetectorSummary(this->AllDefaultRegex["tape"],this->SummaryData);
	for( const auto& evt : this->SummaryData ){
	}
	
	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool MtasTapeProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool MtasTapeProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->Reset();

	return true;
}

void MtasTapeProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void MtasTapeProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void MtasTapeProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}
		
void MtasTapeProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void MtasTapeProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	//MtasTape diagnostic plots, always want these no matter what
	this->console->info("Finished Declaring Plots");
}

void MtasTapeProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
}

void MtasTapeProcessor::CleanupTree(){
}

void MtasTapeProcessor::Reset(){
}

unsigned int MtasTapeProcessor::GetCurrentCycleNumber() const{
	return this->CycleCount;
}

MtasTapeProcessor::TapeCycleState MtasTapeProcessor::GetCurrentCycleState() const{
	return this->CurrState;
}

void MtasTapeProcessor::IncrementCycleNumber(){
	++(this->CycleCount);
}
