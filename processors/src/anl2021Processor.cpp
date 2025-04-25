#include "anl2021Processor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "TapeCycle.hpp"
#include <TTree.h>
#include <stdexcept>

anl2021Processor::anl2021Processor(const std::string& log) : Processor(log,"anl2021Processor",{}){
	this->MtasProc = std::make_shared<MtasProcessor>(log);
	this->SiliconProc = std::make_shared<MtasSSDProcessor>(log);
	this->TapeProc = std::make_shared<MtasTapeProcessor>(log);
	this->HPGeProc = std::make_shared<SimpleHPGeProcessor>(log);
	this->ImplantProc = std::make_shared<PSPMTProcessor>(log);

	//need also the hpge and two implants
	for( const auto& type : this->MtasProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	for( const auto& type : this->SiliconProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	for( const auto& type : this->TapeProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	for( const auto& type : this->HPGeProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	for( const auto& type : this->ImplantProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	this->h1dsettings = {
		{3100,{16384,0,16384}},
		{3200,{16384,0,16384}},
		{3300,{16384,0,16384}}
	};

	this->h2dsettings = {
		{3700,{8192,0,8192,1000,0,100000}}
	};

	this->beta = "beta";
	this->gamma = "gamma";
	this->muon = "muon";

	this->HasMTAS = false;
	this->HasTape = false;
	this->HasSilicon = false;
	this->HasHPGe = false;
	this->HasPSPMT = false;

	this->SiliconThreshold = 0.0;
	this->ImplantThreshold = 0.0;

	this->Reset();
}

[[maybe_unused]] bool anl2021Processor::PreProcess(EventHistoryManager* eventhistory, PLOTS::PlotRegistry* hismanager, CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	auto summary = eventhistory->GetCurrentEventSummary();
	auto types = summary->GetKnownTypes();

	this->HasMTAS = (types.find("mtas") != types.end());
	this->HasSilicon = (types.find("silicon") != types.end());
	this->HasTape = (types.find("tape") != types.end());
	this->HasHPGe = (types.find("hpge") != types.end()); 
	this->HasPSPMT = (types.find("pspmt") != types.end()); 

	if( this->HasTape ){
		this->TapeProc->PreProcess(eventhistory,hismanager,cutmanager);
	}

	if( this->HasPSPMT ){
		this->ImplantProc->PreProcess(eventhistory,hismanager,cutmanager);
		auto lgImage = this->ImplantProc->GetLowGainImage();
		if( lgImage.anodesum > this->ImplantThreshold ){
			summary->AddEventTag(this->implant);
		}
	}

	if( this->HasHPGe ){
		this->HPGeProc->PreProcess(eventhistory,hismanager,cutmanager);
	}

	if( this->HasSilicon ){
		this->SiliconProc->PreProcess(eventhistory,hismanager,cutmanager);
		if( this->SiliconProc->GetMaxEnergy() > this->SiliconThreshold ){
			summary->AddEventTag(this->beta);
		}
	}

	if( this->HasMTAS ){
		this->MtasProc->PreProcess(eventhistory,hismanager,cutmanager);
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool anl2021Processor::Process( EventHistoryManager* eventhistory, PLOTS::PlotRegistry* hismanager, CUTS::CutRegistry* cutmanager){
	Processor::Process();
	//determine if we had a beta trigger in MTAS and are in the correct cycle
	auto numhist = eventhistory->GetMaxHistoryID();
	auto summary = eventhistory->GetCurrentEventSummary();
	auto isomer_tdiff = 0.0;
	auto erg = MtasProc->GetTotalEnergy(0);
	bool possible_gamma_isomer = summary->ContainsEventTag(this->gamma) and not summary->ContainsEventTag(this->beta) and not summary->ContainsEventTag(this->muon); 

	if( numhist > 1 ){
		//current event has gamma not-muon, not-beta
		if( possible_gamma_isomer ){
			//search through history and try to find past event that has does have beta but does not have muon or gamma
			for( size_t ii = 1; ii < numhist; ++ii ){
				auto currhist = eventhistory->GetPreviousEventSummary(ii);
				if( currhist->ContainsEventTag(this->beta) and not currhist->ContainsEventTag(this->gamma) and not currhist->ContainsEventTag(this->muon) ){
					isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - currhist->GetRawEvents().front().GetTimeStamp();
					hismanager->Fill("ISOMER_3701",erg,isomer_tdiff*1.0e-3);
				}
			}
		}
	}

	if( TapeProc->GetCurrentCycleState() == TAPE::MEASURE ){
		if( summary->ContainsEventTag(this->beta) ){
			this->MtasProc->FillBetaPlots(hismanager);
		}else{
			this->MtasProc->FillNonBetaPlots(hismanager);
		}
		if( possible_gamma_isomer ){
			hismanager->Fill("ISOMER_3700",erg,isomer_tdiff*1.0e-3);
		}
	}else if( TapeProc->GetCurrentCycleState() == TAPE::BACKGROUND ){
		if( not this->MtasProc->DidAnyPileup() and not this->MtasProc->DidAnySaturate() ){
			hismanager->Fill("BKG_3200",this->MtasProc->GetTotalEnergy(0));
			if( summary->ContainsEventTag(this->beta) ){
				hismanager->Fill("BKG_3300",this->MtasProc->GetTotalEnergy(0));
			}else{
				hismanager->Fill("BKG_3100",this->MtasProc->GetTotalEnergy(0));
			}
		}
	}
	
	if( summary->ContainsEventTag(this->beta) ){
		this->MtasProc->FillNoLogicBetaPlots(hismanager);
	}else{
		this->MtasProc->FillNoLogicNonBetaPlots(hismanager);
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool anl2021Processor::PostProcess( EventHistoryManager* eventhistory, PLOTS::PlotRegistry* hismanager, CUTS::CutRegistry* cutmanager){
	Processor::PostProcess();
	if( this->HasTape ){
		this->TapeProc->PostProcess(eventhistory,hismanager,cutmanager);
	}
	if( this->HasPSPMT ){
		this->ImplantProc->PostProcess(eventhistory,hismanager,cutmanager);
	}
	if( this->HasHPGe ){
		this->HPGeProc->PostProcess(eventhistory,hismanager,cutmanager);
	}
	if( this->HasSilicon ){
		this->SiliconProc->PostProcess(eventhistory,hismanager,cutmanager);
	}
	if( this->HasMTAS ){
		this->MtasProc->PostProcess(eventhistory,hismanager,cutmanager);
	}
	this->Reset();

	Processor::EndProcess();
	return true;
}

void anl2021Processor::Init(const pugi::xml_node& config){
	for( pugi::xml_node proc = config.child("Processor"); proc; proc = proc.next_sibling("Processor") ){
		std::string name = proc.attribute("name").as_string();
		if( name.compare("MtasProcessor") == 0 ){
			this->MtasProc->Init(proc);
			this->HasMTAS = true;
		}else if( name.compare("MtasSSDProcessor") == 0 ){
			this->SiliconProc->Init(proc);
			this->HasSilicon = true;
		}else if( name.compare("MtasTapeProcessor") == 0 ){
			this->TapeProc->Init(proc);
			this->HasTape = true;
		}else if( name.compare("PSPMTProcessor") == 0 ){
			this->ImplantProc->Init(proc);
			this->HasPSPMT = true;
		}else if( name.compare("SimpleHPGeProcessor") == 0 ){
			this->HPGeProc->Init(proc);
			this->HasHPGe = true;
		}else{
			throw std::runtime_error("unknown subprocessor declared in anl2021Processor");
		}
	}

	this->SiliconThreshold = config.attribute("siliconthresh").as_double(0.0);
	this->ImplantThreshold = config.attribute("implantthresh").as_double(0.0);

	if( not this->HasMTAS ){
		throw std::runtime_error("missing MtasProcessor in anl2021Processor");
	}

	if( not this->HasSilicon ){
		throw std::runtime_error("missing MtasSSDProcessor in anl2021Processor");
	}

	if( not this->HasTape ){
		throw std::runtime_error("missing MtasTapeProcessor in anl2021Processor");
	}

	if( not this->HasHPGe ){
		throw std::runtime_error("missing SimpleHPGeProcessor in anl2021Processor");
	}

	if( not this->HasPSPMT ){
		throw std::runtime_error("missing PSPMTProcessor in anl2021Processor");
	}

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}
		
void anl2021Processor::Finalize(){
	this->MtasProc->Finalize();
	this->TapeProc->Finalize();
	this->SiliconProc->Finalize();
	this->HPGeProc->Finalize();
	this->ImplantProc->Finalize();

	this->console->info("{} has been finalized",this->ProcessorName);
}

void anl2021Processor::DeclarePlots(PLOTS::PlotRegistry* hismanager){
	this->MtasProc->DeclarePlots(hismanager);
	this->TapeProc->DeclarePlots(hismanager);
	this->SiliconProc->DeclarePlots(hismanager);
	this->HPGeProc->DeclarePlots(hismanager);
	this->ImplantProc->DeclarePlots(hismanager);

	hismanager->RegisterPlot<TH1F>("BKG_3100","Mtas Background Cycle Gated anti-#beta Gated; Energy (keV)",this->h1dsettings.at(3100));
	hismanager->RegisterPlot<TH1F>("BKG_3200","Mtas Background Cycle Gated; Energy (keV)",this->h1dsettings.at(3200));
	hismanager->RegisterPlot<TH1F>("BKG_3300","Mtas Background Cycle Gated #beta Gated; Energy (keV)",this->h1dsettings.at(3300));
	
	hismanager->RegisterPlot<TH2F>("ISOMER_3700","Mtas delayed #gamma Measure Cycle Gated (no-#beta); Energy (keV); Time (us)",this->h2dsettings.at(3700));
	hismanager->RegisterPlot<TH2F>("ISOMER_3701","Mtas delayed #gamma No Cycle (no-#beta); Energy (keV); Time (us)",this->h2dsettings.at(3700));

	this->console->info("Finished Declaring Plots");
}

void anl2021Processor::RegisterTree(std::unordered_map<std::string,TTree*>& outputtrees){
	this->MtasProc->RegisterTree(outputtrees);
	this->TapeProc->RegisterTree(outputtrees);
	this->SiliconProc->RegisterTree(outputtrees);
	this->HPGeProc->RegisterTree(outputtrees);
	this->ImplantProc->RegisterTree(outputtrees);
}

void anl2021Processor::CleanupTree(){
	this->MtasProc->CleanupTree();
	this->TapeProc->CleanupTree();
	this->SiliconProc->CleanupTree();
	this->HPGeProc->CleanupTree();
	this->ImplantProc->CleanupTree();
}

void anl2021Processor::Reset(){
}
