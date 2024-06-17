#include "ribf168Processor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>

ribf168Processor::ribf168Processor(const std::string& log) : Processor(log,"ribf168Processor",{"hagrid","ionchamber","pspmt","pid"}){
	this->HagridProc = std::make_unique<HagridProcessor>(log);
	this->RIKENIonizationChamberProc = std::make_unique<RIKENIonizationChamberProcessor>(log);
	this->PidProc = std::make_unique<PidProcessor>(log);
	this->PSPMTProc = std::make_unique<PSPMTProcessor>(log);

	for( const auto& type : this->HagridProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	for( const auto& type : this->RIKENIonizationChamberProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	for( const auto& type : this->PidProc->GetKnownTypes() ){
		this->AssociateType(type);
	}
	
	for( const auto& type : this->PSPMTProc->GetKnownTypes() ){
		this->AssociateType(type);
	}
}

[[maybe_unused]] bool ribf168Processor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	auto types = summary.GetKnownTypes();

	this->HasHagrid = (types.find("hagrid") != types.end());
	this->HasRIKENIonChamber = (types.find("ionchamber") != types.end());
	this->HasPid = (types.find("pid") != types.end());
	this->HasPSPMT = (types.find("pspmt") != types.end());

	if( this->HasHagrid ){
		this->HagridProc->PreProcess(summary,hismanager,cutmanager);
	}

	if( this->HasRIKENIonChamber ){
		this->RIKENIonizationChamberProc->PreProcess(summary,hismanager,cutmanager);
	}

	if( this->HasPid ){
		this->PidProc->PreProcess(summary,hismanager,cutmanager);
	}

	if( this->HasPSPMT ){
		this->PSPMTProc->PreProcess(summary,hismanager,cutmanager);
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool ribf168Processor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	
	if( this->HasHagrid ){
		this->HagridProc->Process(summary,hismanager,cutmanager);
	}

	if( this->HasRIKENIonChamber ){
		this->RIKENIonizationChamberProc->Process(summary,hismanager,cutmanager);
	}

	if( this->HasPid ){
		this->PidProc->Process(summary,hismanager,cutmanager);
	}

	if( this->HasPSPMT ){
		this->PSPMTProc->Process(summary,hismanager,cutmanager);
	}

	return true;
}

[[maybe_unused]] bool ribf168Processor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	
	if( this->HasHagrid ){
		this->HagridProc->PostProcess(summary,hismanager,cutmanager);
	}

	if( this->HasRIKENIonChamber ){
		this->RIKENIonizationChamberProc->PostProcess(summary,hismanager,cutmanager);
	}

	if( this->HasPid ){
		this->PidProc->PostProcess(summary,hismanager,cutmanager);
	}

	if( this->HasPSPMT ){
		this->PSPMTProc->PostProcess(summary,hismanager,cutmanager);
	}

	return true;
}

void ribf168Processor::Init(const YAML::Node& config){
	this->HagridProc->Init(config);
	this->RIKENIonizationChamberProc->Init(config);
	this->PidProc->Init(config);
	this->PSPMTProc->Init(config);
}

void ribf168Processor::Init(const Json::Value& config){
	this->HagridProc->Init(config);
	this->RIKENIonizationChamberProc->Init(config);
	this->PidProc->Init(config);
	this->PSPMTProc->Init(config);
}

void ribf168Processor::Init(const pugi::xml_node& config){
	this->HagridProc->Init(config);
	this->RIKENIonizationChamberProc->Init(config);
	this->PidProc->Init(config);
	this->PSPMTProc->Init(config);
}
		
void ribf168Processor::Finalize(){
	this->HagridProc->Finalize();
	this->RIKENIonizationChamberProc->Finalize();
	this->PidProc->Finalize();
	this->PSPMTProc->Finalize();
	this->console->info("{} has been finalized",this->ProcessorName);
}

void ribf168Processor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	this->HagridProc->DeclarePlots(hismanager);
	this->RIKENIonizationChamberProc->DeclarePlots(hismanager);
	this->PidProc->DeclarePlots(hismanager);
	this->PSPMTProc->DeclarePlots(hismanager);
	this->console->info("Finished Declaring Plots");
}

void ribf168Processor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
	this->HagridProc->RegisterTree(outputtrees);
	this->RIKENIonizationChamberProc->RegisterTree(outputtrees);
	this->PidProc->RegisterTree(outputtrees);
	this->PSPMTProc->RegisterTree(outputtrees);
}

void ribf168Processor::CleanupTree(){
	this->HagridProc->CleanupTree();
	this->RIKENIonizationChamberProc->CleanupTree();
	this->PidProc->CleanupTree();
	this->PSPMTProc->CleanupTree();
}
