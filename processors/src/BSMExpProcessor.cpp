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

	if( this->HasBSM and AllWithinTDiff and AllWithinPos ){
		if( this->MtasProc->GetFirstFireTime() > 0.0 and this->BSMProc->GetFirstFireTime() > 0.0 ){
			hismanager->Fill(this->BSMEXP_2000,this->MtasProc->GetFirstFireTime() - this->BSMProc->GetFirstFireTime());
		}
		
		if( this->PPCutExists ){
			if( this->BSMProc->DidAnyPileup() ){
				hismanager->Fill(this->BSMEXP_3300_PILEUP,this->MtasProc->GetTotalEnergy(0));
			}
			for( size_t ii = 0; ii < 6 ; ++ii ){
				if( cutmanager->IsWithin("PairProduction",this->MtasProc->GetTotalEnergy(0),this->MtasProc->GetSumFrontBackEnergy(ii)) ){
					if( this->MtasProc->GetFirstFireTime() > 0.0 and this->BSMProc->GetFirstFireTime() > 0.0 ){
						hismanager->Fill(this->BSMEXP_2000_PP,this->MtasProc->GetFirstFireTime() - this->BSMProc->GetFirstFireTime());
						hismanager->Fill(this->BSMEXP_3600_PP,this->BSMProc->GetAverageTotalEnergy());
						break;
					}
				}
			}
		}

		if( (this->BSMProc->GetAverageTotalEnergy() >= this->BetaThreshold) and (not this->BSMProc->DidAnySaturate()) and (not this->BSMProc->DidAnyPileup()) ){
			this->MtasProc->FillBetaPlots(hismanager);
		}else{
			this->MtasProc->FillNonBetaPlots(hismanager);
		}
	}else{
		this->MtasProc->FillNonBetaPlots(hismanager);
	}

	hismanager->Fill("BSM_3610",this->BSMProc->GetAverageTotalEnergy());
	if( (not this->MtasProc->DidAnySaturate()) and (not this->MtasProc->DidAnyPileup()) and AllWithinTDiff and AllWithinPos ){
		this->BSMProc->FillPositionPlots(hismanager);
	
		hismanager->Fill(this->BSMEXP_3650,this->MtasProc->GetTotalEnergy(0),this->BSMProc->GetAverageTotalEnergy());
		hismanager->Fill(this->BSMEXP_36508,this->MtasProc->GetTotalEnergy(0),this->BSMProc->GetAverageTotalEnergy());

		hismanager->Fill(this->BSMEXP_3660,this->MtasProc->GetTotalEnergy(0),this->BSMProc->GetAverageTotalEnergy()+this->MtasProc->GetTotalEnergy(0));
		hismanager->Fill(this->BSMEXP_36608,this->MtasProc->GetTotalEnergy(0),this->BSMProc->GetAverageTotalEnergy()+this->MtasProc->GetTotalEnergy(0));

		hismanager->Fill(this->BSMEXP_3661,this->MtasProc->GetTotalEnergy(0)+this->BSMProc->GetAverageTotalEnergy(),this->BSMProc->GetAverageTotalEnergy());
		hismanager->Fill(this->BSMEXP_36618,this->MtasProc->GetTotalEnergy(0)+this->BSMProc->GetAverageTotalEnergy(),this->BSMProc->GetAverageTotalEnergy());

		hismanager->Fill(this->BSMEXP_3652,this->MtasProc->GetTotalEnergy(1),this->BSMProc->GetAverageTotalEnergy());
		hismanager->Fill(this->BSMEXP_36528,this->MtasProc->GetTotalEnergy(1),this->BSMProc->GetAverageTotalEnergy());

		if( not (this->MtasProc->DidAnyMiddleFire() or this->MtasProc->DidAnyOuterFire()) ){
			hismanager->Fill(this->BSMEXP_3654,this->MtasProc->GetTotalEnergy(0),this->BSMProc->GetAverageTotalEnergy());
			hismanager->Fill(this->BSMEXP_36548,this->MtasProc->GetTotalEnergy(0),this->BSMProc->GetAverageTotalEnergy());

			hismanager->Fill(this->BSMEXP_3655,this->MtasProc->GetTotalEnergy(1),this->BSMProc->GetAverageTotalEnergy());
			hismanager->Fill(this->BSMEXP_36558,this->MtasProc->GetTotalEnergy(1),this->BSMProc->GetAverageTotalEnergy());

			if( not this->MtasProc->DidAnyInnerFire() ){
				hismanager->Fill(this->BSMEXP_3657,this->MtasProc->GetTotalEnergy(1),this->BSMProc->GetAverageTotalEnergy());
				hismanager->Fill(this->BSMEXP_36578,this->MtasProc->GetTotalEnergy(1),this->BSMProc->GetAverageTotalEnergy());
			}
		}

		for( int ii = 0; ii < 6; ++ii ){
			hismanager->Fill(this->BSMEXP_3651,this->MtasProc->GetSumFrontBackEnergy(ii+6),this->BSMProc->GetAverageTotalEnergy());
			hismanager->Fill(this->BSMEXP_3651,this->MtasProc->GetSumFrontBackEnergy(ii+12),this->BSMProc->GetAverageTotalEnergy());
			hismanager->Fill(this->BSMEXP_3651,this->MtasProc->GetSumFrontBackEnergy(ii+18),this->BSMProc->GetAverageTotalEnergy());
			hismanager->Fill(this->BSMEXP_36518,this->MtasProc->GetSumFrontBackEnergy(ii+6),this->BSMProc->GetAverageTotalEnergy());
			hismanager->Fill(this->BSMEXP_36518,this->MtasProc->GetSumFrontBackEnergy(ii+12),this->BSMProc->GetAverageTotalEnergy());
			hismanager->Fill(this->BSMEXP_36518,this->MtasProc->GetSumFrontBackEnergy(ii+18),this->BSMProc->GetAverageTotalEnergy());
			
			hismanager->Fill(this->BSMEXP_3653,this->MtasProc->GetSumFrontBackEnergy(ii),this->BSMProc->GetAverageTotalEnergy());
			hismanager->Fill(this->BSMEXP_36538,this->MtasProc->GetSumFrontBackEnergy(ii),this->BSMProc->GetAverageTotalEnergy());
			
			if( not (this->MtasProc->DidAnyMiddleFire() or this->MtasProc->DidAnyOuterFire()) ){
				hismanager->Fill(this->BSMEXP_3656,this->MtasProc->GetSumFrontBackEnergy(ii),this->BSMProc->GetAverageTotalEnergy());
				hismanager->Fill(this->BSMEXP_36568,this->MtasProc->GetSumFrontBackEnergy(ii),this->BSMProc->GetAverageTotalEnergy());
				
				if( not this->MtasProc->DidAnyInnerFire() ){
					hismanager->Fill(this->BSMEXP_3658,this->MtasProc->GetSumFrontBackEnergy(ii),this->BSMProc->GetAverageTotalEnergy());
					hismanager->Fill(this->BSMEXP_36588,this->MtasProc->GetSumFrontBackEnergy(ii),this->BSMProc->GetAverageTotalEnergy());
				}
			}
		}

		hismanager->Fill(this->BSMEXP_3600,this->BSMProc->GetAverageTotalEnergy());
		hismanager->Fill(this->BSMEXP_3602,this->BSMProc->GetAverageTotalEnergy()+this->MtasProc->GetTotalEnergy(0));
		if( not (this->MtasProc->DidAnyMiddleFire() or this->MtasProc->DidAnyOuterFire()) ){
			hismanager->Fill(this->BSMEXP_3603,this->BSMProc->GetAverageTotalEnergy()+this->MtasProc->GetTotalEnergy(0));
			hismanager->Fill(this->BSMEXP_3604,this->BSMProc->GetAverageTotalEnergy()+this->MtasProc->GetTotalEnergy(1));
			if( not this->MtasProc->DidAnyInnerFire() ){
				hismanager->Fill(this->BSMEXP_3605,this->BSMProc->GetAverageTotalEnergy()+this->MtasProc->GetTotalEnergy(1));
			}
		}
		if( (not this->HasMTAS) or this->MtasProc->GetTotalEnergy(0) < 1.0 ){
			hismanager->Fill(this->BSMEXP_3601,this->BSMProc->GetAverageTotalEnergy());
			if( this->BSMProc->GetAverageTotalEnergy() > this->QBeta ){
				this->BSMProc->FillGSPileupTracePlots(hismanager);
			}
		}
	}else{
		if( this->MtasProc->DidAnyPileup() ){
			hismanager->Fill(this->BSMEXP_3611,this->BSMProc->GetAverageTotalEnergy());
		}
		if( this->MtasProc->DidAnySaturate() ){
			hismanager->Fill(this->BSMEXP_3612,this->BSMProc->GetAverageTotalEnergy());
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
}
		
void BSMExpProcessor::Finalize(){
	this->MtasProc->Finalize();
	this->BSMProc->Finalize();

	this->console->info("{} has been finalized",this->ProcessorName);
}

void BSMExpProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager){
	this->MtasProc->DeclarePlots(hismanager);
	this->BSMProc->DeclarePlots(hismanager);

	this->BSMEXP_2000 = hismanager->RegisterPlot<TH1F>("BSMEXP_2000","TDiff (#betaSM - Mtas); TDiff (ns)",this->h1dsettings.at(2000));

	if( this->PPCutExists ){
		this->BSMEXP_3600_PP = hismanager->RegisterPlot<TH1F>("BSMEXP_3600_PP","#betaSM Energy [PairProduction]; Energy (keV)",this->h1dsettings.at(3600));
		this->BSMEXP_2000_PP =  hismanager->RegisterPlot<TH1F>("BSMEXP_2000_PP","TDiff (#betaSM - Mtas) [PairProduction]; TDiff (ns)",this->h1dsettings.at(2000));
	}

	this->BSMEXP_3300_PILEUP =  hismanager->RegisterPlot<TH1F>("BSMEXP_3300_PILEUP","MTAS Total #betaSM Pileup; Energy (kev)",this->h1dsettings.at(3300));

	this->BSMEXP_3600 = hismanager->RegisterPlot<TH1F>("BSM_3600","#betaSM Total; Energy (keV)",this->h1dsettings.at(3600));
	this->BSMEXP_3601 = hismanager->RegisterPlot<TH1F>("BSM_3601","#betaSM Total No MTAS; Energy (keV)",this->h1dsettings.at(3601));
	this->BSMEXP_3602 = hismanager->RegisterPlot<TH1F>("BSM_3602","#betaSM Total + MTAS Total; Energy (keV)",this->h1dsettings.at(3602));
	this->BSMEXP_3603 = hismanager->RegisterPlot<TH1F>("BSM_3603","#betaSM Total + MTAS Total veto M,O; Energy (keV)",this->h1dsettings.at(3603));
	this->BSMEXP_3604 = hismanager->RegisterPlot<TH1F>("BSM_3604","#betaSM Total + MTAS C Sum veto M,O; Energy (keV)",this->h1dsettings.at(3604));
	this->BSMEXP_3605 = hismanager->RegisterPlot<TH1F>("BSM_3605","#betaSM Total + MTAS C Sum veto I,M,O; Energy (keV)",this->h1dsettings.at(3605));
	this->BSMEXP_3610 = hismanager->RegisterPlot<TH1F>("BSM_3610","#betaSM Total; Energy (keV)",this->h1dsettings.at(3610));
	this->BSMEXP_3611 = hismanager->RegisterPlot<TH1F>("BSM_3611","#betaSM Total [MTAS Pileup]; Energy (keV)",this->h1dsettings.at(3611));
	this->BSMEXP_3612 = hismanager->RegisterPlot<TH1F>("BSM_3612","#betaSM Total [MTAS Saturate]; Energy (keV)",this->h1dsettings.at(3612));
	
	this->BSMEXP_3650 = hismanager->RegisterPlot<TH2F>("BSM_3650","#betaSM Total vs MTAS Total; MTAS Total Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3650));
	this->BSMEXP_36508 = hismanager->RegisterPlot<TH2F>("BSM_36508","#betaSM Total vs MTAS Total; MTAS Total Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36508));

	this->BSMEXP_3651 = hismanager->RegisterPlot<TH2F>("BSM_3651","#betaSM Total vs MTAS I,M,O; MTAS I,M,O Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3651));
	this->BSMEXP_36518 = hismanager->RegisterPlot<TH2F>("BSM_36518","#betaSM Total vs MTAS I,M,O; MTAS I,M,O Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36518));

	this->BSMEXP_3652 = hismanager->RegisterPlot<TH2F>("BSM_3652","#betaSM Total vs MTAS Center Sum; MTAS Center Sum Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3651));
	this->BSMEXP_36528 = hismanager->RegisterPlot<TH2F>("BSM_36528","#betaSM Total vs MTAS Center Sum; MTAS Center Sum Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36518));

	this->BSMEXP_3653 = hismanager->RegisterPlot<TH2F>("BSM_3653","#betaSM Total vs MTAS C; MTAS C Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3653));
	this->BSMEXP_36538 = hismanager->RegisterPlot<TH2F>("BSM_36538","#betaSM Total vs MTAS C; MTAS C Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36538));

	this->BSMEXP_3654 = hismanager->RegisterPlot<TH2F>("BSM_3654","#betaSM Total vs MTAS Total Veto M,O; MTAS Total Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3654));
	this->BSMEXP_36548 = hismanager->RegisterPlot<TH2F>("BSM_36548","#betaSM Total vs MTAS Total Veot M,O; MTAS Total Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36548));

	this->BSMEXP_3655 = hismanager->RegisterPlot<TH2F>("BSM_3655","#betaSM Total vs MTAS Center Sum Veto M,O; MTAS Center Sum Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3655));
	this->BSMEXP_36558 = hismanager->RegisterPlot<TH2F>("BSM_36558","#betaSM Total vs MTAS Center Sum Veto M,O; MTAS Center Sum Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36558));

	this->BSMEXP_3656 = hismanager->RegisterPlot<TH2F>("BSM_3656","#betaSM Total vs MTAS C Veto M,O; MTAS C Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3656));
	this->BSMEXP_36568 = hismanager->RegisterPlot<TH2F>("BSM_36568","#betaSM Total vs MTAS C Veto M,O; MTAS C Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36568));
	
	this->BSMEXP_3657 = hismanager->RegisterPlot<TH2F>("BSM_3657","#betaSM Total vs MTAS Center Sum Veto I,M,O; MTAS Center Sum Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3657));
	this->BSMEXP_36578 = hismanager->RegisterPlot<TH2F>("BSM_36578","#betaSM Total vs MTAS Center Sum Veto I,M,O; MTAS Center Sum Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36578));
	
	this->BSMEXP_3658 = hismanager->RegisterPlot<TH2F>("BSM_3658","#betaSM Total vs MTAS C Veto I,M,O; MTAS C Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3658));
	this->BSMEXP_36588 = hismanager->RegisterPlot<TH2F>("BSM_36588","#betaSM Total vs MTAS C Veto I,M,O; MTAS C Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36588));
	
	this->BSMEXP_3660 = hismanager->RegisterPlot<TH2F>("BSM_3660","#betaSM Total + MTAS Total vs MTAS Total; MTAS Total Energy (keV); #betaSM Energy + MTAS Total Energy (keV)",this->h2dsettings.at(3660));
	this->BSMEXP_36608 = hismanager->RegisterPlot<TH2F>("BSM_36608","#betaSM Total + MTAS Total vs MTAS Total; MTAS Total Energy (8 keV/bin); #betaSM Energy + MTAS Total Energy (8 keV/bin)",this->h2dsettings.at(36608));

	this->BSMEXP_3661 = hismanager->RegisterPlot<TH2F>("BSM_3661","#betaSM Total vs #betaSM Total + MTAS Total; #betaSM Energy + MTAS Total Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3661));
	this->BSMEXP_36618 = hismanager->RegisterPlot<TH2F>("BSM_36618","#betaSM Total vs #betaSM Total + MTAS Total; #betaSM Energy + MTAS Total Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36618));

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
