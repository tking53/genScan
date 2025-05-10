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
		{1000,{16384,0,16384}},
		{1001,{16384,0,16384}},
		{1002,{16384,0,16384}},
		{1003,{16384,0,16384}},
		
		{1010,{16384,-8192,8191}},

		{3100,{16384,0,16384}},

		{3200,{16384,0,16384}},

		{3300,{16384,0,16384}}
	};

	this->h2dsettings = {
		{3160,{8192,0,8192,512,0,512}},
		{3161,{8192,0,8192,512,0,512}},
		{3162,{8192,0,8192,512,0,512}},
		{3163,{8192,0,8192,512,0,512}},

		{3260,{8192,0,8192,512,0,512}},
		{3261,{8192,0,8192,512,0,512}},
		{3262,{8192,0,8192,512,0,512}},
		{3263,{8192,0,8192,512,0,512}},
		
		{3350,{4096,0,4096,4096,0,4096}},
		{33508,{2048,0,8192,2048,0,8192}},
		{3351,{4096,0,4096,4096,0,4096}},
		{33518,{2048,0,8192,2048,0,8192}},

		{3360,{8192,0,8192,512,0,512}},
		{3361,{8192,0,8192,512,0,512}},
		{3362,{8192,0,8192,512,0,512}},
		{3363,{8192,0,8192,512,0,512}},

		{31608,{2048,0,16384,512,0,512}},
		{31618,{2048,0,16384,512,0,512}},
		{31628,{2048,0,16384,512,0,512}},
		{31638,{2048,0,16384,512,0,512}},

		{32608,{2048,0,16384,512,0,512}},
		{32618,{2048,0,16384,512,0,512}},
		{32628,{2048,0,16384,512,0,512}},
		{32638,{2048,0,16384,512,0,512}},

		{33608,{2048,0,16384,512,0,512}},
		{33618,{2048,0,16384,512,0,512}},
		{33628,{2048,0,16384,512,0,512}},
		{33638,{2048,0,16384,512,0,512}},

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

	hismanager->Fill("EARLY_1010",this->EarlyCycle.GetLowerBound());
	hismanager->Fill("EARLY_1010",this->EarlyCycle.GetUpperBound());

	hismanager->Fill("MID_1010",this->MidCycle.GetLowerBound());
	hismanager->Fill("MID_1010",this->MidCycle.GetUpperBound());

	hismanager->Fill("LATE_1010",this->LateCycle.GetLowerBound());
	hismanager->Fill("LATE_1010",this->LateCycle.GetUpperBound());

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
		auto cerg = MtasProc->GetTotalEnergy(1);
		auto cyclestarttime = TapeProc->GetCycleTimeInSeconds();
		//mtas gives the time in ns
		auto firstmtastime = MtasProc->GetFirstFireTime()*1.0e-9;
		auto cycletime = firstmtastime - cyclestarttime;

		bool hasbeta = summary->ContainsEventTag(this->beta);
		bool hasgamma = summary->ContainsEventTag(this->gamma);

		if( TapeProc->GetCurrentCycleState() == TAPE::MEASURE ){
			hismanager->Fill("CYCLE_1000",cycletime*1.0e3);
			hismanager->Fill("CYCLE_1001",cycletime);
			hismanager->Fill("CYCLE_1002",cycletime/60.0);
			hismanager->Fill("CYCLE_1003",cycletime/(60.0*60.0));

			hismanager->Fill("MEASURE_3260",erg,cycletime*1.0e3);
			hismanager->Fill("MEASURE_3261",erg,cycletime);
			hismanager->Fill("MEASURE_3262",erg,cycletime/60.0);
			hismanager->Fill("MEASURE_3263",erg,cycletime/(60.0*60.0));

			hismanager->Fill("MEASURE_32608",erg,cycletime*1.0e3);
			hismanager->Fill("MEASURE_32618",erg,cycletime);
			hismanager->Fill("MEASURE_32628",erg,cycletime/60.0);
			hismanager->Fill("MEASURE_32638",erg,cycletime/(60.0*60.0));

			if( hasbeta ){
				if( this->EarlyCycle.IsWithin(cycletime) ){
					hismanager->Fill("EARLY_3300",erg);
					hismanager->Fill("EARLY_3351",erg,cerg);
					for( size_t ii = 6; ii < 24; ++ii ){
						hismanager->Fill("EARLY_3350",MtasProc->GetCrystalEnergy(ii),erg);
					}
				}else if( this->MidCycle.IsWithin(cycletime) ){
					hismanager->Fill("MID_3300",erg);
					hismanager->Fill("MID_3351",erg,cerg);
					for( size_t ii = 6; ii < 24; ++ii ){
						hismanager->Fill("MID_3350",MtasProc->GetCrystalEnergy(ii),erg);
					}
				}else if( this->LateCycle.IsWithin(cycletime) ){
					hismanager->Fill("LATE_3300",erg);
					hismanager->Fill("LATE_3351",erg,cerg);
					for( size_t ii = 6; ii < 24; ++ii ){
						hismanager->Fill("LATE_3350",MtasProc->GetCrystalEnergy(ii),erg);
					}
				}else{
					//no-op
				}
				this->MtasProc->FillBetaPlots(hismanager);
				hismanager->Fill("MEASURE_3360",erg,cycletime*1.0e3);
				hismanager->Fill("MEASURE_3361",erg,cycletime);
				hismanager->Fill("MEASURE_3362",erg,cycletime/60.0);
				hismanager->Fill("MEASURE_3363",erg,cycletime/(60.0*60.0));

				hismanager->Fill("MEASURE_33608",erg,cycletime*1.0e3);
				hismanager->Fill("MEASURE_33618",erg,cycletime);
				hismanager->Fill("MEASURE_33628",erg,cycletime/60.0);
				hismanager->Fill("MEASURE_33638",erg,cycletime/(60.0*60.0));
			}else{
				this->MtasProc->FillNonBetaPlots(hismanager);
				hismanager->Fill("MEASURE_3160",erg,cycletime*1.0e3);
				hismanager->Fill("MEASURE_3161",erg,cycletime);
				hismanager->Fill("MEASURE_3162",erg,cycletime/60.0);
				hismanager->Fill("MEASURE_3163",erg,cycletime/(60.0*60.0));

				hismanager->Fill("MEASURE_31608",erg,cycletime*1.0e3);
				hismanager->Fill("MEASURE_31618",erg,cycletime);
				hismanager->Fill("MEASURE_31628",erg,cycletime/60.0);
				hismanager->Fill("MEASURE_31638",erg,cycletime/(60.0*60.0));
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
			//we can find beam isomers from the implant if we do this
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
	//need to load in early and late time gate for generating duplicates of 3350 3351 since they're not easy to make without a shitload of memory
	auto earlygate = config.child("EarlyCycle");
	if( earlygate ){
		this->EarlyCycle = Gate<double>(earlygate.attribute("lowerbound").as_double(0.0),earlygate.attribute("upperbound").as_double(0.0));
	}else{
		this->EarlyCycle = Gate<double>(0.0,0.0);
	}

	auto midgate = config.child("MidCycle");
	if( midgate ){
		this->MidCycle = Gate<double>(midgate.attribute("lowerbound").as_double(0.0),midgate.attribute("upperbound").as_double(0.0));
	}else{
		this->MidCycle = Gate<double>(0.0,0.0);
	}

	auto lategate = config.child("LateCycle");
	if( lategate ){
		this->LateCycle = Gate<double>(lategate.attribute("lowerbound").as_double(0.0),lategate.attribute("upperbound").as_double(0.0));
	}else{
		this->LateCycle = Gate<double>(0.0,0.0);
	}

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

	hismanager->RegisterPlot<TH2F>("MEASURE_3160","Mtas Total vs Cycle Time (ms) anti-#beta-gated; Mtas Total Energy (keV); Cycle Time (ms)",this->h2dsettings.at(3160));
	hismanager->RegisterPlot<TH2F>("MEASURE_3161","Mtas Total vs Cycle Time (s) anti-#beta-gated; Mtas Total Energy (keV); Cycle Time (s)",this->h2dsettings.at(3161));
	hismanager->RegisterPlot<TH2F>("MEASURE_3162","Mtas Total vs Cycle Time (min) anti-#beta-gated; Mtas Total Energy (keV); Cycle Time (min)",this->h2dsettings.at(3162));
	hismanager->RegisterPlot<TH2F>("MEASURE_3163","Mtas Total vs Cycle Time (hr) anti-#beta-gated; Mtas Total Energy (keV); Cycle Time (hr)",this->h2dsettings.at(3163));

	hismanager->RegisterPlot<TH2F>("MEASURE_3260","Mtas Total vs Cycle Time (ms); Mtas Total Energy (keV); Cycle Time (ms)",this->h2dsettings.at(3260));
	hismanager->RegisterPlot<TH2F>("MEASURE_3261","Mtas Total vs Cycle Time (s); Mtas Total Energy (keV); Cycle Time (s)",this->h2dsettings.at(3261));
	hismanager->RegisterPlot<TH2F>("MEASURE_3262","Mtas Total vs Cycle Time (min); Mtas Total Energy (keV); Cycle Time (min)",this->h2dsettings.at(3262));
	hismanager->RegisterPlot<TH2F>("MEASURE_3263","Mtas Total vs Cycle Time (hr); Mtas Total Energy (keV); Cycle Time (hr)",this->h2dsettings.at(3263));

	hismanager->RegisterPlot<TH2F>("MEASURE_3360","Mtas Total vs Cycle Time (ms) #beta-gated; Mtas Total Energy (keV); Cycle Time (ms)",this->h2dsettings.at(3360));
	hismanager->RegisterPlot<TH2F>("MEASURE_3361","Mtas Total vs Cycle Time (s) #beta-gated; Mtas Total Energy (keV); Cycle Time (s)",this->h2dsettings.at(3361));
	hismanager->RegisterPlot<TH2F>("MEASURE_3362","Mtas Total vs Cycle Time (min) #beta-gated; Mtas Total Energy (keV); Cycle Time (min)",this->h2dsettings.at(3362));
	hismanager->RegisterPlot<TH2F>("MEASURE_3363","Mtas Total vs Cycle Time (hr) #beta-gated; Mtas Total Energy (keV); Cycle Time (hr)",this->h2dsettings.at(3363));

	hismanager->RegisterPlot<TH2F>("MEASURE_31608","Mtas Total vs Cycle Time (ms) anti-#beta-gated; Mtas Total Energy (8 keV/bin); Cycle Time (ms)",this->h2dsettings.at(31608));
	hismanager->RegisterPlot<TH2F>("MEASURE_31618","Mtas Total vs Cycle Time (s) anti-#beta-gated; Mtas Total Energy (8 keV/bin); Cycle Time (s)",this->h2dsettings.at(31618));
	hismanager->RegisterPlot<TH2F>("MEASURE_31628","Mtas Total vs Cycle Time (min) anti-#beta-gated; Mtas Total Energy (8 keV/bin); Cycle Time (min)",this->h2dsettings.at(31628));
	hismanager->RegisterPlot<TH2F>("MEASURE_31638","Mtas Total vs Cycle Time (hr) anti-#beta-gated; Mtas Total Energy (8 keV/bin); Cycle Time (hr)",this->h2dsettings.at(31638));

	hismanager->RegisterPlot<TH2F>("MEASURE_32608","Mtas Total vs Cycle Time (ms); Mtas Total Energy (8 keV/bin); Cycle Time (ms)",this->h2dsettings.at(32608));
	hismanager->RegisterPlot<TH2F>("MEASURE_32618","Mtas Total vs Cycle Time (s); Mtas Total Energy (8 keV/bin); Cycle Time (s)",this->h2dsettings.at(32618));
	hismanager->RegisterPlot<TH2F>("MEASURE_32628","Mtas Total vs Cycle Time (min); Mtas Total Energy (8 keV/bin); Cycle Time (min)",this->h2dsettings.at(32628));
	hismanager->RegisterPlot<TH2F>("MEASURE_32638","Mtas Total vs Cycle Time (hr); Mtas Total Energy (8 keV/bin); Cycle Time (hr)",this->h2dsettings.at(32638));

	hismanager->RegisterPlot<TH2F>("MEASURE_33608","Mtas Total vs Cycle Time (ms) #beta-gated; Mtas Total Energy (8 keV/bin); Cycle Time (ms)",this->h2dsettings.at(33608));
	hismanager->RegisterPlot<TH2F>("MEASURE_33618","Mtas Total vs Cycle Time (s) #beta-gated; Mtas Total Energy (8 keV/bin); Cycle Time (s)",this->h2dsettings.at(33618));
	hismanager->RegisterPlot<TH2F>("MEASURE_33628","Mtas Total vs Cycle Time (min) #beta-gated; Mtas Total Energy (8 keV/bin); Cycle Time (min)",this->h2dsettings.at(33628));
	hismanager->RegisterPlot<TH2F>("MEASURE_33638","Mtas Total vs Cycle Time (hr) #beta-gated; Mtas Total Energy (8 keV/bin); Cycle Time (hr)",this->h2dsettings.at(33638));

	hismanager->RegisterPlot<TH1F>("CYCLE_1000","Cycle Time; Cycle Time (ms);",this->h1dsettings.at(1000));
	hismanager->RegisterPlot<TH1F>("CYCLE_1001","Cycle Time; Cycle Time (s);",this->h1dsettings.at(1001));
	hismanager->RegisterPlot<TH1F>("CYCLE_1002","Cycle Time; Cycle Time (min);",this->h1dsettings.at(1002));
	hismanager->RegisterPlot<TH1F>("CYCLE_1003","Cycle Time; Cycle Time (hr);",this->h1dsettings.at(1003));

	hismanager->RegisterPlot<TH1F>("EARLY_1010","Early Cycle Gate; Value (arb.)",this->h1dsettings.at(1010));
	hismanager->RegisterPlot<TH1F>("MID_1010","Mid Cycle Gate; Value (arb.)",this->h1dsettings.at(1010));
	hismanager->RegisterPlot<TH1F>("LATE_1010","Late Cycle Gate; Value (arb.)",this->h1dsettings.at(1010));

	hismanager->RegisterPlot<TH1F>("EARLY_3300","Mtas Total #beta-gated Early Cycle Time",this->h1dsettings.at(3300));
	hismanager->RegisterPlot<TH1F>("MID_3300","Mtas Total #beta-gated MID Cycle Time",this->h1dsettings.at(3300));
	hismanager->RegisterPlot<TH1F>("LATE_3300","Mtas Total #beta-gated LATE Cycle Time",this->h1dsettings.at(3300));

	hismanager->RegisterPlot<TH2F>("EARLY_3350","I,M,O vs Mtas Total #beta-gated Early Cycle Time",this->h2dsettings.at(3350));
	hismanager->RegisterPlot<TH2F>("MID_3350","I,M,O vs Mtas Total #beta-gated MID Cycle Time",this->h2dsettings.at(3350));
	hismanager->RegisterPlot<TH2F>("LATE_3350","I,M,O vs Mtas Total #beta-gated LATE Cycle Time",this->h2dsettings.at(3350));

	hismanager->RegisterPlot<TH2F>("EARLY_33508","I,M,O vs Mtas Total #beta-gated Early Cycle Time; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33508));
	hismanager->RegisterPlot<TH2F>("MID_33508","I,M,O vs Mtas Total #beta-gated MID Cycle Time; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33508));
	hismanager->RegisterPlot<TH2F>("LATE_33508","I,M,O vs Mtas Total #beta-gated LATE Cycle Time; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33508));

	hismanager->RegisterPlot<TH2F>("EARLY_3351","C vs Mtas Total #beta-gated Early Cycle Time",this->h2dsettings.at(3351));
	hismanager->RegisterPlot<TH2F>("MID_3351","C vs Mtas Total #beta-gated MID Cycle Time",this->h2dsettings.at(3351));
	hismanager->RegisterPlot<TH2F>("LATE_3351","C vs Mtas Total #beta-gated LATE Cycle Time",this->h2dsettings.at(3351));

	hismanager->RegisterPlot<TH2F>("EARLY_33518","C vs Mtas Total #beta-gated Early Cycle Time; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33518));
	hismanager->RegisterPlot<TH2F>("MID_33518","C vs Mtas Total #beta-gated MID Cycle Time; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33518));
	hismanager->RegisterPlot<TH2F>("LATE_33518","C vs Mtas Total #beta-gated LATE Cycle Time; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33518));


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
