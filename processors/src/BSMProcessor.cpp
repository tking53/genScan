#include "BSMProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>

BSMProcessor::BSMProcessor(const std::string& log) : Processor(log,"BSMProcessor",{"bsm"}){
}

[[maybe_unused]] bool BSMProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();
	summary.GetDetectorSummary(this->AllDefaultRegex["bsm"],this->SummaryData);

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool BSMProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool BSMProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

void BSMProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
}

void BSMProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
}

void BSMProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
}
		
void BSMProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void BSMProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	(void) hismanager;
	this->console->info("Finished Declaring Plots");
}

void BSMProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
}

void BSMProcessor::CleanupTree(){
}
