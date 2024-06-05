#include "e21069bProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>

e21069bProcessor::e21069bProcessor(const std::string& log) : Processor(log,"e21069bProcessor",{}){
}

[[maybe_unused]] bool e21069bProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();
	summary.GetDetectorSummary(this->AllDefaultRegex["pid"],this->SummaryData);

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool e21069bProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool e21069bProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

void e21069bProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
}

void e21069bProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
}

void e21069bProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
}
		
void e21069bProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void e21069bProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	(void) hismanager;
	this->console->info("Finished Declaring Plots");
}

TTree* e21069bProcessor::RegisterTree(){
	this->OutputTree = new TTree("e21069b","e21069b Processor output");
	return this->OutputTree;
}

void e21069bProcessor::CleanupTree(){
}
