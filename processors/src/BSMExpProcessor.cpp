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
		{3300,{16384,0.0,16384}},
		{3600 , {16384,0.0,16384}},
		{3601 , {16384,0.0,16384}},
		{3602 , {16384,0.0,16384}},
		{3603 , {16384,0.0,16384}},
		{3604 , {16384,0.0,16384}},
		{3605 , {16384,0.0,16384}},
		{3610 , {16384,0.0,16384}},
		{3611 , {16384,0.0,16384}},
		{3612 , {16384,0.0,16384}}
	};

	this->h2dsettings = {
		{3650 , {4096,0.0,4096.0,4096,0.0,4096.0}},
		{36508 , {2048,0.0,16384.0,2048,0.0,16384.0}},
		{3651 , {4096,0.0,4096.0,4096,0.0,4096.0}},
		{36518 , {2048,0.0,16384.0,2048,0.0,16384.0}},
		{3652 , {4096,0.0,4096.0,4096,0.0,4096.0}},
		{36528 , {2048,0.0,16384.0,2048,0.0,16384.0}},
		{3653 , {4096,0.0,4096.0,4096,0.0,4096.0}},
		{36538 , {2048,0.0,16384.0,2048,0.0,16384.0}},
		{3654 , {4096,0.0,4096.0,4096,0.0,4096.0}},
		{36548 , {2048,0.0,16384.0,2048,0.0,16384.0}},
		{3655 , {4096,0.0,4096.0,4096,0.0,4096.0}},
		{36558 , {2048,0.0,16384.0,2048,0.0,16384.0}},
		{3656 , {4096,0.0,4096.0,4096,0.0,4096.0}},
		{36568 , {2048,0.0,16384.0,2048,0.0,16384.0}},
		{3657 , {4096,0.0,4096.0,4096,0.0,4096.0}},
		{36578 , {2048,0.0,16384.0,2048,0.0,16384.0}},
		{3658 , {4096,0.0,4096.0,4096,0.0,4096.0}},
		{36588 , {2048,0.0,16384.0,2048,0.0,16384.0}},
		{3660 , {4096,0.0,4096.0,4096,0.0,4096.0}},
		{36608 , {2048,0.0,16384.0,2048,0.0,16384.0}},
		{3661 , {4096,0.0,4096.0,4096,0.0,4096.0}},
		{36618 , {2048,0.0,16384.0,2048,0.0,16384.0}}
	};

	this->BetaThreshold = 0.0;
	this->QBeta = 8192.0;
	this->PPCutExists = false;
	this->PPBkgExists = {
		{"A",false},
		{"B",false},
		{"C",false},
		{"D",false},
		{"E",false},
		{"F",false},
		{"G",false},
		{"H",false},
		{"Compton",false},
		{"Single",false},
		{"Double",false}
	};
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

	bool AllWithinTDiff = (std::abs(this->BSMProc->GetTDiff(0)) <= 80.0);
	bool AllWithinPos = ((this->BSMProc->GetPosition(0) >= this->BSMPosBounds.first) and (this->BSMProc->GetPosition(0) <= this->BSMPosBounds.second));

	//auto BSMErg = this->BSMProc->GetAverageTotalEnergy();
	auto BSMErg = this->BSMProc->GetGeometricTotalEnergy();
	auto MTASErg = this->MtasProc->GetTotalEnergy(0);
	auto TDiff = this->MtasProc->GetFirstFireTime() - this->BSMProc->GetFirstFireTime();

	if( this->HasBSM and AllWithinTDiff and AllWithinPos ){
		if( this->MtasProc->GetFirstFireTime() > 0.0 and this->BSMProc->GetFirstFireTime() > 0.0 ){
			hismanager->Fill("BSMEXP_2000",TDiff);
		}
		
		if( this->PPCutExists ){
			if( this->BSMProc->DidAnyPileup() ){
				hismanager->Fill("BSMEXP_3300_PILEUP",MTASErg);
			}
			for( size_t ii = 0; ii < 6 ; ++ii ){
				//if( cutmanager->IsWithin("PairProduction",MTASErg,this->MtasProc->GetCrystalEnergy(ii)) ){
				//above is old version, below is better segmented 3353, which should then allow us to look at 3300 for what states we 
				//populate, this won't necessarily catch everything but should cleanly catch all the pure e+/e- 
				//to catch all we would need to make this gate such that we look for back to back with 511's and allow anything 
				//in the other 4 crystals, but this won't catch if the two that we allow share some portion of the energy
				if( cutmanager->IsWithin("PairProduction",this->MtasProc->GetTotalEnergy(1),this->MtasProc->GetCrystalEnergy(ii)) ){
					if( this->MtasProc->GetFirstFireTime() > 0.0 and this->BSMProc->GetFirstFireTime() > 0.0 ){
						hismanager->Fill("BSMEXP_3300_PP",MTASErg);
						hismanager->Fill("BSMEXP_2000_PP",TDiff);
						hismanager->Fill("BSMEXP_3600_PP",BSMErg);
						break;
					}
				}
			}
		}
		for( const auto& kv : this->PPBkgExists ){
			if( kv.second ){
				for( size_t ii = 0; ii < 6; ++ii ){
					if( cutmanager->IsWithin("PPBkg"+kv.first,this->MtasProc->GetTotalEnergy(1),this->MtasProc->GetCrystalEnergy(ii)) ){
						if( this->MtasProc->GetFirstFireTime() > 0.0 and this->BSMProc->GetFirstFireTime() > 0.0 ){
							hismanager->Fill("BSMEXP_3300_PPBkg"+kv.first,MTASErg);
							hismanager->Fill("BSMEXP_2000_PPBkg"+kv.first,TDiff);
							hismanager->Fill("BSMEXP_3600_PPBkg"+kv.first,BSMErg);
							break;
						}
					}
				}
			}
		}

		if( (BSMErg >= this->BetaThreshold) and (not this->BSMProc->DidAnySaturate()) and (not this->BSMProc->DidAnyPileup()) ){
			this->MtasProc->FillBetaPlots(hismanager);
			this->MtasProc->FillNoLogicBetaPlots(hismanager);
		}else{
			this->MtasProc->FillNonBetaPlots(hismanager);
			this->MtasProc->FillNoLogicNonBetaPlots(hismanager);
		}
	}else{
		this->MtasProc->FillNonBetaPlots(hismanager);
	}

	auto bsmf = BSMProc->GetIndividualPMTEnergy(0);
	auto bsmb = BSMProc->GetIndividualPMTEnergy(1);
	hismanager->Fill("BSM_3610",BSMErg);
	hismanager->Fill("BSM_3610_F",bsmf);
	hismanager->Fill("BSM_3610_B",bsmb);
	if( (not this->MtasProc->DidAnySaturate()) and (not this->MtasProc->DidAnyPileup()) and AllWithinTDiff and AllWithinPos ){
		this->BSMProc->FillPositionPlots(hismanager);
	
		hismanager->Fill("BSM_3650",MTASErg,BSMErg);
		hismanager->Fill("BSM_3650_F",MTASErg,bsmf);
		hismanager->Fill("BSM_3650_B",MTASErg,bsmb);
		hismanager->Fill("BSM_36508",MTASErg,BSMErg);
		hismanager->Fill("BSM_36508_F",MTASErg,bsmf);
		hismanager->Fill("BSM_36508_B",MTASErg,bsmb);

		hismanager->Fill("BSM_3660",MTASErg,BSMErg+MTASErg);
		hismanager->Fill("BSM_3660_F",MTASErg,bsmf+MTASErg);
		hismanager->Fill("BSM_3660_B",MTASErg,bsmb+MTASErg);
		hismanager->Fill("BSM_36608",MTASErg,BSMErg+MTASErg);
		hismanager->Fill("BSM_36608_F",MTASErg,bsmf+MTASErg);
		hismanager->Fill("BSM_36608_B",MTASErg,bsmb+MTASErg);

		hismanager->Fill("BSM_3661",MTASErg+BSMErg,BSMErg);
		hismanager->Fill("BSM_3661_F",MTASErg+bsmf,bsmf);
		hismanager->Fill("BSM_3661_B",MTASErg+bsmb,bsmb);
		hismanager->Fill("BSM_36618",MTASErg+BSMErg,BSMErg);
		hismanager->Fill("BSM_36618_F",MTASErg+bsmf,bsmf);
		hismanager->Fill("BSM_36618_B",MTASErg+bsmb,bsmb);

		hismanager->Fill("BSM_3652",this->MtasProc->GetTotalEnergy(1),BSMErg);
		hismanager->Fill("BSM_3652_F",this->MtasProc->GetTotalEnergy(1),bsmf);
		hismanager->Fill("BSM_3652_B",this->MtasProc->GetTotalEnergy(1),bsmb);
		hismanager->Fill("BSM_36528",this->MtasProc->GetTotalEnergy(1),BSMErg);
		hismanager->Fill("BSM_36528_F",this->MtasProc->GetTotalEnergy(1),bsmf);
		hismanager->Fill("BSM_36528_B",this->MtasProc->GetTotalEnergy(1),bsmb);

		if( not (this->MtasProc->DidAnyMiddleFire() or this->MtasProc->DidAnyOuterFire()) ){
			hismanager->Fill("BSM_3654",MTASErg,BSMErg);
			hismanager->Fill("BSM_3654_F",MTASErg,bsmf);
			hismanager->Fill("BSM_3654_B",MTASErg,bsmb);
			hismanager->Fill("BSM_36548",MTASErg,BSMErg);
			hismanager->Fill("BSM_36548_F",MTASErg,bsmf);
			hismanager->Fill("BSM_36548_B",MTASErg,bsmb);

			hismanager->Fill("BSM_3655",this->MtasProc->GetTotalEnergy(1),BSMErg);
			hismanager->Fill("BSM_3655_F",this->MtasProc->GetTotalEnergy(1),bsmf);
			hismanager->Fill("BSM_3655_B",this->MtasProc->GetTotalEnergy(1),bsmb);
			hismanager->Fill("BSM_36558",this->MtasProc->GetTotalEnergy(1),BSMErg);
			hismanager->Fill("BSM_36558_F",this->MtasProc->GetTotalEnergy(1),bsmf);
			hismanager->Fill("BSM_36558_B",this->MtasProc->GetTotalEnergy(1),bsmb);

			if( not this->MtasProc->DidAnyInnerFire() ){
				hismanager->Fill("BSM_3657",this->MtasProc->GetTotalEnergy(1),BSMErg);
				hismanager->Fill("BSM_3657_F",this->MtasProc->GetTotalEnergy(1),bsmf);
				hismanager->Fill("BSM_3657_B",this->MtasProc->GetTotalEnergy(1),bsmb);
				hismanager->Fill("BSM_36578",this->MtasProc->GetTotalEnergy(1),BSMErg);
				hismanager->Fill("BSM_36578_F",this->MtasProc->GetTotalEnergy(1),bsmf);
				hismanager->Fill("BSM_36578_B",this->MtasProc->GetTotalEnergy(1),bsmb);
			}
		}

		for( int ii = 0; ii < 6; ++ii ){
			hismanager->Fill("BSM_3651",this->MtasProc->GetCrystalEnergy(ii+6),BSMErg);
			hismanager->Fill("BSM_3651_F",this->MtasProc->GetCrystalEnergy(ii+6),bsmf);
			hismanager->Fill("BSM_3651_B",this->MtasProc->GetCrystalEnergy(ii+6),bsmb);
			hismanager->Fill("BSM_3651",this->MtasProc->GetCrystalEnergy(ii+12),BSMErg);
			hismanager->Fill("BSM_3651_F",this->MtasProc->GetCrystalEnergy(ii+12),bsmf);
			hismanager->Fill("BSM_3651_B",this->MtasProc->GetCrystalEnergy(ii+12),bsmb);
			hismanager->Fill("BSM_3651",this->MtasProc->GetCrystalEnergy(ii+18),BSMErg);
			hismanager->Fill("BSM_3651_F",this->MtasProc->GetCrystalEnergy(ii+18),bsmf);
			hismanager->Fill("BSM_3651_B",this->MtasProc->GetCrystalEnergy(ii+18),bsmb);
			hismanager->Fill("BSM_36518",this->MtasProc->GetCrystalEnergy(ii+6),BSMErg);
			hismanager->Fill("BSM_36518_F",this->MtasProc->GetCrystalEnergy(ii+6),bsmf);
			hismanager->Fill("BSM_36518_B",this->MtasProc->GetCrystalEnergy(ii+6),bsmb);
			hismanager->Fill("BSM_36518",this->MtasProc->GetCrystalEnergy(ii+12),BSMErg);
			hismanager->Fill("BSM_36518_F",this->MtasProc->GetCrystalEnergy(ii+12),bsmf);
			hismanager->Fill("BSM_36518_B",this->MtasProc->GetCrystalEnergy(ii+12),bsmb);
			hismanager->Fill("BSM_36518",this->MtasProc->GetCrystalEnergy(ii+18),BSMErg);
			hismanager->Fill("BSM_36518_F",this->MtasProc->GetCrystalEnergy(ii+18),bsmf);
			hismanager->Fill("BSM_36518_B",this->MtasProc->GetCrystalEnergy(ii+18),bsmb);
			
			hismanager->Fill("BSM_3653",this->MtasProc->GetCrystalEnergy(ii),BSMErg);
			hismanager->Fill("BSM_3653_F",this->MtasProc->GetCrystalEnergy(ii),bsmf);
			hismanager->Fill("BSM_3653_B",this->MtasProc->GetCrystalEnergy(ii),bsmb);
			hismanager->Fill("BSM_36538",this->MtasProc->GetCrystalEnergy(ii),BSMErg);
			hismanager->Fill("BSM_36538_F",this->MtasProc->GetCrystalEnergy(ii),bsmf);
			hismanager->Fill("BSM_36538_B",this->MtasProc->GetCrystalEnergy(ii),bsmb);
			
			if( not (this->MtasProc->DidAnyMiddleFire() or this->MtasProc->DidAnyOuterFire()) ){
				hismanager->Fill("BSM_3656",this->MtasProc->GetCrystalEnergy(ii),BSMErg);
				hismanager->Fill("BSM_3656_F",this->MtasProc->GetCrystalEnergy(ii),bsmf);
				hismanager->Fill("BSM_3656_B",this->MtasProc->GetCrystalEnergy(ii),bsmb);
				hismanager->Fill("BSM_36568",this->MtasProc->GetCrystalEnergy(ii),BSMErg);
				hismanager->Fill("BSM_36568_F",this->MtasProc->GetCrystalEnergy(ii),bsmf);
				hismanager->Fill("BSM_36568_B",this->MtasProc->GetCrystalEnergy(ii),bsmb);
				
				if( not this->MtasProc->DidAnyInnerFire() ){
					hismanager->Fill("BSM_3658",this->MtasProc->GetCrystalEnergy(ii),BSMErg);
					hismanager->Fill("BSM_3658_F",this->MtasProc->GetCrystalEnergy(ii),bsmf);
					hismanager->Fill("BSM_3658_B",this->MtasProc->GetCrystalEnergy(ii),bsmb);
					hismanager->Fill("BSM_36588",this->MtasProc->GetCrystalEnergy(ii),BSMErg);
					hismanager->Fill("BSM_36588_F",this->MtasProc->GetCrystalEnergy(ii),bsmf);
					hismanager->Fill("BSM_36588_B",this->MtasProc->GetCrystalEnergy(ii),bsmb);
				}
			}
		}

		hismanager->Fill("BSM_3600",BSMErg);
		hismanager->Fill("BSM_3600_F",bsmf);
		hismanager->Fill("BSM_3600_B",bsmb);
		hismanager->Fill("BSM_3602",BSMErg+MTASErg);
		hismanager->Fill("BSM_3602_F",bsmf+MTASErg);
		hismanager->Fill("BSM_3602_B",bsmb+MTASErg);
		if( not (this->MtasProc->DidAnyMiddleFire() or this->MtasProc->DidAnyOuterFire()) ){
			hismanager->Fill("BSM_3603",BSMErg+MTASErg);
			hismanager->Fill("BSM_3603_F",bsmf+MTASErg);
			hismanager->Fill("BSM_3603_B",bsmb+MTASErg);
			hismanager->Fill("BSM_3604",BSMErg+this->MtasProc->GetTotalEnergy(1));
			hismanager->Fill("BSM_3604_F",bsmf+this->MtasProc->GetTotalEnergy(1));
			hismanager->Fill("BSM_3604_B",bsmb+this->MtasProc->GetTotalEnergy(1));
			if( not this->MtasProc->DidAnyInnerFire() ){
				hismanager->Fill("BSM_3605",BSMErg+this->MtasProc->GetTotalEnergy(1));
				hismanager->Fill("BSM_3605_F",bsmf+this->MtasProc->GetTotalEnergy(1));
				hismanager->Fill("BSM_3605_B",bsmb+this->MtasProc->GetTotalEnergy(1));
			}
		}
		if( (not this->HasMTAS) or MTASErg < 1.0 ){
			hismanager->Fill("BSM_3601",BSMErg);
			hismanager->Fill("BSM_3601_F",bsmf);
			hismanager->Fill("BSM_3601_B",bsmb);
			if( BSMErg > this->QBeta ){
				this->BSMProc->FillGSPileupTracePlots(hismanager);
			}
		}
	}else{
		if( this->MtasProc->DidAnyPileup() ){
			hismanager->Fill("BSM_3611",BSMErg);
			hismanager->Fill("BSM_3611_F",bsmf);
			hismanager->Fill("BSM_3611_B",bsmb);
		}
		if( this->MtasProc->DidAnySaturate() ){
			hismanager->Fill("BSM_3612",BSMErg);
			hismanager->Fill("BSM_3612_F",bsmf);
			hismanager->Fill("BSM_3612_B",bsmb);
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
	for( auto& kv : this->PPBkgExists ){
		kv.second = (this->customcuts.find("PPBkg"+kv.first) != this->customcuts.end());
	}
}
		
void BSMExpProcessor::Finalize(){
	this->MtasProc->Finalize();
	this->BSMProc->Finalize();

	this->console->info("{} has been finalized",this->ProcessorName);
}

void BSMExpProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager){
	this->MtasProc->DeclarePlots(hismanager);
	this->BSMProc->DeclarePlots(hismanager);

	hismanager->RegisterPlot<TH1F>("BSMEXP_2000","TDiff (#betaSM - Mtas); TDiff (ns)",this->h1dsettings.at(2000));

	if( this->PPCutExists ){
		hismanager->RegisterPlot<TH1F>("BSMEXP_3600_PP","#betaSM Energy [PairProduction]; Energy (keV)",this->h1dsettings.at(3600));
		hismanager->RegisterPlot<TH1F>("BSMEXP_2000_PP","TDiff (#betaSM - Mtas) [PairProduction]; TDiff (ns)",this->h1dsettings.at(2000));
		hismanager->RegisterPlot<TH1F>("BSMEXP_3300_PP","MTAS Total [PairProduction]; Energy (keV)",this->h1dsettings.at(3300));
	}

	for( const auto& kv : this->PPBkgExists ){
		if( kv.second ){
			hismanager->RegisterPlot<TH1F>("BSMEXP_3600_PPBkg"+kv.first,"#betaSM Energy [PPBkg"+kv.first+"]; Energy (keV)",this->h1dsettings.at(3600));
			hismanager->RegisterPlot<TH1F>("BSMEXP_2000_PPBkg"+kv.first,"TDiff (#betaSM - Mtas) [PPBkg"+kv.first+"]; TDiff (ns)",this->h1dsettings.at(2000));
			hismanager->RegisterPlot<TH1F>("BSMEXP_3300_PPBkg"+kv.first,"MTAS Total [PPBkg"+kv.first+"]; Energy (keV)",this->h1dsettings.at(3300));
		}
	}

	hismanager->RegisterPlot<TH1F>("BSMEXP_3300_PILEUP","MTAS Total #betaSM Pileup; Energy (kev)",this->h1dsettings.at(3300));

	//1D plots
	hismanager->RegisterPlot<TH1F>("BSM_3600","#betaSM Total; Energy (keV)",this->h1dsettings.at(3600));
	hismanager->RegisterPlot<TH1F>("BSM_3601","#betaSM Total No MTAS; Energy (keV)",this->h1dsettings.at(3601));
	hismanager->RegisterPlot<TH1F>("BSM_3602","#betaSM Total + MTAS Total; Energy (keV)",this->h1dsettings.at(3602));
	hismanager->RegisterPlot<TH1F>("BSM_3603","#betaSM Total + MTAS Total veto M,O; Energy (keV)",this->h1dsettings.at(3603));
	hismanager->RegisterPlot<TH1F>("BSM_3604","#betaSM Total + MTAS C Sum veto M,O; Energy (keV)",this->h1dsettings.at(3604));
	hismanager->RegisterPlot<TH1F>("BSM_3605","#betaSM Total + MTAS C Sum veto I,M,O; Energy (keV)",this->h1dsettings.at(3605));
	hismanager->RegisterPlot<TH1F>("BSM_3610","#betaSM Total; Energy (keV)",this->h1dsettings.at(3610));
	hismanager->RegisterPlot<TH1F>("BSM_3611","#betaSM Total [MTAS Pileup]; Energy (keV)",this->h1dsettings.at(3611));
	hismanager->RegisterPlot<TH1F>("BSM_3612","#betaSM Total [MTAS Saturate]; Energy (keV)",this->h1dsettings.at(3612));
	//duplicate, but split among the front and back pmts
	hismanager->RegisterPlot<TH1F>("BSM_3600_F","#betaSMF; Energy (keV)",this->h1dsettings.at(3600));
	hismanager->RegisterPlot<TH1F>("BSM_3601_F","#betaSMF No MTAS; Energy (keV)",this->h1dsettings.at(3601));
	hismanager->RegisterPlot<TH1F>("BSM_3602_F","#betaSMF + MTAS Total; Energy (keV)",this->h1dsettings.at(3602));
	hismanager->RegisterPlot<TH1F>("BSM_3603_F","#betaSMF + MTAS Total veto M,O; Energy (keV)",this->h1dsettings.at(3603));
	hismanager->RegisterPlot<TH1F>("BSM_3604_F","#betaSMF + MTAS C Sum veto M,O; Energy (keV)",this->h1dsettings.at(3604));
	hismanager->RegisterPlot<TH1F>("BSM_3605_F","#betaSMF + MTAS C Sum veto I,M,O; Energy (keV)",this->h1dsettings.at(3605));
	hismanager->RegisterPlot<TH1F>("BSM_3610_F","#betaSMF; Energy (keV)",this->h1dsettings.at(3610));
	hismanager->RegisterPlot<TH1F>("BSM_3611_F","#betaSMF [MTAS Pileup]; Energy (keV)",this->h1dsettings.at(3611));
	hismanager->RegisterPlot<TH1F>("BSM_3612_F","#betaSMF [MTAS Saturate]; Energy (keV)",this->h1dsettings.at(3612));
	hismanager->RegisterPlot<TH1F>("BSM_3600_B","#betaSM ; Energy (keV)",this->h1dsettings.at(3600));
	hismanager->RegisterPlot<TH1F>("BSM_3601_B","#betaSMB No MTAS; Energy (keV)",this->h1dsettings.at(3601));
	hismanager->RegisterPlot<TH1F>("BSM_3602_B","#betaSMB + MTAS Total; Energy (keV)",this->h1dsettings.at(3602));
	hismanager->RegisterPlot<TH1F>("BSM_3603_B","#betaSMB + MTAS Total veto M,O; Energy (keV)",this->h1dsettings.at(3603));
	hismanager->RegisterPlot<TH1F>("BSM_3604_B","#betaSMB + MTAS C Sum veto M,O; Energy (keV)",this->h1dsettings.at(3604));
	hismanager->RegisterPlot<TH1F>("BSM_3605_B","#betaSMB + MTAS C Sum veto I,M,O; Energy (keV)",this->h1dsettings.at(3605));
	hismanager->RegisterPlot<TH1F>("BSM_3610_B","#betaSMB; Energy (keV)",this->h1dsettings.at(3610));
	hismanager->RegisterPlot<TH1F>("BSM_3611_B","#betaSMB [MTAS Pileup]; Energy (keV)",this->h1dsettings.at(3611));
	hismanager->RegisterPlot<TH1F>("BSM_3612_B","#betaSMB [MTAS Saturate]; Energy (keV)",this->h1dsettings.at(3612));


	hismanager->RegisterPlot<TH2F>("BSM_3650","#betaSM Total vs MTAS Total; MTAS Total Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3650));
	hismanager->RegisterPlot<TH2F>("BSM_36508","#betaSM Total vs MTAS Total; MTAS Total Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36508));

	hismanager->RegisterPlot<TH2F>("BSM_3651","#betaSM Total vs MTAS I,M,O; MTAS I,M,O Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3651));
	hismanager->RegisterPlot<TH2F>("BSM_36518","#betaSM Total vs MTAS I,M,O; MTAS I,M,O Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36518));

	hismanager->RegisterPlot<TH2F>("BSM_3652","#betaSM Total vs MTAS Center Sum; MTAS Center Sum Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3651));
	hismanager->RegisterPlot<TH2F>("BSM_36528","#betaSM Total vs MTAS Center Sum; MTAS Center Sum Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36518));

	hismanager->RegisterPlot<TH2F>("BSM_3653","#betaSM Total vs MTAS C; MTAS C Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3653));
	hismanager->RegisterPlot<TH2F>("BSM_36538","#betaSM Total vs MTAS C; MTAS C Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36538));

	hismanager->RegisterPlot<TH2F>("BSM_3654","#betaSM Total vs MTAS Total Veto M,O; MTAS Total Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3654));
	hismanager->RegisterPlot<TH2F>("BSM_36548","#betaSM Total vs MTAS Total Veto M,O; MTAS Total Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36548));

	hismanager->RegisterPlot<TH2F>("BSM_3655","#betaSM Total vs MTAS Center Sum Veto M,O; MTAS Center Sum Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3655));
	hismanager->RegisterPlot<TH2F>("BSM_36558","#betaSM Total vs MTAS Center Sum Veto M,O; MTAS Center Sum Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36558));

	hismanager->RegisterPlot<TH2F>("BSM_3656","#betaSM Total vs MTAS C Veto M,O; MTAS C Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3656));
	hismanager->RegisterPlot<TH2F>("BSM_36568","#betaSM Total vs MTAS C Veto M,O; MTAS C Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36568));
	
	hismanager->RegisterPlot<TH2F>("BSM_3657","#betaSM Total vs MTAS Center Sum Veto I,M,O; MTAS Center Sum Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3657));
	hismanager->RegisterPlot<TH2F>("BSM_36578","#betaSM Total vs MTAS Center Sum Veto I,M,O; MTAS Center Sum Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36578));
	
	hismanager->RegisterPlot<TH2F>("BSM_3658","#betaSM Total vs MTAS C Veto I,M,O; MTAS C Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3658));
	hismanager->RegisterPlot<TH2F>("BSM_36588","#betaSM Total vs MTAS C Veto I,M,O; MTAS C Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36588));
	
	hismanager->RegisterPlot<TH2F>("BSM_3660","#betaSM Total + MTAS Total vs MTAS Total; MTAS Total Energy (keV); #betaSM Energy + MTAS Total Energy (keV)",this->h2dsettings.at(3660));
	hismanager->RegisterPlot<TH2F>("BSM_36608","#betaSM Total + MTAS Total vs MTAS Total; MTAS Total Energy (8 keV/bin); #betaSM Energy + MTAS Total Energy (8 keV/bin)",this->h2dsettings.at(36608));

	hismanager->RegisterPlot<TH2F>("BSM_3661","#betaSM Total vs #betaSM Total + MTAS Total; #betaSM Energy + MTAS Total Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3661));
	hismanager->RegisterPlot<TH2F>("BSM_36618","#betaSM Total vs #betaSM Total + MTAS Total; #betaSM Energy + MTAS Total Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36618));

	//duplicate for f/b, this is probably better done with the trees, but charlie wished for this for the initial publication, will probably
	//leave once we're done with it
	hismanager->RegisterPlot<TH2F>("BSM_3650_F","#betaSMF vs MTAS Total; MTAS Total Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3650));
	hismanager->RegisterPlot<TH2F>("BSM_36508_F","#betaSMF vs MTAS Total; MTAS Total Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36508));

	hismanager->RegisterPlot<TH2F>("BSM_3651_F","#betaSMF vs MTAS I,M,O; MTAS I,M,O Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3651));
	hismanager->RegisterPlot<TH2F>("BSM_36518_F","#betaSMF vs MTAS I,M,O; MTAS I,M,O Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36518));

	hismanager->RegisterPlot<TH2F>("BSM_3652_F","#betaSMF vs MTAS Center Sum; MTAS Center Sum Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3651));
	hismanager->RegisterPlot<TH2F>("BSM_36528_F","#betaSMF vs MTAS Center Sum; MTAS Center Sum Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36518));

	hismanager->RegisterPlot<TH2F>("BSM_3653_F","#betaSMF vs MTAS C; MTAS C Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3653));
	hismanager->RegisterPlot<TH2F>("BSM_36538_F","#betaSMF vs MTAS C; MTAS C Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36538));

	hismanager->RegisterPlot<TH2F>("BSM_3654_F","#betaSMF vs MTAS Total Veto M,O; MTAS Total Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3654));
	hismanager->RegisterPlot<TH2F>("BSM_36548_F","#betaSMF vs MTAS Total Veot M,O; MTAS Total Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36548));

	hismanager->RegisterPlot<TH2F>("BSM_3655_F","#betaSMF vs MTAS Center Sum Veto M,O; MTAS Center Sum Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3655));
	hismanager->RegisterPlot<TH2F>("BSM_36558_F","#betaSMF vs MTAS Center Sum Veto M,O; MTAS Center Sum Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36558));

	hismanager->RegisterPlot<TH2F>("BSM_3656_F","#betaSMF vs MTAS C Veto M,O; MTAS C Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3656));
	hismanager->RegisterPlot<TH2F>("BSM_36568_F","#betaSMF vs MTAS C Veto M,O; MTAS C Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36568));
	
	hismanager->RegisterPlot<TH2F>("BSM_3657_F","#betaSMF vs MTAS Center Sum Veto I,M,O; MTAS Center Sum Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3657));
	hismanager->RegisterPlot<TH2F>("BSM_36578_F","#betaSMF vs MTAS Center Sum Veto I,M,O; MTAS Center Sum Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36578));
	
	hismanager->RegisterPlot<TH2F>("BSM_3658_F","#betaSMF vs MTAS C Veto I,M,O; MTAS C Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3658));
	hismanager->RegisterPlot<TH2F>("BSM_36588_F","#betaSMF vs MTAS C Veto I,M,O; MTAS C Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36588));
	
	hismanager->RegisterPlot<TH2F>("BSM_3660_F","#betaSMF + MTAS Total vs MTAS Total; MTAS Total Energy (keV); #betaSM Energy + MTAS Total Energy (keV)",this->h2dsettings.at(3660));
	hismanager->RegisterPlot<TH2F>("BSM_36608_F","#betaSMF + MTAS Total vs MTAS Total; MTAS Total Energy (8 keV/bin); #betaSM Energy + MTAS Total Energy (8 keV/bin)",this->h2dsettings.at(36608));

	hismanager->RegisterPlot<TH2F>("BSM_3661_F","#betaSMF vs #betaSM Total + MTAS Total; #betaSM Energy + MTAS Total Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3661));
	hismanager->RegisterPlot<TH2F>("BSM_36618_F","#betaSMF vs #betaSM Total + MTAS Total; #betaSM Energy + MTAS Total Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36618));

	hismanager->RegisterPlot<TH2F>("BSM_3650_B","#betaSMB vs MTAS Total; MTAS Total Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3650));
	hismanager->RegisterPlot<TH2F>("BSM_36508_B","#betaSMB vs MTAS Total; MTAS Total Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36508));

	hismanager->RegisterPlot<TH2F>("BSM_3651_B","#betaSMB vs MTAS I,M,O; MTAS I,M,O Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3651));
	hismanager->RegisterPlot<TH2F>("BSM_36518_B","#betaSMB vs MTAS I,M,O; MTAS I,M,O Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36518));

	hismanager->RegisterPlot<TH2F>("BSM_3652_B","#betaSMB vs MTAS Center Sum; MTAS Center Sum Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3651));
	hismanager->RegisterPlot<TH2F>("BSM_36528_B","#betaSMB vs MTAS Center Sum; MTAS Center Sum Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36518));

	hismanager->RegisterPlot<TH2F>("BSM_3653_B","#betaSMB vs MTAS C; MTAS C Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3653));
	hismanager->RegisterPlot<TH2F>("BSM_36538_B","#betaSMB vs MTAS C; MTAS C Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36538));

	hismanager->RegisterPlot<TH2F>("BSM_3654_B","#betaSMB vs MTAS Total Veto M,O; MTAS Total Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3654));
	hismanager->RegisterPlot<TH2F>("BSM_36548_B","#betaSMB vs MTAS Total Veot M,O; MTAS Total Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36548));

	hismanager->RegisterPlot<TH2F>("BSM_3655_B","#betaSMB vs MTAS Center Sum Veto M,O; MTAS Center Sum Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3655));
	hismanager->RegisterPlot<TH2F>("BSM_36558_B","#betaSMB vs MTAS Center Sum Veto M,O; MTAS Center Sum Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36558));

	hismanager->RegisterPlot<TH2F>("BSM_3656_B","#betaSMB vs MTAS C Veto M,O; MTAS C Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3656));
	hismanager->RegisterPlot<TH2F>("BSM_36568_B","#betaSMB vs MTAS C Veto M,O; MTAS C Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36568));
	
	hismanager->RegisterPlot<TH2F>("BSM_3657_B","#betaSMB vs MTAS Center Sum Veto I,M,O; MTAS Center Sum Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3657));
	hismanager->RegisterPlot<TH2F>("BSM_36578_B","#betaSMB vs MTAS Center Sum Veto I,M,O; MTAS Center Sum Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36578));
	
	hismanager->RegisterPlot<TH2F>("BSM_3658_B","#betaSMB vs MTAS C Veto I,M,O; MTAS C Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3658));
	hismanager->RegisterPlot<TH2F>("BSM_36588_B","#betaSMB vs MTAS C Veto I,M,O; MTAS C Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36588));
	
	hismanager->RegisterPlot<TH2F>("BSM_3660_B","#betaSMB + MTAS Total vs MTAS Total; MTAS Total Energy (keV); #betaSM Energy + MTAS Total Energy (keV)",this->h2dsettings.at(3660));
	hismanager->RegisterPlot<TH2F>("BSM_36608_B","#betaSMB + MTAS Total vs MTAS Total; MTAS Total Energy (8 keV/bin); #betaSM Energy + MTAS Total Energy (8 keV/bin)",this->h2dsettings.at(36608));

	hismanager->RegisterPlot<TH2F>("BSM_3661_B","#betaSMB vs #betaSM Total + MTAS Total; #betaSM Energy + MTAS Total Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3661));
	hismanager->RegisterPlot<TH2F>("BSM_36618_B","#betaSMB vs #betaSM Total + MTAS Total; #betaSM Energy + MTAS Total Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36618));


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
