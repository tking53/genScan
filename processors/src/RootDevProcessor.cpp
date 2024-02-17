#include "RootDevProcessor.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "PhysicsData.hpp"
#include "RootDevStruct.hpp"
#include <TTree.h>

RootDevProcessor::RootDevProcessor(const std::string& log) : Processor(log,"RootDevProcessor",{"RD"}){
}

[[maybe_unused]] bool RootDevProcessor::PreProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager){
	Processor::PreProcess();
	summary.GetDetectorSummary(this->DefaultRegex,this->SummaryData);
	for( const auto& evt : this->SummaryData ){
		this->CurrData.crateNum = evt->GetCrate();
		this->CurrData.modNum = evt->GetModule();
		this->CurrData.chanNum = evt->GetChannel();
		this->DataVec.push_back(this->CurrData);
	}
	this->CurrData = ProcessorStruct::DEFAULT_RD_STRUCT;
	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool RootDevProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager){
	return true;
}

[[maybe_unused]] bool RootDevProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager){
	return true;
}

void RootDevProcessor::Init(const YAML::Node&){
	this->console->info("Init called with YAML::Node");
}

void RootDevProcessor::Init(const Json::Value&){
	this->console->info("Init called with Json::Value");
}

void RootDevProcessor::Init(const pugi::xml_node&){
	this->console->info("Init called with pugi::xml_node");
}

void RootDevProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void RootDevProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	(void) hismanager;
	this->console->info("Finished Declaring Plots");
}

TTree* RootDevProcessor::RegisterTree(){
	this->OutputTree = new TTree("RootDev","RootDev Processor output");
	this->OutputTree->Branch("data_vec",&(this->DataVec));
	return this->OutputTree;
}

void RootDevProcessor::CleanupTree(){
	this->DataVec.clear();
}
