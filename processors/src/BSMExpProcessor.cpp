#include "BSMExpProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>
#include <utility>

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

	this->CurrBSM = BSMProcessor::EventInfo();
}

[[maybe_unused]] bool BSMExpProcessor::PreProcess(EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	auto types = eventhistory->GetCurrentEventSummary()->GetKnownTypes();

	this->HasMTAS = (types.find("mtas") != types.end());
	this->HasBSM = (types.find("bsm") != types.end());

	//auto currevtcount = eventhistory->GetEventCount();
	//if( currevtcount > 1 ){
	//	if( eventhistory->GetPreviousEventSummary(1)->ContainsEventTag("muon") ){
	//		this->console->info("Previous mtas event registered as muon, current evt count : {}",currevtcount);
	//	}
	//}

	if( this->HasMTAS ){
		this->MtasProc->PreProcess(eventhistory,hismanager,cutmanager);
	}

	if( this->HasBSM ){
		this->BSMProc->PreProcess(eventhistory,hismanager,cutmanager);
	}
	this->CurrBSM = this->BSMProc->GetCurrEvt();

	bool AllWithinTDiff = (std::abs(this->CurrBSM.TDiff[0]) <= 80.0);
	bool AllWithinPos = ((this->CurrBSM.Position[0] >= this->BSMPosBounds.first) and (this->CurrBSM.Position[0] <= this->BSMPosBounds.second));
	//for( int ii = 0; ii < 6; ++ii ){
	//	if( std::abs(this->CurrBSM.TDiff[ii]) > 80.0 ){ 
	//		AllWithinTDiff = false;
	//	}
	//}

	if( this->HasBSM and AllWithinTDiff and AllWithinPos ){
		if( this->MtasProc->GetFirstFireTime() > 0.0 and this->CurrBSM.FirstTime > 0.0 ){
			hismanager->Fill("BSMEXP_2000",this->MtasProc->GetFirstFireTime() - this->CurrBSM.FirstTime);
		}
		
		if( this->PPCutExists ){
			if( this->CurrBSM.Pileup ){
				hismanager->Fill("BSMEXP_3300_PILEUP",this->MtasProc->GetTotalEnergy(0));
			}
			for( size_t ii = 0; ii < 6 ; ++ii ){
				if( cutmanager->IsWithin("PairProduction",this->MtasProc->GetTotalEnergy(0),this->MtasProc->GetSumFrontBackEnergy(ii)) ){
					if( this->MtasProc->GetFirstFireTime() > 0.0 and this->CurrBSM.FirstTime > 0.0 ){
						hismanager->Fill("BSMEXP_2000_PP",this->MtasProc->GetFirstFireTime() - this->CurrBSM.FirstTime);
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

	hismanager->Fill("BSM_3610",this->CurrBSM.TotalEnergy);
	if( (not this->MtasProc->DidAnySaturate()) and (not this->MtasProc->DidAnyPileup()) and AllWithinTDiff and AllWithinPos ){
		this->BSMProc->FillPositionPlots(hismanager);
	
		hismanager->Fill("BSM_3650",this->MtasProc->GetTotalEnergy(0),this->CurrBSM.TotalEnergy);
		hismanager->Fill("BSM_36508",this->MtasProc->GetTotalEnergy(0),this->CurrBSM.TotalEnergy);

		hismanager->Fill("BSM_3660",this->MtasProc->GetTotalEnergy(0),this->CurrBSM.TotalEnergy+this->MtasProc->GetTotalEnergy(0));
		hismanager->Fill("BSM_36608",this->MtasProc->GetTotalEnergy(0),this->CurrBSM.TotalEnergy+this->MtasProc->GetTotalEnergy(0));

		hismanager->Fill("BSM_3661",this->MtasProc->GetTotalEnergy(0)+this->CurrBSM.TotalEnergy,this->CurrBSM.TotalEnergy);
		hismanager->Fill("BSM_36618",this->MtasProc->GetTotalEnergy(0)+this->CurrBSM.TotalEnergy,this->CurrBSM.TotalEnergy);

		hismanager->Fill("BSM_3652",this->MtasProc->GetTotalEnergy(1),this->CurrBSM.TotalEnergy);
		hismanager->Fill("BSM_36528",this->MtasProc->GetTotalEnergy(1),this->CurrBSM.TotalEnergy);

		if( not (this->MtasProc->DidAnyMiddleFire() or this->MtasProc->DidAnyOuterFire()) ){
			hismanager->Fill("BSM_3654",this->MtasProc->GetTotalEnergy(0),this->CurrBSM.TotalEnergy);
			hismanager->Fill("BSM_36548",this->MtasProc->GetTotalEnergy(0),this->CurrBSM.TotalEnergy);

			hismanager->Fill("BSM_3655",this->MtasProc->GetTotalEnergy(1),this->CurrBSM.TotalEnergy);
			hismanager->Fill("BSM_36558",this->MtasProc->GetTotalEnergy(1),this->CurrBSM.TotalEnergy);

			if( not this->MtasProc->DidAnyInnerFire() ){
				hismanager->Fill("BSM_3657",this->MtasProc->GetTotalEnergy(1),this->CurrBSM.TotalEnergy);
				hismanager->Fill("BSM_36578",this->MtasProc->GetTotalEnergy(1),this->CurrBSM.TotalEnergy);
			}
		}

		for( int ii = 0; ii < 6; ++ii ){
			hismanager->Fill("BSM_3651",this->MtasProc->GetSumFrontBackEnergy(ii+6),this->CurrBSM.TotalEnergy);
			hismanager->Fill("BSM_3651",this->MtasProc->GetSumFrontBackEnergy(ii+12),this->CurrBSM.TotalEnergy);
			hismanager->Fill("BSM_3651",this->MtasProc->GetSumFrontBackEnergy(ii+18),this->CurrBSM.TotalEnergy);
			hismanager->Fill("BSM_36518",this->MtasProc->GetSumFrontBackEnergy(ii+6),this->CurrBSM.TotalEnergy);
			hismanager->Fill("BSM_36518",this->MtasProc->GetSumFrontBackEnergy(ii+12),this->CurrBSM.TotalEnergy);
			hismanager->Fill("BSM_36518",this->MtasProc->GetSumFrontBackEnergy(ii+18),this->CurrBSM.TotalEnergy);
			
			hismanager->Fill("BSM_3653",this->MtasProc->GetSumFrontBackEnergy(ii),this->CurrBSM.TotalEnergy);
			hismanager->Fill("BSM_36538",this->MtasProc->GetSumFrontBackEnergy(ii),this->CurrBSM.TotalEnergy);
			
			if( not (this->MtasProc->DidAnyMiddleFire() or this->MtasProc->DidAnyOuterFire()) ){
				hismanager->Fill("BSM_3656",this->MtasProc->GetSumFrontBackEnergy(ii),this->CurrBSM.TotalEnergy);
				hismanager->Fill("BSM_36568",this->MtasProc->GetSumFrontBackEnergy(ii),this->CurrBSM.TotalEnergy);
				
				if( not this->MtasProc->DidAnyInnerFire() ){
					hismanager->Fill("BSM_3658",this->MtasProc->GetSumFrontBackEnergy(ii),this->CurrBSM.TotalEnergy);
					hismanager->Fill("BSM_36588",this->MtasProc->GetSumFrontBackEnergy(ii),this->CurrBSM.TotalEnergy);
				}
			}
		}

		hismanager->Fill("BSM_3600",this->CurrBSM.TotalEnergy);
		hismanager->Fill("BSM_3602",this->CurrBSM.TotalEnergy+this->MtasProc->GetTotalEnergy(0));
		if( not (this->MtasProc->DidAnyMiddleFire() or this->MtasProc->DidAnyOuterFire()) ){
			hismanager->Fill("BSM_3603",this->CurrBSM.TotalEnergy+this->MtasProc->GetTotalEnergy(0));
			hismanager->Fill("BSM_3604",this->CurrBSM.TotalEnergy+this->MtasProc->GetTotalEnergy(1));
			if( not this->MtasProc->DidAnyInnerFire() ){
				hismanager->Fill("BSM_3605",this->CurrBSM.TotalEnergy+this->MtasProc->GetTotalEnergy(1));
			}
		}
		if( (not this->HasMTAS) or this->MtasProc->GetTotalEnergy(0) < 1.0 ){
			hismanager->Fill("BSM_3601",this->CurrBSM.TotalEnergy);
			if( this->CurrBSM.TotalEnergy > this->QBeta ){
				this->BSMProc->FillGSPileupTracePlots(hismanager);
			}
		}
	}else{
		if( this->MtasProc->DidAnyPileup() ){
			hismanager->Fill("BSM_3611",this->CurrBSM.TotalEnergy);
		}
		if( this->MtasProc->DidAnySaturate() ){
			hismanager->Fill("BSM_3612",this->CurrBSM.TotalEnergy);
		}
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool BSMExpProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){

	return true;
}

[[maybe_unused]] bool BSMExpProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->MtasProc->PostProcess(eventhistory,hismanager,cutmanager);
	this->BSMProc->PostProcess(eventhistory,hismanager,cutmanager);

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
	this->BSMPosBounds = std::make_pair<double,double>(config.attribute("posmin").as_double(-1.0),config.attribute("posmax").as_double(1.0));

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
