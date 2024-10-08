#include "BSMExpProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>

BSMExpProcessor::BSMExpProcessor(const std::string& log) : Processor(log,"BSMExpProcessor",{"mtas","bsm"}){
	this->MtasProc = std::make_unique<MtasProcessor>(log);
	this->BSMProc = std::make_unique<BSMProcessor>(log);

	for( const auto& type : this->MtasProc->GetKnownTypes() ){
		this->AssociateType(type);
	}
	for( const auto& type : this->BSMProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	this->HasBSM = false;
	this->HasMTAS = false;

	this->h1dsettings = {
		{2000,{2048,-1024.0,1023.0}},
		{3600,{16384,0.0,16384.0}},
		{3300,{16384,0.0,16384}}
	};

	this->BetaThreshold = 0.0;
	this->QBeta = 8192.0;
	this->PPCutExists = false;

	this->CurrMTAS = MtasProcessor::EventInfo();
	this->CurrBSM = BSMProcessor::EventInfo();
}

[[maybe_unused]] bool BSMExpProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	auto types = summary.GetKnownTypes();

	this->HasMTAS = (types.find("mtas") != types.end());
	this->HasBSM = (types.find("bsm") != types.end());

	if( this->HasMTAS ){
		this->MtasProc->PreProcess(summary,hismanager,cutmanager);
	}
	this->CurrMTAS = this->MtasProc->GetCurrEvt();

	if( this->HasBSM ){
		this->BSMProc->PreProcess(summary,hismanager,cutmanager);
	}
	this->CurrBSM = this->BSMProc->GetCurrEvt();

	if( this->HasBSM ){
		if( this->CurrMTAS.FirstTime > 0.0 and this->CurrBSM.FirstTime > 0.0 ){
			hismanager->Fill("BSMEXP_2000",this->CurrMTAS.FirstTime - this->CurrBSM.FirstTime);
		}
		
		if( this->PPCutExists ){
			if( this->CurrBSM.Pileup ){
				hismanager->Fill("BSMEXP_3300_PILEUP",this->CurrMTAS.TotalEnergy[0]);
			}
			for( size_t ii = 0; ii < 6 ; ++ii ){
				if( cutmanager->IsWithin("PairProduction",this->CurrMTAS.TotalEnergy[0],this->CurrMTAS.SumFrontBackEnergy[ii]) ){
					if( this->CurrMTAS.FirstTime > 0.0 and this->CurrBSM.FirstTime > 0.0 ){
						hismanager->Fill("BSMEXP_2000_PP",this->CurrMTAS.FirstTime - this->CurrBSM.FirstTime);
						hismanager->Fill("BSM_3600_PP",this->CurrBSM.TotalEnergy);
						break;
					}
				}
			}
		}

		if( (this->CurrBSM.TotalEnergy >= this->BetaThreshold) and (not this->CurrBSM.Saturate) and (not this->CurrBSM.Pileup) ){
			this->MtasProc->FillBetaPlots(hismanager);
		}else{
			this->MtasProc->FillNonBetaPlots(hismanager);
		}
	}else{
		this->MtasProc->FillNonBetaPlots(hismanager);
	}

	hismanager->Fill("BSM_3650",this->CurrMTAS.TotalEnergy[0],this->CurrBSM.TotalEnergy);
	hismanager->Fill("BSM_36508",this->CurrMTAS.TotalEnergy[0],this->CurrBSM.TotalEnergy);

	hismanager->Fill("BSM_3610",this->CurrBSM.TotalEnergy);
	if( (not this->CurrMTAS.Saturate) and (not this->CurrMTAS.Pileup) ){
		hismanager->Fill("BSM_3600",this->CurrBSM.TotalEnergy);
		hismanager->Fill("BSM_3602",this->CurrBSM.TotalEnergy+this->CurrMTAS.TotalEnergy[0]);
		if( (not this->HasMTAS) or this->CurrMTAS.TotalEnergy[0] < 1.0 ){
			hismanager->Fill("BSM_3601",this->CurrBSM.TotalEnergy);
			if( this->CurrBSM.TotalEnergy > this->QBeta ){
				this->BSMProc->FillGSPileupTracePlots(hismanager);
			}
		}
	}else{
		if( this->CurrMTAS.Pileup ){
			hismanager->Fill("BSM_3611",this->CurrBSM.TotalEnergy);
		}
		if( this->CurrMTAS.Saturate ){
			hismanager->Fill("BSM_3612",this->CurrBSM.TotalEnergy);
		}
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool BSMExpProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){

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
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void BSMExpProcessor::Init(const Json::Value& config){
	this->MtasProc->Init(config);
	this->BSMProc->Init(config);
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void BSMExpProcessor::Init(const pugi::xml_node& config){
	for( pugi::xml_node proc = config.child("Processor"); proc; proc = proc.next_sibling("Processor") ){
		std::string name = proc.attribute("name").as_string();
		if( name.compare("MtasProcessor") == 0 ){
			this->MtasProc->Init(proc);
			this->HasMTAS = true;
		}else if( name.compare("BSMProcessor") == 0 ){
			this->BSMProc->Init(proc);
			this->HasBSM = true;
		}else{
			throw std::runtime_error("unknown subprocessor declared in BSMExpProcessor");
		}
	}

	this->BetaThreshold = config.attribute("betathresh").as_double(0.0);
	this->QBeta = config.attribute("qvalue").as_double(8192.0);

	if( not this->HasMTAS ){
		throw std::runtime_error("missing MtasProcessor in BSMExpProcessor");
	}

	if( not this->HasBSM ){
		throw std::runtime_error("missing BSMProcessor in BSMExpProcessor");
	}

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
	
	this->PPCutExists = (this->customcuts.find("PairProduction") != this->customcuts.end());
}
		
void BSMExpProcessor::Finalize(){
	this->MtasProc->Finalize();
	this->BSMProc->Finalize();

	this->CurrMTAS = this->MtasProc->GetCurrEvt();
	this->CurrBSM = this->BSMProc->GetCurrEvt();

	this->console->info("{} has been finalized",this->ProcessorName);
}

void BSMExpProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	this->MtasProc->DeclarePlots(hismanager);
	this->BSMProc->DeclarePlots(hismanager);

	hismanager->RegisterPlot<TH1F>("BSMEXP_2000","TDiff (#betaSM - Mtas); TDiff (ns)",this->h1dsettings.at(2000));

	if( this->PPCutExists ){
		hismanager->RegisterPlot<TH1F>("BSM_3600_PP","#betaSM Energy [PairProduction]; Energy (keV)",this->h1dsettings.at(3600));
		hismanager->RegisterPlot<TH1F>("BSMEXP_2000_PP","TDiff (#betaSM - Mtas) [PairProduction]; TDiff (ns)",this->h1dsettings.at(2000));
	}

	hismanager->RegisterPlot<TH1F>("BSMEXP_3300_PILEUP","MTAS Total #betaSM Pileup; Energy (kev)",this->h1dsettings.at(3300));

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
