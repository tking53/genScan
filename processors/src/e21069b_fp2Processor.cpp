#include "e21069b_fp2Processor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>

e21069b_fp2Processor::e21069b_fp2Processor(const std::string& log) : Processor(log,"e21069b_fp2Processor",{"mtas","mtasimplant","pid"}){
	this->MtasProc = std::make_unique<MtasProcessor>(log);
	this->MtasImplantProc = std::make_unique<MtasImplantProcessor>(log);
	this->PidProc = std::make_unique<PidProcessor>(log);

	for( const auto& type : this->MtasProc->GetKnownTypes() ){
		this->AssociateType(type);
	}
	for( const auto& type : this->MtasImplantProc->GetKnownTypes() ){
		this->AssociateType(type);
	}
	for( const auto& type : this->PidProc->GetKnownTypes() ){
		this->AssociateType(type);
	}
}

[[maybe_unused]] bool e21069b_fp2Processor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	this->MtasProc->PreProcess(summary,hismanager,cutmanager);
	this->MtasImplantProc->PreProcess(summary,hismanager,cutmanager);
	this->PidProc->PreProcess(summary,hismanager,cutmanager);

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool e21069b_fp2Processor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){

	this->MtasProc->Process(summary,hismanager,cutmanager);
	this->MtasImplantProc->Process(summary,hismanager,cutmanager);
	this->PidProc->Process(summary,hismanager,cutmanager);

	return true;
}

[[maybe_unused]] bool e21069b_fp2Processor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){

	this->MtasProc->PostProcess(summary,hismanager,cutmanager);
	this->MtasImplantProc->PostProcess(summary,hismanager,cutmanager);
	this->PidProc->PostProcess(summary,hismanager,cutmanager);

	return true;
}

void e21069b_fp2Processor::Init(const YAML::Node& config){
	this->MtasProc->Init(config);
	this->MtasImplantProc->Init(config);
	this->PidProc->Init(config);
}

void e21069b_fp2Processor::Init(const Json::Value& config){
	this->MtasProc->Init(config);
	this->MtasImplantProc->Init(config);
	this->PidProc->Init(config);
}

void e21069b_fp2Processor::Init(const pugi::xml_node& config){
	this->MtasProc->Init(config);
	this->MtasImplantProc->Init(config);
	this->PidProc->Init(config);
}
		
void e21069b_fp2Processor::Finalize(){
	this->MtasProc->Finalize();
	this->MtasImplantProc->Finalize();
	this->PidProc->Finalize();
	this->console->info("{} has been finalized",this->ProcessorName);
}

void e21069b_fp2Processor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	this->MtasProc->DeclarePlots(hismanager);
	this->MtasImplantProc->DeclarePlots(hismanager);
	this->PidProc->DeclarePlots(hismanager);
	this->console->info("Finished Declaring Plots");
}

void e21069b_fp2Processor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
	this->MtasProc->RegisterTree(outputtrees);
	this->MtasImplantProc->RegisterTree(outputtrees);
	this->PidProc->RegisterTree(outputtrees);
}

void e21069b_fp2Processor::CleanupTree(){
	this->MtasProc->CleanupTree();
	this->MtasImplantProc->CleanupTree();
	this->PidProc->CleanupTree();
}
