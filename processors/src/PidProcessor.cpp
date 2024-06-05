#include "PidProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>

PidProcessor::PidProcessor(const std::string& log) : Processor(log,"PidProcessor",{"pid"}){
}

[[maybe_unused]] bool PidProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();
	summary.GetDetectorSummary(this->AllDefaultRegex["pid"],this->SummaryData);

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool PidProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool PidProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

void PidProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
}

void PidProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
}

void PidProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
}
		
void PidProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void PidProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	(void) hismanager;
	this->console->info("Finished Declaring Plots");
}

TTree* PidProcessor::RegisterTree(){
	this->OutputTree = new TTree("Pid","Pid Processor output");
	return this->OutputTree;
}

void PidProcessor::CleanupTree(){
}
