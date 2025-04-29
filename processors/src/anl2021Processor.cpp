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
		{3700,{8192,0,8192,1000,0,10000}},
		{3701,{8192,0,8192,1000,0,10000}},
		{3800,{8192,0,8192,1000,0,10000}},
		{3801,{8192,0,8192,1000,0,10000}}
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
	auto summary = eventhistory->GetCurrentEventSummary();
	bool hasmuon = summary->ContainsEventTag(this->muon);

	if( not hasmuon ){
		auto numhist = eventhistory->GetMaxHistoryID();
		auto erg = MtasProc->GetTotalEnergy(0);

		bool hasbeta = summary->ContainsEventTag(this->beta);
		bool hasgamma = summary->ContainsEventTag(this->gamma);

		if( TapeProc->GetCurrentCycleState() == TAPE::MEASURE ){
			if( hasbeta ){
				this->MtasProc->FillBetaPlots(hismanager);
			}else{
				this->MtasProc->FillNonBetaPlots(hismanager);
			}

			if( numhist > 1 ){
				//current event has gamma not-muon, not-beta
				for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
					auto prevsummary =  eventhistory->GetPreviousEventSummary(ii);
					auto prevmuon = prevsummary->ContainsEventTag(this->muon);
					if( prevmuon ){
						continue;
					}
					auto prevbeta = prevsummary->ContainsEventTag(this->beta);
					auto prevgamma = prevsummary->ContainsEventTag(this->gamma);
					auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
					//this looks for a beta decay into a delayed level
					//like 137Cs
					if( not hasbeta and hasgamma and prevbeta ){
						hismanager->Fill("ISOMER_3700",erg,isomer_tdiff);
						hismanager->Fill("ISOMER_3701",erg,isomer_tdiff*1.0e-3);
						break;
					}
				}
				for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
					auto prevsummary =  eventhistory->GetPreviousEventSummary(ii);
					auto prevmuon = prevsummary->ContainsEventTag(this->muon);
					if( prevmuon ){
						continue;
					}
					auto prevbeta = prevsummary->ContainsEventTag(this->beta);
					auto prevgamma = prevsummary->ContainsEventTag(this->gamma);
					auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();

					//this looks for a gamma decay into a delayed beta
					//i.e. beam isomer, but need mtas energy for this old event
					if( hasbeta and not prevbeta and prevgamma ){
						auto olderg = prevsummary->GetEventObservable("MTAS_Total").value();
						hismanager->Fill("ISOMER_3800",olderg,isomer_tdiff);
						hismanager->Fill("ISOMER_3801",olderg,isomer_tdiff*1.0e-3);
						break;
					}
				}
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
		}else{
			//no-op
			//these are when we're in move or irradiate which we probably should check irradiate
		}

		if( hasbeta ){
			this->MtasProc->FillNoLogicBetaPlots(hismanager);
		}else{
			this->MtasProc->FillNoLogicNonBetaPlots(hismanager);
		}
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
			if( not proc.attribute("oldcenter").as_bool(false) ){
				throw std::runtime_error("Need oldcenter=\"true\" on the MtasProcessor child tag for anl2021Processor"); 
			}
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
	
	hismanager->RegisterPlot<TH2F>("ISOMER_3700","Mtas prev-#beta curr-no-#beta Measure Cycle Gated; Energy (keV); Time (ns)",this->h2dsettings.at(3700));
	hismanager->RegisterPlot<TH2F>("ISOMER_3701","Mtas prev-#beta curr-no-#beta Measure Cycle Gated; Energy (keV); Time (us)",this->h2dsettings.at(3701));

	hismanager->RegisterPlot<TH2F>("ISOMER_3800","Mtas prev-#gamma curr-#beta Measure Cycle Gated; Energy (keV); Time (ns)",this->h2dsettings.at(3800));
	hismanager->RegisterPlot<TH2F>("ISOMER_3801","Mtas prev-#gamma curr-#beta Measure Cycle Gated; Energy (keV); Time (us)",this->h2dsettings.at(3801));

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
