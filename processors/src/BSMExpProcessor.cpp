#include "BSMExpProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>

BSMExpProcessor::BSMExpProcessor(const std::string& log) : Processor(log,"BSMExpProcessor",{"mtas","bsm"}){
	this->MtasProc = std::make_unique<MtasProcessor>(log);
	this->BSMProc = std::make_unique<BSMProcessor>(log);

	for( const auto& type : this->MtasProc->GetKnownTypes() ){
		this->AssociateType(type);
	}
	for( const auto& type : this->BSMProc->GetKnownTypes() ){
		this->AssociateType(type);
	}
}

[[maybe_unused]] bool BSMExpProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	this->MtasProc->PreProcess(summary,hismanager,cutmanager);
	this->BSMProc->PreProcess(summary,hismanager,cutmanager);

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool BSMExpProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){

	this->MtasProc->Process(summary,hismanager,cutmanager);
	this->BSMProc->Process(summary,hismanager,cutmanager);

	return true;
}

[[maybe_unused]] bool BSMExpProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){

	this->MtasProc->PostProcess(summary,hismanager,cutmanager);
	this->BSMProc->PostProcess(summary,hismanager,cutmanager);

	return true;
}

void BSMExpProcessor::Init(const YAML::Node& config){
	this->MtasProc->Init(config);
	this->BSMProc->Init(config);
}

void BSMExpProcessor::Init(const Json::Value& config){
	this->MtasProc->Init(config);
	this->BSMProc->Init(config);
}

void BSMExpProcessor::Init(const pugi::xml_node& config){
	this->MtasProc->Init(config);
	this->BSMProc->Init(config);
}
		
void BSMExpProcessor::Finalize(){
	this->MtasProc->Finalize();
	this->BSMProc->Finalize();
	this->console->info("{} has been finalized",this->ProcessorName);
}

void BSMExpProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	this->MtasProc->DeclarePlots(hismanager);
	this->BSMProc->DeclarePlots(hismanager);
	this->console->info("Finished Declaring Plots");
}

void BSMExpProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
	this->MtasProc->RegisterTree(outputtrees);
	this->BSMProc->RegisterTree(outputtrees);
}

void BSMExpProcessor::CleanupTree(){
	this->MtasProc->CleanupTree();
	this->BSMProc->CleanupTree();
}
