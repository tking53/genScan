#include "KClComptonProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <limits>
#include <stdexcept>
#include <utility>

KClComptonProcessor::KClComptonProcessor(const std::string& log) : Processor(log,"KClComptonProcessor",{"hpge","bsm"}){
	this->HPGeProc = std::make_unique<SimpleHPGeProcessor>(log);
	this->BSMProc = std::make_unique<BSMProcessor>(log);

	for( const auto& type : this->HPGeProc->GetKnownTypes() ){
		this->AssociateType(type);
	}
	for( const auto& type : this->BSMProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	this->HasBSM = false;
	this->HasHPGe = false;

	this->h1dsettings = {
		{2000,{2048,-1024.0,1023.0}},
		{3600,{16384,0.0,16384.0}},
		{3600 , {16384,0.0,16384}},
		{3602 , {16384,0.0,16384}},
		{3610 , {16384,0.0,16384}}
	};

	this->h2dsettings = {
		{3650 , {4096,0.0,4096.0,4096,0.0,4096.0}},
		{36508 , {2048,0.0,16384.0,2048,0.0,16384.0}},
		{3660 , {4096,0.0,4096.0,4096,0.0,4096.0}},
		{36608 , {2048,0.0,16384.0,2048,0.0,16384.0}},
		{3661 , {4096,0.0,4096.0,4096,0.0,4096.0}},
		{36618 , {2048,0.0,16384.0,2048,0.0,16384.0}}
	};
}

[[maybe_unused]] bool KClComptonProcessor::PreProcess(EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	auto types = eventhistory->GetCurrentEventSummary()->GetKnownTypes();

	this->HasHPGe = (types.find("hpge") != types.end());
	this->HasBSM = (types.find("bsm") != types.end());

	if( this->HasHPGe ){
		this->HPGeProc->PreProcess(eventhistory,hismanager,cutmanager);
	}

	if( this->HasBSM ){
		this->BSMProc->PreProcess(eventhistory,hismanager,cutmanager);
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool KClComptonProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::Process();

	//auto BSMErg = this->BSMProc->GetAverageTotalEnergy();
	auto BSMErg = this->BSMProc->GetGeometricTotalEnergy();
	auto HPGeErg = this->HPGeProc->GetEnergy(0);
	auto HPGeTS = this->HPGeProc->GetCrystalFireTime(0);
	auto BSMTS =  this->BSMProc->GetFirstFireTime();
	auto TDiff = (BSMTS > 0.0 and HPGeTS > 0.0) ? (HPGeTS - BSMTS) : std::numeric_limits<double>::max();
	if( HPGeTS < 0.0 ){
		TDiff *= -1;
	}

	hismanager->Fill("COMPTON_2000",TDiff);
	hismanager->Fill("BSM_3610",BSMErg);
	if( (not this->HPGeProc->DidCrystalSaturate(0)) and (not this->HPGeProc->DidCrystalPileup(0)) ){
		this->BSMProc->FillPositionPlots(hismanager);
	
		hismanager->Fill("BSM_3650",HPGeErg,BSMErg);
		hismanager->Fill("BSM_36508",HPGeErg,BSMErg);

		hismanager->Fill("BSM_3660",HPGeErg,BSMErg+HPGeErg);
		hismanager->Fill("BSM_36608",HPGeErg,BSMErg+HPGeErg);

		hismanager->Fill("BSM_3661",HPGeErg+BSMErg,BSMErg);
		hismanager->Fill("BSM_36618",HPGeErg+BSMErg,BSMErg);

		hismanager->Fill("BSM_3600",BSMErg);
		hismanager->Fill("BSM_3602",BSMErg+HPGeErg);
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool KClComptonProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->HPGeProc->PostProcess(eventhistory,hismanager,cutmanager);
	this->BSMProc->PostProcess(eventhistory,hismanager,cutmanager);

	return true;
}

void KClComptonProcessor::Init(const pugi::xml_node& config){
	for( pugi::xml_node proc = config.child("Processor"); proc; proc = proc.next_sibling("Processor") ){
		std::string name = proc.attribute("name").as_string();
		if( name.compare("SimpleHPGeProcessor") == 0 ){
			this->HPGeProc->Init(proc);
			this->HasHPGe = true;
		}else if( name.compare("BSMProcessor") == 0 ){
			this->BSMProc->Init(proc);
			this->HasBSM = true;
		}else{
			throw std::runtime_error("unknown subprocessor declared in KClComptonProcessor");
		}
	}

	if( not this->HasHPGe ){
		throw std::runtime_error("missing SimpleHPGeProcessor in KClComptonProcessor");
	}

	if( not this->HasBSM ){
		throw std::runtime_error("missing BSMProcessor in KClComptonProcessor");
	}

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}
		
void KClComptonProcessor::Finalize(){
	this->HPGeProc->Finalize();
	this->BSMProc->Finalize();

	this->console->info("{} has been finalized",this->ProcessorName);
}

void KClComptonProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager){
	this->HPGeProc->DeclarePlots(hismanager);
	this->BSMProc->DeclarePlots(hismanager);

	hismanager->RegisterPlot<TH1F>("COMPTON_2000","TDiff (HPGe - #betaSM); TDiff (ns)",this->h1dsettings.at(2000));

	hismanager->RegisterPlot<TH1F>("BSM_3600","#betaSM Total; Energy (keV)",this->h1dsettings.at(3600));
	hismanager->RegisterPlot<TH1F>("BSM_3602","#betaSM Total + HPGe; Energy (keV)",this->h1dsettings.at(3602));
	hismanager->RegisterPlot<TH1F>("BSM_3610","#betaSM Total; Energy (keV)",this->h1dsettings.at(3610));
	
	hismanager->RegisterPlot<TH2F>("BSM_3650","#betaSM Total vs HPGe; HPGe Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3650));
	hismanager->RegisterPlot<TH2F>("BSM_36508","#betaSM Total vs HPGe; HPGe Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36508));

	hismanager->RegisterPlot<TH2F>("BSM_3660","#betaSM Total + HPGe vs HPGe; HPGe Energy (keV); #betaSM Energy + HPGe Energy (keV)",this->h2dsettings.at(3660));
	hismanager->RegisterPlot<TH2F>("BSM_36608","#betaSM Total + HPGe vs HPGe; HPGe Energy (8 keV/bin); #betaSM Energy + HPGe Energy (8 keV/bin)",this->h2dsettings.at(36608));

	hismanager->RegisterPlot<TH2F>("BSM_3661","#betaSM Total vs #betaSM Total + HPGe; #betaSM Energy + HPGe Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3661));
	hismanager->RegisterPlot<TH2F>("BSM_36618","#betaSM Total vs #betaSM Total + HPGe; #betaSM Energy + HPGe Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36618));

	this->console->info("Finished Declaring Plots");
}

void KClComptonProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
	this->HPGeProc->RegisterTree(outputtrees);
	this->BSMProc->RegisterTree(outputtrees);
}

void KClComptonProcessor::CleanupTree(){
	this->HPGeProc->CleanupTree();
	this->BSMProc->CleanupTree();
}
