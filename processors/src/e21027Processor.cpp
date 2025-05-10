#include "e21027Processor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

e21027Processor::e21027Processor(const std::string& log) : Processor(log,"e21027Processor",{}){
	this->MtasProc = std::make_shared<MtasProcessor>(log);
	this->ImplantProc = std::make_shared<MtasImplantProcessor>(log);
	this->PidProc = std::make_shared<PidProcessor>(log);
	this->VetoProc = std::make_shared<VetoProcessor>(log);

	for( const auto& type : this->MtasProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	for( const auto& type : this->ImplantProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	for( const auto& type : this->PidProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	for( const auto& type : this->VetoProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	this->h1dsettings = {
		{2000,{65536,0,65536}},
		{2010,{65536,0,65536}},
		{2020,{65536,0,65536}},
		{2030,{65536,0,65536}},
		{2040,{65536,0,65536}},
		{2015,{65536,0,65536}},
		{2025,{65536,0,65536}},
		{2035,{65536,0,65536}},
		{2045,{65536,0,65536}},
		
		{2100,{65536,0,65536}},
		{2110,{65536,0,65536}},
		{2120,{65536,0,65536}},
		{2130,{65536,0,65536}},
		{2140,{65536,0,65536}},
		{2115,{65536,0,65536}},
		{2125,{65536,0,65536}},
		{2135,{65536,0,65536}},
		{2145,{65536,0,65536}}
	};

	this->h2dsettings = {

		//isomer plots
		{3700,{8192,0,8192,1000,0,10000}},
		{3701,{8192,0,8192,1000,0,10000}},

		{3800,{8192,0,8192,1000,0,10000}},
		{3801,{8192,0,8192,1000,0,10000}},

		//beta plots
		{3650,{4096,0,4096,4096,0,4096}},
		{3651,{4096,0,4096,4096,0,4096}},
		{3652,{4096,0,4096,4096,0,4096}},
		
		{36508,{4096,0,65536,4096,0,65536}},
		{36518,{4096,0,65536,4096,0,65536}},
		{36528,{4096,0,65536,4096,0,65536}},
		
		{8000,{8192,0,16384,1024,0,16}},
		{8001,{8192,0,16384,1024,0,16}},
		{8002,{8192,0,16384,1024,0,16}},
		{8003,{1024,0,10,1024,0,10}},
		
		{9000,{1024,0,10,1024,0,10}},

		//ion plots
		{3750,{4096,0,4096,4096,0,4096}},
		{3751,{4096,0,4096,4096,0,4096}},
		{3752,{4096,0,4096,4096,0,4096}},

		{37508,{4096,0,65536,4096,0,65536}},
		{37518,{4096,0,65536,4096,0,65536}},
		{37528,{4096,0,65536,4096,0,65536}},

		{8005,{8192,0,16384,1024,0,16}},
		{8006,{8192,0,16384,1024,0,16}},
		{8007,{8192,0,16384,1024,0,16}},
		{8008,{1024,0,10,1024,0,10}},
		
		{6000,{1024,0,10,1024,0,10}},
		
		//pin 1
		{10000,{2000,-1000,2000,16000,0,16000}},
		{10004,{2000,-1000,2000,16000,0,16000}},
		{10008,{2000,-1000,2000,16000,0,16000}},
		{10012,{2000,-1000,2000,16000,0,16000}},

		//pin 2
		{10001,{2000,-1000,2000,16000,0,16000}},
		{10005,{2000,-1000,2000,16000,0,16000}},
		{10009,{2000,-1000,2000,16000,0,16000}},
		{10013,{2000,-1000,2000,16000,0,16000}},

		//pin 3
		{10002,{2000,-1000,2000,16000,0,16000}},
		{10006,{2000,-1000,2000,16000,0,16000}},
		{10010,{2000,-1000,2000,16000,0,16000}},
		{10014,{2000,-1000,2000,16000,0,16000}},

		//pin 4
		{10003,{2000,-1000,2000,16000,0,16000}},
		{10007,{2000,-1000,2000,16000,0,16000}},
		{10011,{2000,-1000,2000,16000,0,16000}},
		{10015,{2000,-1000,2000,16000,0,16000}}
	};

	this->beta = "beta";
	this->gamma = "gamma";
	this->implant = "ion";

	this->HasMTAS = false;
	this->HasSIPM = false;
	this->HasVeto = false;
	this->HasPID = false;

	this->ion_beta_limits = std::unique_ptr<boost::circular_buffer<std::pair<unsigned long long,unsigned long long>>>(nullptr);

	this->Reset();
}

[[maybe_unused]] bool e21027Processor::PreProcess(EventHistoryManager* eventhistory, PLOTS::PlotRegistry* hismanager, CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();
	if( this->ion_beta_limits == nullptr ){
		this->ion_beta_limits.reset(new boost::circular_buffer<std::pair<unsigned long long,unsigned long long>>(eventhistory->GetMaxHistorySize()));
	}

	auto summary = eventhistory->GetCurrentEventSummary();
	auto types = summary->GetKnownTypes();

	this->HasMTAS = (types.find("mtas") != types.end());
	this->HasSIPM = (types.find("mtasimplant") != types.end()); 
	this->HasPID = (types.find("pid") != types.end()); 
	this->HasVeto = (types.find("veto") != types.end());

	if( this->HasSIPM ){
		this->ImplantProc->PreProcess(eventhistory,hismanager,cutmanager);
		//auto lgImage = this->ImplantProc->GetLowGainImage();
		//if( lgImage.anodesum > this->ImplantThreshold ){
		//	summary->AddEventTag(this->implant);
		//}
	}

	if( this->HasPID ){
		this->PidProc->PreProcess(eventhistory,hismanager,cutmanager);
	}

	if( this->HasMTAS ){
		this->MtasProc->PreProcess(eventhistory,hismanager,cutmanager);
	}

	if( this->HasVeto ){
		this->VetoProc->PreProcess(eventhistory,hismanager,cutmanager);
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool e21027Processor::Process( EventHistoryManager* eventhistory, PLOTS::PlotRegistry* hismanager, CUTS::CutRegistry* cutmanager){
	Processor::Process();
	//determine if we had a beta trigger in MTAS and are in the correct cycle
	auto numhist = eventhistory->GetMaxHistoryID();
	auto summary = eventhistory->GetCurrentEventSummary();
	auto erg = MtasProc->GetTotalEnergy(0);

	auto hasion = summary->ContainsEventTag(this->implant);
	auto hasbeta = summary->ContainsEventTag(this->beta);
	auto hasgamma = summary->ContainsEventTag(this->gamma);
	auto hasmuon = summary->ContainsEventTag("muon");
	auto hasrit = summary->ContainsEventTag("rit");

	if( not hasmuon ){
		if( hasion ){
			auto total = this->MtasProc->GetTotalEnergy(0);
			auto ctotal = this->MtasProc->GetTotalEnergy(1);
			auto itotal = this->MtasProc->GetTotalEnergy(2);
			auto mtotal = this->MtasProc->GetTotalEnergy(3);
			auto ototal = this->MtasProc->GetTotalEnergy(4);
			auto dynode = this->ImplantProc->GetLowGainImage().dynode;
			hismanager->Fill("EXP_3750",total,dynode);
			hismanager->Fill("EXP_3751",ctotal,dynode);
			hismanager->Fill("EXP_37508",total,dynode);
			hismanager->Fill("EXP_37518",ctotal,dynode);
			hismanager->Fill("EXP_2100",dynode+total);
			hismanager->Fill("EXP_2110",dynode+ctotal);
			hismanager->Fill("EXP_2120",dynode+itotal);
			hismanager->Fill("EXP_2130",dynode+mtotal);
			hismanager->Fill("EXP_2140",dynode+ototal);
			for( size_t ii = 0; ii < 6; ++ii ){
				hismanager->Fill("EXP_3752",this->MtasProc->GetCrystalEnergy(ii),dynode);
				hismanager->Fill("EXP_37528",this->MtasProc->GetCrystalEnergy(ii),dynode);
				
				hismanager->Fill("EXP_2115",dynode+this->MtasProc->GetCrystalEnergy(ii));
				hismanager->Fill("EXP_2125",dynode+this->MtasProc->GetCrystalEnergy(ii+6));
				hismanager->Fill("EXP_2135",dynode+this->MtasProc->GetCrystalEnergy(ii+12));
				hismanager->Fill("EXP_2145",dynode+this->MtasProc->GetCrystalEnergy(ii+18));
			}

			auto ionx = summary->GetEventObservable("ION_X").value();
			auto iony = summary->GetEventObservable("ION_Y").value();
			auto ionr = std::sqrt(ionx*ionx + iony*iony);
			hismanager->Fill("EXP_8005",dynode,ionr);
			hismanager->Fill("EXP_8006",dynode,ionx);
			hismanager->Fill("EXP_8007",dynode,iony);
			hismanager->Fill("EXP_8008",ionx,iony);
			for (size_t iPins = 0 ; iPins < this->PidProc->GetNumFP1Pins(); ++iPins){
				hismanager->Fill("EXP_" + std::to_string(10000 +iPins ), this->PidProc->GetFP1Tof(0),this->PidProc->GetFP1PinEnergy(iPins));
				hismanager->Fill("EXP_" + std::to_string(10004 +iPins ), this->PidProc->GetFP1Tof(2),this->PidProc->GetFP1PinEnergy(iPins));
				hismanager->Fill("EXP_" + std::to_string(10008 +iPins ), this->PidProc->GetFP1Tof(4),this->PidProc->GetFP1PinEnergy(iPins));
				hismanager->Fill("EXP_" + std::to_string(10012 +iPins ), this->PidProc->GetFP1Tof(6),this->PidProc->GetFP1PinEnergy(iPins));
			}
			for( size_t ii = 0; ii < this->isotopetags.size(); ++ii ){
				if( summary->ContainsEventTag(this->isotopetags.at(ii)) ){
					std::string title = "EXP_600"+std::to_string(ii);
					hismanager->Fill(title,ionx,iony);
				}
			}
			////found new ion, need to add it to the limit list
			////and then correlate it with all known betas
			//auto ion_idx = static_cast<unsigned long long>(summary->GetEventObservable("Event_idx").value());
			////use the boost::circular_buffer to queue the things
			//this->ion_beta_limits->push_front({ion_idx,0});
			////search through the summaries previous and we'll grab their idx
			//for( size_t ii = 1; ii < eventhistory->GetMaxHistorySize(); ++ii ){
			//	auto prevsummary = eventhistory->GetPreviousEventSummary(ii);
			//	auto preveventidx = static_cast<unsigned long long>(prevsummary->GetEventObservable("Event_idx").value());
			//	auto isprevbeta = prevsummary->ContainsEventTag(this->beta);
			//	if( isprevbeta ){
			//		this->ion_beta_limits->at(0).second = std::max(this->ion_beta_limits->at(0).second,preveventidx);
			//		//determine which tdiff plot to fill
			//		//these are all the negative time portions of the tdiff
			//	}
			//}
		}

		if( numhist > 1){
			//search through the old indices to find the delayed gamma from a beta
			for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
				auto prevsummary = eventhistory->GetPreviousEventSummary(ii);
				auto prevmuon = prevsummary->ContainsEventTag("muon");
				if( prevmuon ){
					continue;
				}
				auto prevbeta = prevsummary->ContainsEventTag(this->beta);
				auto prevgamma = prevsummary->ContainsEventTag(this->gamma);
				auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
				if( not hasbeta and hasgamma and prevbeta ){
					hismanager->Fill("ISOMER_3700",erg,isomer_tdiff);
					hismanager->Fill("ISOMER_3701",erg,isomer_tdiff*1.0e-3);
						break;
				}
			}
			for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
				auto prevsummary =  eventhistory->GetPreviousEventSummary(ii);
				auto prevmuon = prevsummary->ContainsEventTag("muon");
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

		if( hasbeta ){
			this->MtasProc->FillBetaPlots(hismanager);
			this->MtasProc->FillNoLogicBetaPlots(hismanager);
			auto total = this->MtasProc->GetTotalEnergy(0);
			auto ctotal = this->MtasProc->GetTotalEnergy(1);
			auto itotal = this->MtasProc->GetTotalEnergy(2);
			auto mtotal = this->MtasProc->GetTotalEnergy(3);
			auto ototal = this->MtasProc->GetTotalEnergy(4);
			auto dynode = this->ImplantProc->GetHighGainImage().dynode;
			hismanager->Fill("EXP_3650",total,dynode);
			hismanager->Fill("EXP_3651",ctotal,dynode);
			hismanager->Fill("EXP_36508",total,dynode);
			hismanager->Fill("EXP_36518",ctotal,dynode);
			hismanager->Fill("EXP_2000",dynode+total);
			hismanager->Fill("EXP_2010",dynode+ctotal);
			hismanager->Fill("EXP_2020",dynode+itotal);
			hismanager->Fill("EXP_2030",dynode+mtotal);
			hismanager->Fill("EXP_2040",dynode+ototal);
			for( size_t ii = 0; ii < 6; ++ii ){
				hismanager->Fill("EXP_3652",this->MtasProc->GetCrystalEnergy(ii),dynode);
				hismanager->Fill("EXP_36528",this->MtasProc->GetCrystalEnergy(ii),dynode);
		
				hismanager->Fill("EXP_2015",dynode+this->MtasProc->GetCrystalEnergy(ii));
				hismanager->Fill("EXP_2025",dynode+this->MtasProc->GetCrystalEnergy(ii+6));
				hismanager->Fill("EXP_2035",dynode+this->MtasProc->GetCrystalEnergy(ii+12));
				hismanager->Fill("EXP_2045",dynode+this->MtasProc->GetCrystalEnergy(ii+18));
			}

			auto betax = summary->GetEventObservable("BETA_X").value();
			auto betay = summary->GetEventObservable("BETA_Y").value();
			auto betar = std::sqrt(betax*betax + betay*betay);
			//this->console->info("X:{}, Y:{}, R:{}",betax,betay,betar); 
			hismanager->Fill("EXP_8000",dynode,betar);
			hismanager->Fill("EXP_8001",dynode,betax);
			hismanager->Fill("EXP_8002",dynode,betay);
			hismanager->Fill("EXP_8003",betax,betay);
			for( size_t ii = 0; ii < this->MTAS_Total_Gates.size(); ++ii ){
				if( this->MTAS_Total_Gates.at(ii).IsWithin(total) ){
					std::string label = "EXP_900"+std::to_string(ii);
					hismanager->Fill(label,betax,betay);
				}
			}
			//found new beta, need to go through the known ion list and correlate it with us
			//and update their secondary
			//auto beta_idx = static_cast<unsigned long long>(summary->GetEventObservable("Event_idx").value());
			//for( size_t ii = 0; ii < this->ion_beta_limits->size(); ++ii ){
			// 	this->ion_beta_limits->at(ii).second = beta_idx;	
			//}
		}else{
			this->MtasProc->FillNonBetaPlots(hismanager);
			this->MtasProc->FillNoLogicNonBetaPlots(hismanager);
		}
	}

	Processor::EndProcess();
	return true;
	}

[[maybe_unused]] bool e21027Processor::PostProcess( EventHistoryManager* eventhistory, PLOTS::PlotRegistry* hismanager, CUTS::CutRegistry* cutmanager){
	Processor::PostProcess();
	if( this->HasSIPM ){
		this->ImplantProc->PostProcess(eventhistory,hismanager,cutmanager);
	}
	if( this->HasPID ){
		this->PidProc->PostProcess(eventhistory,hismanager,cutmanager);
	}
	if( this->HasMTAS ){
		this->MtasProc->PostProcess(eventhistory,hismanager,cutmanager);
	}
	if( this->HasVeto ){
		this->VetoProc->PostProcess(eventhistory,hismanager,cutmanager);
	}
	this->Reset();

	Processor::EndProcess();
	return true;
}

void e21027Processor::Init(const pugi::xml_node& config){
	for( pugi::xml_node proc = config.child("Processor"); proc; proc = proc.next_sibling("Processor") ){
		std::string name = proc.attribute("name").as_string();
		if( name.compare("MtasProcessor") == 0 ){
			this->MtasProc->Init(proc);
			this->HasMTAS = true;
		}else if( name.compare("MtasImplantProcessor") == 0 ){
			this->ImplantProc->Init(proc);
			this->HasSIPM = true;
		}else if( name.compare("PidProcessor") == 0 ){
			this->PidProc->Init(proc);
			this->HasPID = true;
		}else if( name.compare("VetoProcessor") == 0 ){
			this->VetoProc->Init(proc);
			this->HasVeto = true;
		}else{
			throw std::runtime_error("unknown subprocessor declared in e21027Processor");
		}
	}

	if( not this->HasPID ){
		throw std::runtime_error("missing PidProcessor in e21027Processor");
	}

	if( not this->HasMTAS ){
		throw std::runtime_error("missing MtasProcessor in e21027Processor");
	}

	if( not this->HasSIPM ){
		throw std::runtime_error("missing MtasImplantProcessor in e21027Processor");
	}

	if( not this->HasVeto ){
		throw std::runtime_error("missing VetoProcessor in e21027Processor");
	}

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);

	this->isotopetags = this->PidProc->GetIsotopeTags();

	for( pugi::xml_node gate = config.child("Gate"); gate; gate = gate.next_sibling("Gate") ){
		std::string label = gate.attribute("label").as_string("");
		if( label.compare("MTAS_Total") == 0 ){
			auto low = gate.attribute("lowerbound").as_double(0.0);
			auto high = gate.attribute("upperbound").as_double(16384.0);
			this->MTAS_Total_Gates.push_back(Gate<double>(low,high));
		}
	}
}
		
void e21027Processor::Finalize(){
	this->MtasProc->Finalize();
	this->ImplantProc->Finalize();
	this->VetoProc->Finalize();
	this->PidProc->Finalize();

	this->console->info("{} has been finalized",this->ProcessorName);
}

void e21027Processor::DeclarePlots(PLOTS::PlotRegistry* hismanager){
	this->MtasProc->DeclarePlots(hismanager);
	this->ImplantProc->DeclarePlots(hismanager);
	this->PidProc->DeclarePlots(hismanager);
	this->VetoProc->DeclarePlots(hismanager);

	hismanager->RegisterPlot<TH2F>("EXP_10000","DB3P0A-FP1XP1 vs Pin 1 Energy :: LG_Dynode Gated",	this->h2dsettings.at(10000));
	hismanager->RegisterPlot<TH2F>("EXP_10001","DB3P0A-FP1XP1 vs Pin 2 Energy :: LG_Dynode Gated",	this->h2dsettings.at(10001));
	hismanager->RegisterPlot<TH2F>("EXP_10002","DB3P0A-FP1XP1 vs Pin 3 Energy :: LG_Dynode Gated",	this->h2dsettings.at(10002));
	hismanager->RegisterPlot<TH2F>("EXP_10003","DB3P0A-FP1XP1 vs Pin 4 Energy :: LG_Dynode Gated",	this->h2dsettings.at(10003));

	hismanager->RegisterPlot<TH2F>("EXP_10004","DB3P1A-FP1XP1 vs Pin 1 Energy :: LG_Dynode Gated",	this->h2dsettings.at(10004));
	hismanager->RegisterPlot<TH2F>("EXP_10005","DB3P1A-FP1XP1 vs Pin 2 Energy :: LG_Dynode Gated",	this->h2dsettings.at(10005));
	hismanager->RegisterPlot<TH2F>("EXP_10006","DB3P1A-FP1XP1 vs Pin 3 Energy :: LG_Dynode Gated",	this->h2dsettings.at(10006));
	hismanager->RegisterPlot<TH2F>("EXP_10007","DB3P1A-FP1XP1 vs Pin 4 Energy :: LG_Dynode Gated",	this->h2dsettings.at(10007));

	hismanager->RegisterPlot<TH2F>("EXP_10008","DB3SL-FP1XP1 vs Pin 1 Energy :: LG_Dynode Gated",	this->h2dsettings.at(10008));
	hismanager->RegisterPlot<TH2F>("EXP_10009","DB3SL-FP1XP1 vs Pin 2 Energy :: LG_Dynode Gated",	this->h2dsettings.at(10009));
	hismanager->RegisterPlot<TH2F>("EXP_10010","DB3SL-FP1XP1 vs Pin 3 Energy :: LG_Dynode Gated",	this->h2dsettings.at(10010));
	hismanager->RegisterPlot<TH2F>("EXP_10011","DB3SL-FP1XP1 vs Pin 4 Energy :: LG_Dynode Gated",	this->h2dsettings.at(10011));

	hismanager->RegisterPlot<TH2F>("EXP_10012","DB3SR-FP1XP1 vs Pin 1 Energy :: LG_Dynode Gated",	this->h2dsettings.at(10012));
	hismanager->RegisterPlot<TH2F>("EXP_10013","DB3SR-FP1XP1 vs Pin 2 Energy :: LG_Dynode Gated",	this->h2dsettings.at(10013));
	hismanager->RegisterPlot<TH2F>("EXP_10014","DB3SR-FP1XP1 vs Pin 3 Energy :: LG_Dynode Gated",	this->h2dsettings.at(10014));
	hismanager->RegisterPlot<TH2F>("EXP_10015","DB3SR-FP1XP1 vs Pin 4 Energy :: LG_Dynode Gated",	this->h2dsettings.at(10015));

	hismanager->RegisterPlot<TH2F>("EXP_3650","High Gain Dynode vs MTAS Total; MTAS Total Energy (keV); Dynode Energy (keV);",this->h2dsettings.at(3650));
	hismanager->RegisterPlot<TH2F>("EXP_3651","High Gain Dynode vs MTAS Center Sum; MTAS Center Energy (keV); Dynode Energy (keV);",this->h2dsettings.at(3651));
	hismanager->RegisterPlot<TH2F>("EXP_3652","High Gain Dynode vs MTAS Center Individual; MTAS Center Crystal Energy (keV); Dynode Energy (keV);",this->h2dsettings.at(3652));
	hismanager->RegisterPlot<TH2F>("EXP_36508","High Gain Dynode vs MTAS Total; MTAS Total Energy (8 keV/bin); Dynode Energy (8 keV/bin);",this->h2dsettings.at(36508));
	hismanager->RegisterPlot<TH2F>("EXP_36518","High Gain Dynode vs MTAS Center Sum; MTAS Center Energy (8 keV/bin); Dynode Energy (8 keV/bin);",this->h2dsettings.at(36518));
	hismanager->RegisterPlot<TH2F>("EXP_36528","High Gain Dynode vs MTAS Center Individual; MTAS Center Crystal Energy (8 keV/bin); Dynode Energy (8 keV/bin);",this->h2dsettings.at(36528));

	hismanager->RegisterPlot<TH1F>("EXP_2000","High Gain Dynode + MTAS Total; Energy (keV)",this->h1dsettings.at(2000));
	hismanager->RegisterPlot<TH1F>("EXP_2010","High Gain Dynode + MTAS Center Sum; Energy (keV)",this->h1dsettings.at(2010));
	hismanager->RegisterPlot<TH1F>("EXP_2020","High Gain Dynode + MTAS Inner Sum; Energy (keV)",this->h1dsettings.at(2020));
	hismanager->RegisterPlot<TH1F>("EXP_2030","High Gain Dynode + MTAS Middle Sum; Energy (keV)",this->h1dsettings.at(2030));
	hismanager->RegisterPlot<TH1F>("EXP_2040","High Gain Dynode + MTAS Outer Sum; Energy (keV)",this->h1dsettings.at(2040));
	hismanager->RegisterPlot<TH1F>("EXP_2015","High Gain Dynode + MTAS Center Ind.; Energy (keV)",this->h1dsettings.at(2015));
	hismanager->RegisterPlot<TH1F>("EXP_2025","High Gain Dynode + MTAS Inner Ind.; Energy (keV)",this->h1dsettings.at(2025));
	hismanager->RegisterPlot<TH1F>("EXP_2035","High Gain Dynode + MTAS Middle Ind.; Energy (keV)",this->h1dsettings.at(2035));
	hismanager->RegisterPlot<TH1F>("EXP_2045","High Gain Dynode + MTAS Outer Ind.; Energy (keV)",this->h1dsettings.at(2045));

	hismanager->RegisterPlot<TH2F>("EXP_3750","Low Gain Dynode vs MTAS Total; MTAS Total Energy (keV); Dynode Energy (keV);",this->h2dsettings.at(3750));
	hismanager->RegisterPlot<TH2F>("EXP_3751","Low Gain Dynode vs MTAS Center Sum; MTAS Center Energy (keV); Dynode Energy (keV);",this->h2dsettings.at(3751));
	hismanager->RegisterPlot<TH2F>("EXP_3752","Low Gain Dynode vs MTAS Center Individual; MTAS Center Crystal Energy (keV); Dynode Energy (keV);",this->h2dsettings.at(3752));
	
	hismanager->RegisterPlot<TH2F>("EXP_37508","Low Gain Dynode vs MTAS Total; MTAS Total Energy (8 keV/bin); Dynode Energy (8 keV/bin);",this->h2dsettings.at(37508));
	hismanager->RegisterPlot<TH2F>("EXP_37518","Low Gain Dynode vs MTAS Center Sum; MTAS Center Energy (8 keV/bin); Dynode Energy (8 keV/bin);",this->h2dsettings.at(37518));
	hismanager->RegisterPlot<TH2F>("EXP_37528","Low Gain Dynode vs MTAS Center Individual; MTAS Center Crystal Energy (8 keV/bin); Dynode Energy (8 keV/bin);",this->h2dsettings.at(37528));
	
	hismanager->RegisterPlot<TH1F>("EXP_2100","High Gain Dynode + MTAS Total; Energy (keV)",this->h1dsettings.at(2100));
	hismanager->RegisterPlot<TH1F>("EXP_2110","High Gain Dynode + MTAS Center Sum; Energy (keV)",this->h1dsettings.at(2110));
	hismanager->RegisterPlot<TH1F>("EXP_2120","High Gain Dynode + MTAS Inner Sum; Energy (keV)",this->h1dsettings.at(2120));
	hismanager->RegisterPlot<TH1F>("EXP_2130","High Gain Dynode + MTAS Middle Sum; Energy (keV)",this->h1dsettings.at(2130));
	hismanager->RegisterPlot<TH1F>("EXP_2140","High Gain Dynode + MTAS Outer Sum; Energy (keV)",this->h1dsettings.at(2140));
	hismanager->RegisterPlot<TH1F>("EXP_2115","High Gain Dynode + MTAS Center Ind.; Energy (keV)",this->h1dsettings.at(2115));
	hismanager->RegisterPlot<TH1F>("EXP_2125","High Gain Dynode + MTAS Inner Ind.; Energy (keV)",this->h1dsettings.at(2125));
	hismanager->RegisterPlot<TH1F>("EXP_2135","High Gain Dynode + MTAS Middle Ind.; Energy (keV)",this->h1dsettings.at(2135));
	hismanager->RegisterPlot<TH1F>("EXP_2145","High Gain Dynode + MTAS Outer Ind.; Energy (keV)",this->h1dsettings.at(2145));

	hismanager->RegisterPlot<TH2F>("EXP_8000","Beta Radius vs Energy; Energy (keV); Radius (pixels)",this->h2dsettings.at(8000));
	hismanager->RegisterPlot<TH2F>("EXP_8001","Beta X vs Energy; Energy (keV); X (pixels)",this->h2dsettings.at(8001));
	hismanager->RegisterPlot<TH2F>("EXP_8002","Beta Y vs Energy; Energy (keV); Y (pixels)",this->h2dsettings.at(8002));
	hismanager->RegisterPlot<TH2F>("EXP_8003","Beta Image ; X (pixel); Y (pixels)",this->h2dsettings.at(8003));

	for( size_t ii = 0; ii < this->MTAS_Total_Gates.size(); ++ii ){
		std::string label = "EXP_900"+std::to_string(ii);
		std::string title = "Beta Image Gated on MTAS Total [";
		title += std::to_string(this->MTAS_Total_Gates.at(ii).GetLowerBound())+","+std::to_string(this->MTAS_Total_Gates.at(ii).GetUpperBound());
		title += "; X (pixel); Y (pixel)";
		hismanager->RegisterPlot<TH2F>(label,title,this->h2dsettings.at(9000));
	}

	for( size_t ii = 0; ii < this->isotopetags.size(); ++ii ){
		std::string label = "EXP_600"+std::to_string(ii);
		std::string title = "Ion Image Gated on "+this->isotopetags.at(ii)+"; X (pixel); Y (pixel)";
		hismanager->RegisterPlot<TH2F>(label,title,this->h2dsettings.at(6000));
	}
	//this->isotopetags = this->PidProc->GetIsotopeTags();

	hismanager->RegisterPlot<TH2F>("EXP_8005","Ion Radius vs Energy; Energy (keV); Radius (pixels)",this->h2dsettings.at(8005));
	hismanager->RegisterPlot<TH2F>("EXP_8006","Ion X vs Energy; Energy (keV); X (pixels)",this->h2dsettings.at(8006));
	hismanager->RegisterPlot<TH2F>("EXP_8007","Ion Y vs Energy; Energy (keV); Y (pixels)",this->h2dsettings.at(8007));
	hismanager->RegisterPlot<TH2F>("EXP_8008","Ion Image ; X (pixel); Y (pixels)",this->h2dsettings.at(8008));

	hismanager->RegisterPlot<TH2F>("ISOMER_3700","Mtas prev-#beta curr-no-#beta; Energy (keV); Time (ns)",this->h2dsettings.at(3700));
	hismanager->RegisterPlot<TH2F>("ISOMER_3701","Mtas prev-#beta curr-no-#beta; Energy (keV); Time (us)",this->h2dsettings.at(3701));

	hismanager->RegisterPlot<TH2F>("ISOMER_3800","Mtas prev-#gamma curr-#beta; Energy (keV); Time (ns)",this->h2dsettings.at(3800));
	hismanager->RegisterPlot<TH2F>("ISOMER_3801","Mtas prev-#gamma curr-#beta; Energy (keV); Time (us)",this->h2dsettings.at(3801));

	for( const auto& t : this->isotopetags ){
		this->console->info("Found Isotope Tag : {}",t);
	}

	this->console->info("Finished Declaring Plots");
}

void e21027Processor::RegisterTree(std::unordered_map<std::string,TTree*>& outputtrees){
	this->MtasProc->RegisterTree(outputtrees);
	this->ImplantProc->RegisterTree(outputtrees);
	this->PidProc->RegisterTree(outputtrees);
	this->VetoProc->RegisterTree(outputtrees);
}

void e21027Processor::CleanupTree(){
	this->MtasProc->CleanupTree();
	this->ImplantProc->CleanupTree();
	this->PidProc->CleanupTree();
	this->VetoProc->CleanupTree();
}

void e21027Processor::Reset(){
}

void e21027Processor::RegisterCuts(CUTS::CutRegistry* cutmanager){
	this->MtasProc->RegisterCuts(cutmanager);
	this->ImplantProc->RegisterCuts(cutmanager);
	this->PidProc->RegisterCuts(cutmanager);
	this->VetoProc->RegisterCuts(cutmanager);
}
