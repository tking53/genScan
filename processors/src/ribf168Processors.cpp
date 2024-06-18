#include "ribf168Processor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>

ribf168Processor::ribf168Processor(const std::string& log) : Processor(log,"ribf168Processor",{"hagrid","ionchamber","pspmt","pid","veto"}){
	this->HagridProc = std::make_unique<HagridProcessor>(log);
	this->RIKENIonizationChamberProc = std::make_unique<RIKENIonizationChamberProcessor>(log);
	this->RIKENPidProc = std::make_unique<RIKENPidProcessor>(log);
	this->PSPMTProc = std::make_unique<PSPMTProcessor>(log);
	this->VetoProc = std::make_unique<VetoProcessor>(log);

	for( const auto& type : this->HagridProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	for( const auto& type : this->RIKENIonizationChamberProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	for( const auto& type : this->RIKENPidProc->GetKnownTypes() ){
		this->AssociateType(type);
	}
	
	for( const auto& type : this->PSPMTProc->GetKnownTypes() ){
		this->AssociateType(type);
	}
	
	for( const auto& type : this->VetoProc->GetKnownTypes() ){
		this->AssociateType(type);
	}
}

[[maybe_unused]] bool ribf168Processor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	auto types = summary.GetKnownTypes();

	this->HasHagrid = (types.find("hagrid") != types.end());
	this->HasRIKENIonChamber = (types.find("ionchamber") != types.end());
	this->HasRIKENPid = (types.find("pid") != types.end());
	this->HasPSPMT = (types.find("pspmt") != types.end());
	this->HasVeto = (types.find("veto") != types.end());

	if( this->HasHagrid ){
		this->HagridProc->PreProcess(summary,hismanager,cutmanager);
	}

	if( this->HasRIKENIonChamber ){
		this->RIKENIonizationChamberProc->PreProcess(summary,hismanager,cutmanager);
		this->CurrIonChamber = this->RIKENIonizationChamberProc->GetCurrEvt();
	}

	if( this->HasRIKENPid ){
		this->RIKENPidProc->PreProcess(summary,hismanager,cutmanager);
	}

	if( this->HasPSPMT ){
		this->PSPMTProc->PreProcess(summary,hismanager,cutmanager);
		this->CurrPSPMT = this->PSPMTProc->GetCurrEvt();
	}

	if( this->HasVeto ){
		this->VetoProc->PreProcess(summary,hismanager,cutmanager);
	}

	if( this->HasPSPMT and this->HasRIKENIonChamber ){
		if( this->CurrIonChamber.MaxAnodeEnergy >= 6400 ){
			if( this->CurrPSPMT.lg.numanodes == 4 ){
				hismanager->Fill("RIBF168_1901_ABOVE",this->CurrPSPMT.lg.position.first,this->CurrPSPMT.lg.position.second);
			}
			if( this->CurrPSPMT.hg.numanodes == 4 ){
				hismanager->Fill("RIBF168_1902_ABOVE",this->CurrPSPMT.hg.position.first,this->CurrPSPMT.hg.position.second);
			}
		}else{
			if( this->CurrPSPMT.lg.numanodes == 4 ){
				hismanager->Fill("RIBF168_1901_BELOW",this->CurrPSPMT.lg.position.first,this->CurrPSPMT.lg.position.second);
			}
			if( this->CurrPSPMT.hg.numanodes == 4 ){
				hismanager->Fill("RIBF168_1902_BELOW",this->CurrPSPMT.hg.position.first,this->CurrPSPMT.hg.position.second);
			}
		}
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

	if( this->HasRIKENPid ){
		this->RIKENPidProc->Process(summary,hismanager,cutmanager);
	}

	if( this->HasPSPMT ){
		this->PSPMTProc->Process(summary,hismanager,cutmanager);
	}

	if( this->HasVeto ){
		this->VetoProc->Process(summary,hismanager,cutmanager);
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

	if( this->HasRIKENPid ){
		this->RIKENPidProc->PostProcess(summary,hismanager,cutmanager);
	}

	if( this->HasPSPMT ){
		this->PSPMTProc->PostProcess(summary,hismanager,cutmanager);
	}

	if( this->HasVeto ){
		this->VetoProc->PostProcess(summary,hismanager,cutmanager);
	}

	return true;
}

void ribf168Processor::Init(const YAML::Node& config){
	this->HagridProc->Init(config);
	this->RIKENIonizationChamberProc->Init(config);
	this->RIKENPidProc->Init(config);
	this->PSPMTProc->Init(config);
	this->VetoProc->Init(config);
}

void ribf168Processor::Init(const Json::Value& config){
	this->HagridProc->Init(config);
	this->RIKENIonizationChamberProc->Init(config);
	this->RIKENPidProc->Init(config);
	this->PSPMTProc->Init(config);
	this->VetoProc->Init(config);
}

void ribf168Processor::Init(const pugi::xml_node& config){
	this->HagridProc->Init(config);
	this->RIKENIonizationChamberProc->Init(config);
	this->RIKENPidProc->Init(config);
	this->PSPMTProc->Init(config);
	this->VetoProc->Init(config);
}
		
void ribf168Processor::Finalize(){
	this->HagridProc->Finalize();
	this->RIKENIonizationChamberProc->Finalize();
	this->RIKENPidProc->Finalize();
	this->PSPMTProc->Finalize();
	this->VetoProc->Finalize();
	this->console->info("{} has been finalized",this->ProcessorName);
}

void ribf168Processor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	this->HagridProc->DeclarePlots(hismanager);
	this->RIKENIonizationChamberProc->DeclarePlots(hismanager);
	this->RIKENPidProc->DeclarePlots(hismanager);
	this->PSPMTProc->DeclarePlots(hismanager);
	this->VetoProc->DeclarePlots(hismanager);

	hismanager->RegisterPlot<TH2F>("RIBF168_1901_ABOVE","Low Gain Image Max IonChamber Gated; Position (arb.); Position (arb.)",1024,-1.0,1.0,1024,-1.0,1.0);
	hismanager->RegisterPlot<TH2F>("RIBF168_1902_ABOVE","High Gain Image Max IonChamber Gated; Position (arb.); Position (arb.)",1024,-1.0,1.0,1024,-1.0,1.0);
	hismanager->RegisterPlot<TH2F>("RIBF168_1901_BELOW","Low Gain Image Max IonChamber Gated; Position (arb.); Position (arb.)",1024,-1.0,1.0,1024,-1.0,1.0);
	hismanager->RegisterPlot<TH2F>("RIBF168_1902_BELOW","High Gain Image Max IonChamber Gated; Position (arb.); Position (arb.)",1024,-1.0,1.0,1024,-1.0,1.0);

	this->console->info("Finished Declaring Plots");
}

void ribf168Processor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
	this->HagridProc->RegisterTree(outputtrees);
	this->RIKENIonizationChamberProc->RegisterTree(outputtrees);
	this->RIKENPidProc->RegisterTree(outputtrees);
	this->PSPMTProc->RegisterTree(outputtrees);
	this->VetoProc->RegisterTree(outputtrees);
}

void ribf168Processor::CleanupTree(){
	this->HagridProc->CleanupTree();
	this->RIKENIonizationChamberProc->CleanupTree();
	this->RIKENPidProc->CleanupTree();
	this->PSPMTProc->CleanupTree();
	this->VetoProc->CleanupTree();
}
