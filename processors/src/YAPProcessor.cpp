#include "YAPProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>

YAPProcessor::YAPProcessor(const std::string& log) : Processor(log,"YAPProcessor",{"mtas","puck"}){
	this->MtasProc = std::make_unique<MtasProcessor>(log);
	this->PuckProc = std::make_unique<PuckProcessor>(log);

	for( const auto& type : this->MtasProc->GetKnownTypes() ){
		this->AssociateType(type);
	}
	for( const auto& type : this->PuckProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	this->HasPuck = false;
	this->HasMTAS = false;

	this->h1dsettings = {
		{2000,{2048,-1024.0,1023.0}},
		{3600,{16384,0.0,16384.0}},
		{3300,{16384,0.0,16384}}
	};

	this->BetaThreshold = 0.0;

	this->CurrMTAS = MtasProcessor::EventInfo();
	this->CurrPuck = PuckProcessor::EventInfo();
}

[[maybe_unused]] bool YAPProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	auto types = summary.GetKnownTypes();

	this->HasMTAS = (types.find("mtas") != types.end());
	this->HasPuck = (types.find("puck") != types.end());

	if( this->HasMTAS ){
		this->MtasProc->PreProcess(summary,hismanager,cutmanager);
	}
	this->CurrMTAS = this->MtasProc->GetCurrEvt();
	this->MTASTotal = this->CurrMTAS.TotalEnergy[0];
	this->MTASSaturate = this->CurrMTAS.Saturate;
	this->MTASPileup = this->CurrMTAS.Pileup;

	if( this->HasPuck ){
		this->PuckProc->PreProcess(summary,hismanager,cutmanager);
	}
	this->CurrPuck = this->PuckProc->GetCurrEvt();
	this->YAPFront = this->CurrPuck.Individual[0];
	this->YAPBack = this->CurrPuck.Individual[1];
	this->YAPPileup = this->CurrPuck.Pileup;
	this->YAPSaturate = this->CurrPuck.Saturate;

	if( this->HasPuck ){
		hismanager->Fill("YAP_2000",this->CurrMTAS.FirstTime - this->CurrPuck.FirstTime);
		
		if( (this->CurrPuck.TotalEnergy >= this->BetaThreshold) and (not this->CurrPuck.Saturate) and (not this->CurrPuck.Pileup) ){
			this->MtasProc->FillBetaPlots(hismanager);
		}else{
			this->MtasProc->FillNonBetaPlots(hismanager);
		}
	}else{
		this->MtasProc->FillNonBetaPlots(hismanager);
	}

	hismanager->Fill("Puck_3650",this->CurrMTAS.TotalEnergy[0],this->CurrPuck.TotalEnergy);
	hismanager->Fill("Puck_36508",this->CurrMTAS.TotalEnergy[0],this->CurrPuck.TotalEnergy);
	hismanager->Fill("Puck_3651",this->CurrMTAS.TotalEnergy[0],this->CurrPuck.Individual[0]);
	hismanager->Fill("Puck_36518",this->CurrMTAS.TotalEnergy[0],this->CurrPuck.Individual[0]);
	hismanager->Fill("Puck_3652",this->CurrMTAS.TotalEnergy[0],this->CurrPuck.Individual[1]);
	hismanager->Fill("Puck_36528",this->CurrMTAS.TotalEnergy[0],this->CurrPuck.Individual[1]);

	hismanager->Fill("Puck_3610",this->CurrPuck.TotalEnergy);
	if( (not this->CurrMTAS.Saturate) and (not this->CurrMTAS.Pileup) ){
		hismanager->Fill("Puck_3600",this->CurrPuck.TotalEnergy);
		hismanager->Fill("Puck_3602",this->CurrPuck.TotalEnergy+this->CurrMTAS.TotalEnergy[0]);
		if( (not this->HasMTAS) or this->CurrMTAS.TotalEnergy[0] < 1.0 ){
			hismanager->Fill("Puck_3601",this->CurrPuck.TotalEnergy);
		}
	}else{
		if( this->CurrMTAS.Pileup ){
			hismanager->Fill("Puck_3611",this->CurrPuck.TotalEnergy);
		}
		if( this->CurrMTAS.Saturate ){
			hismanager->Fill("Puck_3612",this->CurrPuck.TotalEnergy);
		}
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool YAPProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){

	return true;
}

[[maybe_unused]] bool YAPProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->MtasProc->PostProcess(summary,hismanager,cutmanager);
	this->PuckProc->PostProcess(summary,hismanager,cutmanager);

	return true;
}

void YAPProcessor::Init(const YAML::Node& config){
	this->MtasProc->Init(config);
	this->PuckProc->Init(config);
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void YAPProcessor::Init(const Json::Value& config){
	this->MtasProc->Init(config);
	this->PuckProc->Init(config);
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void YAPProcessor::Init(const pugi::xml_node& config){
	for( pugi::xml_node proc = config.child("Processor"); proc; proc = proc.next_sibling("Processor") ){
		std::string name = proc.attribute("name").as_string();
		if( name.compare("MtasProcessor") == 0 ){
			this->MtasProc->Init(proc);
			this->HasMTAS = true;
		}else if( name.compare("PuckProcessor") == 0 ){
			this->PuckProc->Init(proc);
			this->HasPuck = true;
		}else{
			throw std::runtime_error("unknown subprocessor declared in YAPProcessor");
		}
	}

	this->BetaThreshold = config.attribute("betathresh").as_double(0.0);

	if( not this->HasMTAS ){
		throw std::runtime_error("missing MtasProcessor in YAPProcessor");
	}

	if( not this->HasPuck ){
		throw std::runtime_error("missing PuckProcessor in YAPProcessor");
	}

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}
		
void YAPProcessor::Finalize(){
	this->MtasProc->Finalize();
	this->PuckProc->Finalize();

	this->CurrMTAS = this->MtasProc->GetCurrEvt();
	this->CurrPuck = this->PuckProc->GetCurrEvt();

	this->console->info("{} has been finalized",this->ProcessorName);
}

void YAPProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	this->MtasProc->DeclarePlots(hismanager);
	this->PuckProc->DeclarePlots(hismanager);

	hismanager->RegisterPlot<TH1F>("YAP_2000","TDiff (Puck - Mtas); TDiff (ns)",this->h1dsettings.at(2000));

	hismanager->RegisterPlot<TH1F>("YAP_3300_PILEUP","MTAS Total Puck Pileup; Energy (kev)",this->h1dsettings.at(3300));

	this->console->info("Finished Declaring Plots");
}

void YAPProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
	this->OutputTree = new TTree("Data","YAPProcessor output");
	this->OutputTree->Branch("MTASTotal",&(this->MTASTotal));
	this->OutputTree->Branch("MTASSaturate",&(this->MTASSaturate));
	this->OutputTree->Branch("MTASPileup",&(this->MTASPileup));
	this->OutputTree->Branch("YAPFront",&(this->YAPFront));
	this->OutputTree->Branch("YAPBack",&(this->YAPBack));
	this->OutputTree->Branch("YAPSaturate",&(this->YAPSaturate));
	this->OutputTree->Branch("YAPSaturate",&(this->YAPSaturate));

	outputtrees[this->ProcessorName] = this->OutputTree;
}

void YAPProcessor::CleanupTree(){
	this->MTASTotal = 0.0;
	this->YAPFront = 0.0;
	this->YAPBack = 0.0;
}
