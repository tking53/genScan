#include "anl2021Processor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "TapeCycle.hpp"
#include <TTree.h>
#include <stdexcept>
#include <string>

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
		{3110,{16384,0,16384}},
		{3120,{16384,0,16384}},
		{3130,{16384,0,16384}},
		{3140,{16384,0,16384}},
		{3115,{16384,0,16384}},
		{3125,{16384,0,16384}},
		{3135,{16384,0,16384}},
		{3145,{16384,0,16384}},

		{3200,{16384,0,16384}},
		{3210,{16384,0,16384}},
		{3220,{16384,0,16384}},
		{3230,{16384,0,16384}},
		{3240,{16384,0,16384}},
		{3215,{16384,0,16384}},
		{3225,{16384,0,16384}},
		{3235,{16384,0,16384}},
		{3245,{16384,0,16384}},

		{3300,{16384,0,16384}},
		{3310,{16384,0,16384}},
		{3320,{16384,0,16384}},
		{3330,{16384,0,16384}},
		{3340,{16384,0,16384}},
		{3315,{16384,0,16384}},
		{3325,{16384,0,16384}},
		{3335,{16384,0,16384}},
		{3345,{16384,0,16384}},
	
		{3730,{16384,0,16384}}

	};

	this->h2dsettings = {
		{2100,{8192,0,8192,4,0,4}},

		{2160,{8192,0,8192,512,0,512}},
		{2161,{8192,0,8192,512,0,512}},
		{2162,{8192,0,8192,512,0,512}},
		{2163,{8192,0,8192,512,0,512}},

		{2200,{8192,0,8192,4,0,4}},

		{2260,{8192,0,8192,512,0,512}},
		{2261,{8192,0,8192,512,0,512}},
		{2262,{8192,0,8192,512,0,512}},
		{2263,{8192,0,8192,512,0,512}},

		{2300,{8192,0,8192,4,0,4}},

		{2360,{8192,0,8192,512,0,512}},
		{2361,{8192,0,8192,512,0,512}},
		{2362,{8192,0,8192,512,0,512}},
		{2363,{8192,0,8192,512,0,512}},

		{2500,{4096,0,4096,4096,0,4096}},
		{2600,{4096,0,4096,4096,0,4096}},
		{2700,{4096,0,4096,4096,0,4096}},

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

		{3411, {8192,0.0,8192.0,12,0,12}},
		{3412, {8192,0.0,8192.0,12,0,12}},
		{3413, {8192,0.0,8192.0,12,0,12}},
		{3414, {8192,0.0,8192.0,12,0,12}},

		{3500,{8192,0,8192,1000,0,10000}},
		{3501,{8192,0,8192,1000,0,10000}},
		{3502,{8192,0,8192,1000,0,10000}},
		{3503,{8192,0,8192,1000,0,10000}},
		{3504,{8192,0,8192,1000,0,10000}},
		{3505,{8192,0,8192,1000,0,10000}},

		{3511, {8192,0.0,8192.0,12,0,12}},
		{3512, {8192,0.0,8192.0,12,0,12}},
		{3513, {8192,0.0,8192.0,12,0,12}},
		{3514, {8192,0.0,8192.0,12,0,12}},
	
		{3600,{8192,0,8192,1000,0,10000}},
		{3601,{8192,0,8192,1000,0,10000}},
		{3602,{8192,0,8192,1000,0,10000}},
		{3603,{8192,0,8192,1000,0,10000}},
		{3604,{8192,0,8192,1000,0,10000}},
		{3605,{8192,0,8192,1000,0,10000}},

		{3700,{8192,0,8192,1000,0,10000}},
		{3701,{8192,0,8192,1000,0,10000}},
		{3702,{8192,0,8192,1000,0,10000}},
		{3703,{8192,0,8192,1000,0,10000}},
		{3704,{8192,0,8192,1000,0,10000}},
		{3705,{8192,0,8192,1000,0,10000}},

		{3800,{8192,0,8192,1000,0,10000}},
		{3801,{8192,0,8192,1000,0,10000}},
		{3802,{8192,0,8192,1000,0,10000}},
		{3803,{8192,0,8192,1000,0,10000}},
		{3804,{8192,0,8192,1000,0,10000}},
		{3805,{8192,0,8192,1000,0,10000}},

		{3900,{8192,0,8192,1000,0,1000}},

		{4500,{8192,0,8192,1000,0,10000}},
		{4501,{8192,0,8192,1000,0,10000}},
	
		{4600,{8192,0,8192,1000,0,10000}},
		{4601,{8192,0,8192,1000,0,10000}},

		{4700,{8192,0,8192,1000,0,10000}},
		{4701,{8192,0,8192,1000,0,10000}},

		{4800,{8192,0,8192,1000,0,10000}},
		{4801,{8192,0,8192,1000,0,10000}}

	};

	this->implant = "implant";
	this->hpge = "hpge";
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
	this->HPGeThreshold = 0.0;

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
			summary->AddEventObservable("LGAnodeSum",lgImage.anodesum);
		}
	}

	if( this->HasHPGe ){
		this->HPGeProc->PreProcess(eventhistory,hismanager,cutmanager);
		for( auto ii = 0; ii < this->HPGeProc->GetNumCrystals(); ++ii ){
			auto erg = this->HPGeProc->GetEnergy(ii);
			if( erg > this->HPGeThreshold ){
				summary->AddEventTag(this->hpge);
				summary->AddEventObservable("HPGe_"+std::to_string(ii),erg);
			}
		}
	}

	if( this->HasSilicon ){
		this->SiliconProc->PreProcess(eventhistory,hismanager,cutmanager);
		auto simax = this->SiliconProc->GetMaxEnergy();
		if( simax > this->SiliconThreshold ){
			summary->AddEventTag(this->beta);
			summary->AddEventObservable("SiMax",simax);
		}
	}

	if( this->HasMTAS ){
		this->MtasProc->PreProcess(eventhistory,hismanager,cutmanager);
		//mtas already adds its own observables and tags
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

			auto internaltdiff = (this->MtasProc->GetLastFireTime() - this->SiliconProc->GetFirstFireTime());
			hismanager->Fill("ISOMER_3900",erg,internaltdiff);

			if( hasbeta ){
				if( this->EarlyCycle.IsWithin(cycletime) ){
					hismanager->Fill("EARLY_3300",erg);
					hismanager->Fill("EARLY_3351",erg,cerg);
					for( size_t ii = 6; ii < 24; ++ii ){
						hismanager->Fill("EARLY_3350",MtasProc->GetCrystalEnergy(ii),erg);
					}
				}
				if( this->MidCycle.IsWithin(cycletime) ){
					hismanager->Fill("MID_3300",erg);
					hismanager->Fill("MID_3351",erg,cerg);
					for( size_t ii = 6; ii < 24; ++ii ){
						hismanager->Fill("MID_3350",MtasProc->GetCrystalEnergy(ii),erg);
					}
				}
				if( this->LateCycle.IsWithin(cycletime) ){
					hismanager->Fill("LATE_3300",erg);
					hismanager->Fill("LATE_3351",erg,cerg);
					for( size_t ii = 6; ii < 24; ++ii ){
						hismanager->Fill("LATE_3350",MtasProc->GetCrystalEnergy(ii),erg);
					}
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
				if( not hasbeta ){
					for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
						auto prevsummary =  eventhistory->GetPreviousEventSummary(ii);
						auto prevmuon = prevsummary->ContainsEventTag(this->muon);
						if( prevmuon ){
							continue;
						}
						auto prevbeta = prevsummary->ContainsEventTag(this->beta);
						auto prevgamma = prevsummary->ContainsEventTag(this->gamma);
						//this looks for a beta decay into a delayed level
						//like 137Cs
						if( prevbeta ){
							auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
							hismanager->Fill("ISOMER_3700",erg,isomer_tdiff);
							hismanager->Fill("ISOMER_3701",erg,isomer_tdiff*1.0e-3);

							auto olderg = prevsummary->GetEventObservable("MTAS_Total").value_or(0.0);
							hismanager->Fill("ISOMER_3702",olderg,isomer_tdiff);
							hismanager->Fill("ISOMER_3703",olderg,isomer_tdiff*1.0e-3);
							//trying to find how we got into this level
							for( size_t ii = 0; ii < this->ISOMER_3701_Gates.size(); ++ii ){
								if( this->ISOMER_3701_Gates.at(ii).IsWithin(erg,isomer_tdiff*1.0e-3) ){
									std::string label = "ISOMER_373"+std::to_string(ii);
									hismanager->Fill(label,olderg);
								}
							}

							hismanager->Fill("ISOMER_3704",erg+olderg,isomer_tdiff);
							hismanager->Fill("ISOMER_3705",erg+olderg,isomer_tdiff*1.0e-3);
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
						//looking for stepping through short isomer after we start in daughter isomer
						if( not prevbeta ){
							auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
							hismanager->Fill("ISOMER_3600",erg,isomer_tdiff);
							hismanager->Fill("ISOMER_3601",erg,isomer_tdiff*1.0e-3);
							auto olderg = prevsummary->GetEventObservable("MTAS_Total").value_or(0.0);
							hismanager->Fill("ISOMER_3602",olderg,isomer_tdiff);
							hismanager->Fill("ISOMER_3603",olderg,isomer_tdiff*1.0e-3);
							
							hismanager->Fill("ISOMER_3604",erg+olderg,isomer_tdiff);
							hismanager->Fill("ISOMER_3605",erg+olderg,isomer_tdiff*1.0e-3);
							break;
						}
					}
				}
				//current event does not have beta 
				if( hasbeta ){
					for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
						auto prevsummary =  eventhistory->GetPreviousEventSummary(ii);
						auto prevmuon = prevsummary->ContainsEventTag(this->muon);
						if( prevmuon ){
							continue;
						}
						auto prevbeta = prevsummary->ContainsEventTag(this->beta);
						auto prevgamma = prevsummary->ContainsEventTag(this->gamma);

						//this looks for a gamma decay into a delayed beta
						//i.e. beam isomer, but need mtas energy for this old event
						if( not prevbeta ){
							auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
							hismanager->Fill("ISOMER_3800",erg,isomer_tdiff);
							hismanager->Fill("ISOMER_3801",erg,isomer_tdiff*1.0e-3);
							auto olderg = prevsummary->GetEventObservable("MTAS_Total").value_or(0.0);
							hismanager->Fill("ISOMER_3802",olderg,isomer_tdiff);
							hismanager->Fill("ISOMER_3803",olderg,isomer_tdiff*1.0e-3);
							
							hismanager->Fill("ISOMER_3804",erg+olderg,isomer_tdiff);
							hismanager->Fill("ISOMER_3805",erg+olderg,isomer_tdiff*1.0e-3);
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

						//this looks for a gamma decay into a delayed beta
						//i.e. beam isomer, but need mtas energy for this old event
						if( prevbeta ){
							auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
							hismanager->Fill("ISOMER_3500",erg,isomer_tdiff);
							hismanager->Fill("ISOMER_3501",erg,isomer_tdiff*1.0e-3);
							auto olderg = prevsummary->GetEventObservable("MTAS_Total").value_or(0.0);
							hismanager->Fill("ISOMER_3502",olderg,isomer_tdiff);
							hismanager->Fill("ISOMER_3503",olderg,isomer_tdiff*1.0e-3);
							
							hismanager->Fill("ISOMER_3504",erg+olderg,isomer_tdiff);
							hismanager->Fill("ISOMER_3505",erg+olderg,isomer_tdiff*1.0e-3);
							break;
						}
					}
				}
			}

		}else if( TapeProc->GetCurrentCycleState() == TAPE::BACKGROUND ){
			if( not this->MtasProc->DidAnyPileup() and not this->MtasProc->DidAnySaturate() ){
				hismanager->Fill("BKG_3200",this->MtasProc->GetTotalEnergy(0));
				hismanager->Fill("BKG_3210",this->MtasProc->GetTotalEnergy(1));
				hismanager->Fill("BKG_3220",this->MtasProc->GetTotalEnergy(2));
				hismanager->Fill("BKG_3230",this->MtasProc->GetTotalEnergy(3));
				hismanager->Fill("BKG_3240",this->MtasProc->GetTotalEnergy(4));
				for( size_t ii = 0; ii < 6; ++ii ){
					hismanager->Fill("BKG_3215",this->MtasProc->GetCrystalEnergy(ii));
					hismanager->Fill("BKG_3225",this->MtasProc->GetCrystalEnergy(ii+6));
					hismanager->Fill("BKG_3235",this->MtasProc->GetCrystalEnergy(ii+12));
					hismanager->Fill("BKG_3245",this->MtasProc->GetCrystalEnergy(ii+18));
				}
				if( summary->ContainsEventTag(this->beta) ){
					hismanager->Fill("BKG_3300",this->MtasProc->GetTotalEnergy(0));
					hismanager->Fill("BKG_3310",this->MtasProc->GetTotalEnergy(1));
					hismanager->Fill("BKG_3320",this->MtasProc->GetTotalEnergy(2));
					hismanager->Fill("BKG_3330",this->MtasProc->GetTotalEnergy(3));
					hismanager->Fill("BKG_3340",this->MtasProc->GetTotalEnergy(4));
					for( size_t ii = 0; ii < 6; ++ii ){
						hismanager->Fill("BKG_3315",this->MtasProc->GetCrystalEnergy(ii));
						hismanager->Fill("BKG_3325",this->MtasProc->GetCrystalEnergy(ii+6));
						hismanager->Fill("BKG_3335",this->MtasProc->GetCrystalEnergy(ii+12));
						hismanager->Fill("BKG_3345",this->MtasProc->GetCrystalEnergy(ii+18));
					}
				}else{
					hismanager->Fill("BKG_3100",this->MtasProc->GetTotalEnergy(0));
					hismanager->Fill("BKG_3110",this->MtasProc->GetTotalEnergy(1));
					hismanager->Fill("BKG_3120",this->MtasProc->GetTotalEnergy(2));
					hismanager->Fill("BKG_3130",this->MtasProc->GetTotalEnergy(3));
					hismanager->Fill("BKG_3140",this->MtasProc->GetTotalEnergy(4));
					for( size_t ii = 0; ii < 6; ++ii ){
						hismanager->Fill("BKG_3115",this->MtasProc->GetCrystalEnergy(ii));
						hismanager->Fill("BKG_3125",this->MtasProc->GetCrystalEnergy(ii+6));
						hismanager->Fill("BKG_3135",this->MtasProc->GetCrystalEnergy(ii+12));
						hismanager->Fill("BKG_3145",this->MtasProc->GetCrystalEnergy(ii+18));
					}
				}

				for( int ii = 0; ii < 6; ++ii ){
					hismanager->Fill("BKG_3411",this->MtasProc->GetIndividualCenterPMTRawEnergy(2*ii),2*ii);
					hismanager->Fill("BKG_3411",this->MtasProc->GetIndividualCenterPMTRawEnergy(2*ii + 1),2*ii + 1);

					hismanager->Fill("BKG_3412",this->MtasProc->GetIndividualInnerPMTRawEnergy(2*ii),2*ii);
					hismanager->Fill("BKG_3412",this->MtasProc->GetIndividualInnerPMTRawEnergy(2*ii + 1),2*ii + 1);

					hismanager->Fill("BKG_3413",this->MtasProc->GetIndividualMiddlePMTRawEnergy(2*ii),2*ii);
					hismanager->Fill("BKG_3413",this->MtasProc->GetIndividualMiddlePMTRawEnergy(2*ii + 1),2*ii + 1);

					hismanager->Fill("BKG_3414",this->MtasProc->GetIndividualOuterPMTRawEnergy(2*ii),2*ii);
					hismanager->Fill("BKG_3414",this->MtasProc->GetIndividualOuterPMTRawEnergy(2*ii + 1),2*ii + 1);

					hismanager->Fill("BKG_3511",this->MtasProc->GetIndividualCenterPMTEnergy(2*ii),2*ii);
					hismanager->Fill("BKG_3511",this->MtasProc->GetIndividualCenterPMTEnergy(2*ii + 1),2*ii + 1);

					hismanager->Fill("BKG_3512",this->MtasProc->GetIndividualInnerPMTEnergy(2*ii),2*ii);
					hismanager->Fill("BKG_3512",this->MtasProc->GetIndividualInnerPMTEnergy(2*ii + 1),2*ii + 1);

					hismanager->Fill("BKG_3513",this->MtasProc->GetIndividualMiddlePMTEnergy(2*ii),2*ii);
					hismanager->Fill("BKG_3513",this->MtasProc->GetIndividualMiddlePMTEnergy(2*ii + 1),2*ii + 1);

					hismanager->Fill("BKG_3514",this->MtasProc->GetIndividualOuterPMTEnergy(2*ii),2*ii);
					hismanager->Fill("BKG_3514",this->MtasProc->GetIndividualOuterPMTEnergy(2*ii + 1),2*ii + 1);
				}
			}
		}else if( TapeProc->GetCurrentCycleState() == TAPE::IRRADIATION ){
			bool hashpge = summary->ContainsEventTag(this->hpge);
			bool hasimplant = summary->ContainsEventTag(this->implant);

			if( numhist > 1 ){
				//current event has gamma not-muon, not-beta
				if( not hasimplant ){
					for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
						auto prevsummary = eventhistory->GetPreviousEventSummary(ii);
						auto prevmuon = prevsummary->ContainsEventTag(this->muon);
						if( prevmuon ){
							continue;
						}
						auto previmplant = prevsummary->ContainsEventTag(this->implant);
						auto prevhpge = prevsummary->ContainsEventTag(this->hpge);
						//this looks for a beta decay into a delayed level
						//like 137Cs
						if( previmplant ){
							auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
							for( size_t jj = 0; jj < this->HPGeProc->GetNumCrystals(); ++jj ){
								auto hpge_erg = this->HPGeProc->GetEnergy(jj);
								hismanager->Fill("ISOMER_4700",hpge_erg,isomer_tdiff);
								hismanager->Fill("ISOMER_4701",hpge_erg,isomer_tdiff*1.0e-3);
							}
							break;
						}
					}
					for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
						auto prevsummary =  eventhistory->GetPreviousEventSummary(ii);
						auto prevmuon = prevsummary->ContainsEventTag(this->muon);
						if( prevmuon ){
							continue;
						}
						auto previmplant = prevsummary->ContainsEventTag(this->implant);
						auto prevhpge = prevsummary->ContainsEventTag(this->hpge);
						//looking for stepping through short isomer after we start in daughter isomer
						if( not previmplant ){
							auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();

							for( size_t jj = 0; jj < this->HPGeProc->GetNumCrystals(); ++jj ){
								auto hpge_erg = this->HPGeProc->GetEnergy(jj);
								hismanager->Fill("ISOMER_4600",hpge_erg,isomer_tdiff);
								hismanager->Fill("ISOMER_4601",hpge_erg,isomer_tdiff*1.0e-3);
							}
							break;
						}
					}
				}
				//current event does not have beta 
				if( hasimplant ){
					for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
						auto prevsummary =  eventhistory->GetPreviousEventSummary(ii);
						auto prevmuon = prevsummary->ContainsEventTag(this->muon);
						if( prevmuon ){
							continue;
						}
						auto previmplant = prevsummary->ContainsEventTag(this->implant);
						auto prevhpge = prevsummary->ContainsEventTag(this->hpge);

						//this looks for a gamma decay into a delayed beta
						//i.e. beam isomer, but need mtas energy for this old event
						if( not previmplant ){
							auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
							for( size_t jj = 0; jj < this->HPGeProc->GetNumCrystals(); ++jj ){
								auto hpge_erg = this->HPGeProc->GetEnergy(jj);
								hismanager->Fill("ISOMER_4800",hpge_erg,isomer_tdiff);
								hismanager->Fill("ISOMER_4801",hpge_erg,isomer_tdiff*1.0e-3);
							}
							break;
						}
					}
					for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
						auto prevsummary =  eventhistory->GetPreviousEventSummary(ii);
						auto prevmuon = prevsummary->ContainsEventTag(this->muon);
						if( prevmuon ){
							continue;
						}
						auto previmplant = prevsummary->ContainsEventTag(this->implant);
						auto prevhpge = prevsummary->ContainsEventTag(this->hpge);

						//this looks for a gamma decay into a delayed beta
						//i.e. beam isomer, but need mtas energy for this old event
						if( previmplant ){
							auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
							for( size_t jj = 0; jj < this->HPGeProc->GetNumCrystals(); ++jj ){
								auto hpge_erg = this->HPGeProc->GetEnergy(jj);
								hismanager->Fill("ISOMER_4500",hpge_erg,isomer_tdiff);
								hismanager->Fill("ISOMER_4501",hpge_erg,isomer_tdiff*1.0e-3);
							}
							break;
						}
					}
				}
			}
			//add in HPGe monitor
			for( auto ii = 0; ii < this->HPGeProc->GetNumCrystals(); ++ii ){
				auto hpge_erg = this->HPGeProc->GetEnergy(ii);
				hismanager->Fill("IRRAD_2200",hpge_erg,ii);
				for( auto jj = ii+1; jj < this->HPGeProc->GetNumCrystals(); ++jj ){
					hismanager->Fill("IRRAD_2600",hpge_erg,this->HPGeProc->GetEnergy(jj));
					hismanager->Fill("IRRAD_2600",this->HPGeProc->GetEnergy(jj),hpge_erg);
				}
				hismanager->Fill("IRRAD_2260",hpge_erg,cycletime*1.0e3);
				hismanager->Fill("IRRAD_2261",hpge_erg,cycletime);
				hismanager->Fill("IRRAD_2262",hpge_erg,cycletime/60.0);
				hismanager->Fill("IRRAD_2263",hpge_erg,cycletime/(60.0*60.0));
			}

			
			//this is the nose implant plastic, was either 2x1 or 2x2
			//this is the logic that makes PSPMT_1902 show up, use it here too
			auto hgimage = this->ImplantProc->GetHighGainImage();
			if( hgimage.anodesum > this->ImplantThreshold ){
				for( auto ii = 0; ii < this->HPGeProc->GetNumCrystals(); ++ii ){
					auto hpge_erg = this->HPGeProc->GetEnergy(ii);
					hismanager->Fill("IRRAD_2300",hpge_erg,ii);
					for( auto jj = ii+1; jj < this->HPGeProc->GetNumCrystals(); ++jj ){
						hismanager->Fill("IRRAD_2700",hpge_erg,this->HPGeProc->GetEnergy(jj));
						hismanager->Fill("IRRAD_2700",this->HPGeProc->GetEnergy(jj),hpge_erg);
					}
					hismanager->Fill("IRRAD_2360",hpge_erg,cycletime*1.0e3);
					hismanager->Fill("IRRAD_2361",hpge_erg,cycletime);
					hismanager->Fill("IRRAD_2362",hpge_erg,cycletime/60.0);
					hismanager->Fill("IRRAD_2363",hpge_erg,cycletime/(60.0*60.0));
				}
			}else{
				for( auto ii = 0; ii < this->HPGeProc->GetNumCrystals(); ++ii ){
					auto hpge_erg = this->HPGeProc->GetEnergy(ii);
					hismanager->Fill("IRRAD_2100",hpge_erg,ii);
					for( auto jj = ii+1; jj < this->HPGeProc->GetNumCrystals(); ++jj ){
						hismanager->Fill("IRRAD_2500",hpge_erg,this->HPGeProc->GetEnergy(jj));
						hismanager->Fill("IRRAD_2500",this->HPGeProc->GetEnergy(jj),hpge_erg);
					}
					hismanager->Fill("IRRAD_2160",hpge_erg,cycletime*1.0e3);
					hismanager->Fill("IRRAD_2161",hpge_erg,cycletime);
					hismanager->Fill("IRRAD_2162",hpge_erg,cycletime/60.0);
					hismanager->Fill("IRRAD_2163",hpge_erg,cycletime/(60.0*60.0));
				}
			}
			
			//this is the diagnostic cross implant plastic, was always a 2x2
			//auto lgimage = this->ImplantProc->GetLowGainImage();
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
	this->HPGeThreshold = config.attribute("hpgethresh").as_double(0.0);
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

	for( pugi::xml_node boxgate = config.child("BoxGate"); boxgate; boxgate = boxgate.next_sibling("BoxGate") ){
		std::string label = boxgate.attribute("label").as_string("");
		if( label.compare("ISOMER_3701") == 0 ){
			auto xlow = boxgate.attribute("xlowerbound").as_double(0.0);
			auto xhigh = boxgate.attribute("xupperbound").as_double(16384.0);
			auto ylow = boxgate.attribute("ylowerbound").as_double(0.0);
			auto yhigh = boxgate.attribute("yupperbound").as_double(16384.0);
			this->ISOMER_3701_Gates.push_back(BoxGate<double>(xlow,xhigh,ylow,yhigh));
		}
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

	hismanager->RegisterPlot<TH1F>("BKG_3100","Mtas Total Background Cycle Gated anti-#beta Gated; Energy (keV)",this->h1dsettings.at(3100));
	hismanager->RegisterPlot<TH1F>("BKG_3110","Mtas Center Sum Background Cycle Gated anti-#beta Gated; Energy (keV)",this->h1dsettings.at(3110));
	hismanager->RegisterPlot<TH1F>("BKG_3120","Mtas Inner Sum Background Cycle Gated anti-#beta Gated; Energy (keV)",this->h1dsettings.at(3120));
	hismanager->RegisterPlot<TH1F>("BKG_3130","Mtas Middle Sum Background Cycle Gated anti-#beta Gated; Energy (keV)",this->h1dsettings.at(3130));
	hismanager->RegisterPlot<TH1F>("BKG_3140","Mtas Outer Sum Background Cycle Gated anti-#beta Gated; Energy (keV)",this->h1dsettings.at(3140));
	hismanager->RegisterPlot<TH1F>("BKG_3115","Mtas Center Stack Background Cycle Gated anti-#beta Gated; Energy (keV)",this->h1dsettings.at(3115));
	hismanager->RegisterPlot<TH1F>("BKG_3125","Mtas Inner Stack Background Cycle Gated anti-#beta Gated; Energy (keV)",this->h1dsettings.at(3125));
	hismanager->RegisterPlot<TH1F>("BKG_3135","Mtas Middle Stack Background Cycle Gated anti-#beta Gated; Energy (keV)",this->h1dsettings.at(3135));
	hismanager->RegisterPlot<TH1F>("BKG_3145","Mtas Outer Stack Background Cycle Gated anti-#beta Gated; Energy (keV)",this->h1dsettings.at(3145));

	hismanager->RegisterPlot<TH1F>("BKG_3200","Mtas Total Background Cycle Gated; Energy (keV)",this->h1dsettings.at(3200));
	hismanager->RegisterPlot<TH1F>("BKG_3210","Mtas Center Sum Background Cycle Gated; Energy (keV)",this->h1dsettings.at(3210));
	hismanager->RegisterPlot<TH1F>("BKG_3220","Mtas Inner Sum Background Cycle Gated; Energy (keV)",this->h1dsettings.at(3220));
	hismanager->RegisterPlot<TH1F>("BKG_3230","Mtas Middle Sum Background Cycle Gated; Energy (keV)",this->h1dsettings.at(3230));
	hismanager->RegisterPlot<TH1F>("BKG_3240","Mtas Outer Sum Background Cycle Gated; Energy (keV)",this->h1dsettings.at(3240));
	hismanager->RegisterPlot<TH1F>("BKG_3215","Mtas Center Stack Background Cycle Gated; Energy (keV)",this->h1dsettings.at(3215));
	hismanager->RegisterPlot<TH1F>("BKG_3225","Mtas Inner Stack Background Cycle Gated; Energy (keV)",this->h1dsettings.at(3225));
	hismanager->RegisterPlot<TH1F>("BKG_3235","Mtas Middle Stack Background Cycle Gated; Energy (keV)",this->h1dsettings.at(3235));
	hismanager->RegisterPlot<TH1F>("BKG_3245","Mtas Outer Stack Background Cycle Gated; Energy (keV)",this->h1dsettings.at(3245));

	hismanager->RegisterPlot<TH1F>("BKG_3300","Mtas Total Background Cycle Gated #beta Gated; Energy (keV)",this->h1dsettings.at(3300));
	hismanager->RegisterPlot<TH1F>("BKG_3310","Mtas Center Sum Background Cycle Gated #beta Gated; Energy (keV)",this->h1dsettings.at(3310));
	hismanager->RegisterPlot<TH1F>("BKG_3320","Mtas Inner Sum Background Cycle Gated #beta Gated; Energy (keV)",this->h1dsettings.at(3320));
	hismanager->RegisterPlot<TH1F>("BKG_3330","Mtas Middle Sum Background Cycle Gated #beta Gated; Energy (keV)",this->h1dsettings.at(3330));
	hismanager->RegisterPlot<TH1F>("BKG_3340","Mtas Outer Sum Background Cycle Gated #beta Gated; Energy (keV)",this->h1dsettings.at(3340));
	hismanager->RegisterPlot<TH1F>("BKG_3315","Mtas Center Stack Background Cycle Gated #beta Gated; Energy (keV)",this->h1dsettings.at(3315));
	hismanager->RegisterPlot<TH1F>("BKG_3325","Mtas Inner Stack Background Cycle Gated #beta Gated; Energy (keV)",this->h1dsettings.at(3325));
	hismanager->RegisterPlot<TH1F>("BKG_3335","Mtas Middle Stack Background Cycle Gated #beta Gated; Energy (keV)",this->h1dsettings.at(3335));
	hismanager->RegisterPlot<TH1F>("BKG_3345","Mtas Outer Stack Background Cycle Gated #beta Gated; Energy (keV)",this->h1dsettings.at(3345));

	hismanager->RegisterPlot<TH2F>("BKG_3411","Raw IndividualPMT C PMTs Background Cycle gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3411));
	hismanager->RegisterPlot<TH2F>("BKG_3412","Raw IndividualPMT I PMTs Background Cycle gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3412));
	hismanager->RegisterPlot<TH2F>("BKG_3413","Raw IndividualPMT M PMTs Background Cycle gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3413));
	hismanager->RegisterPlot<TH2F>("BKG_3414","Raw IndividualPMT O PMTs Background Cycle gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3414));

	hismanager->RegisterPlot<TH2F>("BKG_3511","Calibrated IndividualPMT C PMTs Background Cycle gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3511));
	hismanager->RegisterPlot<TH2F>("BKG_3512","Calibrated IndividualPMT I PMTs Background Cycle gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3512));
	hismanager->RegisterPlot<TH2F>("BKG_3513","Calibrated IndividualPMT M PMTs Background Cycle gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3513));
	hismanager->RegisterPlot<TH2F>("BKG_3514","Calibrated IndividualPMT O PMTs Background Cycle gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3514));

	hismanager->RegisterPlot<TH2F>("IRRAD_2100","HPGe Irradiation Cycle Gated anti-#beta Gated; Energy (keV); Crystal Number (arb.)",this->h2dsettings.at(2100));
	hismanager->RegisterPlot<TH2F>("IRRAD_2160","HPGe vs Cycle Time (ms) anti-#beta-gated; Energy (keV); Cycle Time (ms)",this->h2dsettings.at(2160));
	hismanager->RegisterPlot<TH2F>("IRRAD_2161","HPGe vs Cycle Time (s) anti-#beta-gated; Energy (keV); Cycle Time (s)",this->h2dsettings.at(2161));
	hismanager->RegisterPlot<TH2F>("IRRAD_2162","HPGe vs Cycle Time (min) anti-#beta-gated; Energy (keV); Cycle Time (min)",this->h2dsettings.at(2162));
	hismanager->RegisterPlot<TH2F>("IRRAD_2163","HPGe vs Cycle Time (hr) anti-#beta-gated; Energy (keV); Cycle Time (hr)",this->h2dsettings.at(2163));

	hismanager->RegisterPlot<TH2F>("IRRAD_2200","HPGe Irradiation Cycle Gated; Energy (keV); Crystal Number (arb.)",this->h2dsettings.at(2200));
	hismanager->RegisterPlot<TH2F>("IRRAD_2260","HPGe vs Cycle Time (ms); Energy (keV); Cycle Time (ms)",this->h2dsettings.at(2260));
	hismanager->RegisterPlot<TH2F>("IRRAD_2261","HPGe vs Cycle Time (s); Energy (keV); Cycle Time (s)",this->h2dsettings.at(2261));
	hismanager->RegisterPlot<TH2F>("IRRAD_2262","HPGe vs Cycle Time (min); Energy (keV); Cycle Time (min)",this->h2dsettings.at(2262));
	hismanager->RegisterPlot<TH2F>("IRRAD_2263","HPGe vs Cycle Time (hr); Energy (keV); Cycle Time (hr)",this->h2dsettings.at(2263));

	hismanager->RegisterPlot<TH2F>("IRRAD_2300","HPGe Irradiation Cycle Gated #beta Gated; Energy (keV); Crystal Number (arb.)",this->h2dsettings.at(2300));
	
	hismanager->RegisterPlot<TH2F>("IRRAD_2360","HPGe vs Cycle Time (ms) #beta-gated; Energy (keV); Cycle Time (ms)",this->h2dsettings.at(2360));
	hismanager->RegisterPlot<TH2F>("IRRAD_2361","HPGe vs Cycle Time (s) #beta-gated; Energy (keV); Cycle Time (s)",this->h2dsettings.at(2361));
	hismanager->RegisterPlot<TH2F>("IRRAD_2362","HPGe vs Cycle Time (min) #beta-gated; Energy (keV); Cycle Time (min)",this->h2dsettings.at(2362));
	hismanager->RegisterPlot<TH2F>("IRRAD_2363","HPGe vs Cycle Time (hr) #beta-gated; Energy (keV); Cycle Time (hr)",this->h2dsettings.at(2363));

	hismanager->RegisterPlot<TH2F>("IRRAD_2500","HPGe Gamma-Gamma Irradiation Cycle Gated anti-#beta Gated; Energy (keV); Energy (keV)",this->h2dsettings.at(2500));
	hismanager->RegisterPlot<TH2F>("IRRAD_2600","HPGe Gamma-Gamma Irradiation Cycle Gated; Energy (keV); Energy (keV)",this->h2dsettings.at(2600));
	hismanager->RegisterPlot<TH2F>("IRRAD_2700","HPGe Gamma-Gamma Irradiation Cycle Gated #beta Gated; Energy (keV); Energy (keV)",this->h2dsettings.at(2700));
	
	
	//Prev    | Curr    | His 
	//beta    | no-beta | 370X   
	//no-beta | beta    | 380X 
	//beta    | beta    | 350X
	//no-beta | no-beta | 360X 
	hismanager->RegisterPlot<TH2F>("ISOMER_3500","Mtas prev-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (ns)",this->h2dsettings.at(3500));
	hismanager->RegisterPlot<TH2F>("ISOMER_3501","Mtas prev-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (us)",this->h2dsettings.at(3501));
	hismanager->RegisterPlot<TH2F>("ISOMER_3502","Mtas prev-#beta curr-#beta Measure Cycle Gated; Prev Energy (keV); Time (ns)",this->h2dsettings.at(3502));
	hismanager->RegisterPlot<TH2F>("ISOMER_3503","Mtas prev-#beta curr-#beta Measure Cycle Gated; Prev Energy (keV); Time (us)",this->h2dsettings.at(3503));
	hismanager->RegisterPlot<TH2F>("ISOMER_3504","Mtas prev-#beta curr-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (ns)",this->h2dsettings.at(3504));
	hismanager->RegisterPlot<TH2F>("ISOMER_3505","Mtas prev-#beta curr-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (us)",this->h2dsettings.at(3505));
	
	hismanager->RegisterPlot<TH2F>("ISOMER_3600","Mtas prev-no-#beta curr-no-#beta Measure Cycle Gated; Curr Energy (keV); Time (ns)",this->h2dsettings.at(3600));
	hismanager->RegisterPlot<TH2F>("ISOMER_3601","Mtas prev-no-#beta curr-no-#beta Measure Cycle Gated; Curr Energy (keV); Time (us)",this->h2dsettings.at(3601));
	hismanager->RegisterPlot<TH2F>("ISOMER_3602","Mtas prev-no-#beta curr-no-#beta Measure Cycle Gated; Prev Energy (keV); Time (ns)",this->h2dsettings.at(3602));
	hismanager->RegisterPlot<TH2F>("ISOMER_3603","Mtas prev-no-#beta curr-no-#beta Measure Cycle Gated; Prev Energy (keV); Time (us)",this->h2dsettings.at(3603));
	hismanager->RegisterPlot<TH2F>("ISOMER_3604","Mtas prev-no-#beta curr-no-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (ns)",this->h2dsettings.at(3604));
	hismanager->RegisterPlot<TH2F>("ISOMER_3605","Mtas prev-no-#beta curr-no-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (us)",this->h2dsettings.at(3605));
	
	hismanager->RegisterPlot<TH2F>("ISOMER_3700","Mtas prev-#beta curr-no-#beta Measure Cycle Gated; Curr Energy (keV); Time (ns)",this->h2dsettings.at(3700));
	hismanager->RegisterPlot<TH2F>("ISOMER_3701","Mtas prev-#beta curr-no-#beta Measure Cycle Gated; Curr Energy (keV); Time (us)",this->h2dsettings.at(3701));
	hismanager->RegisterPlot<TH2F>("ISOMER_3702","Mtas prev-#beta curr-no-#beta Measure Cycle Gated; Prev Energy (keV); Time (ns)",this->h2dsettings.at(3702));
	hismanager->RegisterPlot<TH2F>("ISOMER_3703","Mtas prev-#beta curr-no-#beta Measure Cycle Gated; Prev Energy (keV); Time (us)",this->h2dsettings.at(3703));
	hismanager->RegisterPlot<TH2F>("ISOMER_3704","Mtas prev-#beta curr-no-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (ns)",this->h2dsettings.at(3704));
	hismanager->RegisterPlot<TH2F>("ISOMER_3705","Mtas prev-#beta curr-no-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (us)",this->h2dsettings.at(3705));

	for( size_t ii = 0; ii < this->ISOMER_3701_Gates.size(); ++ii ){
		std::string label = "ISOMER_373"+std::to_string(ii);
		std::string title = "Mtas prev-#beta curr-no-#beta Measure Cycle Gated Curr Total Energy Gated [";
		title += std::to_string(this->ISOMER_3701_Gates.at(ii).GetLowerXBound())+","+std::to_string(this->ISOMER_3701_Gates.at(ii).GetUpperXBound());
		title += "] Time Gated [";
		title += std::to_string(this->ISOMER_3701_Gates.at(ii).GetLowerYBound())+","+std::to_string(this->ISOMER_3701_Gates.at(ii).GetUpperYBound());
		title += "]; Prev. Energy (keV);";
		hismanager->RegisterPlot<TH1F>(label,title,this->h1dsettings.at(3730));
	}

	hismanager->RegisterPlot<TH2F>("ISOMER_3800","Mtas prev-no-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (ns)",this->h2dsettings.at(3800));
	hismanager->RegisterPlot<TH2F>("ISOMER_3801","Mtas prev-no-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (us)",this->h2dsettings.at(3801));
	hismanager->RegisterPlot<TH2F>("ISOMER_3802","Mtas prev-no-#beta curr-#beta Measure Cycle Gated; Prev Energy (keV); Time (ns)",this->h2dsettings.at(3802));
	hismanager->RegisterPlot<TH2F>("ISOMER_3803","Mtas prev-no-#beta curr-#beta Measure Cycle Gated; Prev Energy (keV); Time (us)",this->h2dsettings.at(3803));
	hismanager->RegisterPlot<TH2F>("ISOMER_3804","Mtas prev-no-#beta curr-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (ns)",this->h2dsettings.at(3804));
	hismanager->RegisterPlot<TH2F>("ISOMER_3805","Mtas prev-no-#beta curr-#beta Measure Cycle Gated; Curr+Prev Energy (keV); Time (us)",this->h2dsettings.at(3805));

	hismanager->RegisterPlot<TH2F>("ISOMER_3900","Mtas curr-#beta TDiff (Last MTAS - First Si) Measure Cycle Gated; Energy (keV); Time (ns)",this->h2dsettings.at(3900));

	hismanager->RegisterPlot<TH2F>("ISOMER_4500","HPGe prev-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (ns)",this->h2dsettings.at(3500));
	hismanager->RegisterPlot<TH2F>("ISOMER_4501","HPGe prev-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (us)",this->h2dsettings.at(3501));
	
	hismanager->RegisterPlot<TH2F>("ISOMER_4600","HPGe prev-no-#beta curr-no-#beta Measure Cycle Gated; Curr Energy (keV); Time (ns)",this->h2dsettings.at(3600));
	hismanager->RegisterPlot<TH2F>("ISOMER_4601","HPGe prev-no-#beta curr-no-#beta Measure Cycle Gated; Curr Energy (keV); Time (us)",this->h2dsettings.at(3601));
	
	hismanager->RegisterPlot<TH2F>("ISOMER_4700","HPGe prev-#beta curr-no-#beta Measure Cycle Gated; Curr Energy (keV); Time (ns)",this->h2dsettings.at(3700));
	hismanager->RegisterPlot<TH2F>("ISOMER_4701","HPGe prev-#beta curr-no-#beta Measure Cycle Gated; Curr Energy (keV); Time (us)",this->h2dsettings.at(3701));

	hismanager->RegisterPlot<TH2F>("ISOMER_4800","HPGe prev-no-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (ns)",this->h2dsettings.at(3800));
	hismanager->RegisterPlot<TH2F>("ISOMER_4801","HPGe prev-no-#beta curr-#beta Measure Cycle Gated; Curr Energy (keV); Time (us)",this->h2dsettings.at(3801));

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
