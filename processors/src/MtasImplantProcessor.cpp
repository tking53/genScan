#include "MtasImplantProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>

MtasImplantProcessor::MtasImplantProcessor(const std::string& log) : Processor(log,"MtasImplantProcessor",{"mtasimplant"}){
}

[[maybe_unused]] bool MtasImplantProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();
	summary.GetDetectorSummary(this->AllDefaultRegex["mtasimplant"],this->SummaryData);

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool MtasImplantProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool MtasImplantProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

void MtasImplantProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
}

void MtasImplantProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
}

void MtasImplantProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
}
		
void MtasImplantProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void MtasImplantProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	(void) hismanager;
	this->console->info("Finished Declaring Plots");
}

TTree* MtasImplantProcessor::RegisterTree(){
	this->OutputTree = new TTree("MtasImplant","MtasImplant Processor output");
	return this->OutputTree;
}

void MtasImplantProcessor::CleanupTree(){
}
