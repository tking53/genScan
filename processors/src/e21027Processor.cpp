#include "e21027Processor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <thread>

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
		//BETA
		{2000,{65536,0,65536}},
		{2010,{65536,0,65536}},
		{2020,{65536,0,65536}},
		{2030,{65536,0,65536}},
		{2040,{65536,0,65536}},
		{2015,{65536,0,65536}},
		{2025,{65536,0,65536}},
		{2035,{65536,0,65536}},
		{2045,{65536,0,65536}},

		//ION
		{2100,{65536,0,65536}},
		{2110,{65536,0,65536}},
		{2120,{65536,0,65536}},
		{2130,{65536,0,65536}},
		{2140,{65536,0,65536}},
		{2115,{65536,0,65536}},
		{2125,{65536,0,65536}},
		{2135,{65536,0,65536}},
		{2145,{65536,0,65536}},

		//DECAY half-lives 
		{4000,{16384,-1024,1024}},
		{4001,{16384,-1024,1024}},
		{4002,{16384,-8,8}},
		{4003,{16384,-2,2}},

		//DECAY radii
		{8000,{1024,0,10}}

	};

	this->h2dsettings = {
		//DECAY half-lives vs radii 
		{5000,{1024,0,10,16384,-1024,1024}},
		{5001,{1024,0,10,16384,-1024,1024}},
		{5002,{1024,0,10,16384,-8,8}},
		{5003,{1024,0,10,16384,-2,2}},

		//DECAY radii vs beta dynode
		{7000,{16384,0,16384,1024,0,10}},

		//DECAY radii vs ion dynode
		{7001,{16384,0,16384,1024,0,10}},


		//ISOMER
		{3700,{8192,0,8192,1000,0,10000}},
		{3701,{8192,0,8192,1000,0,10000}},

		{3800,{8192,0,8192,1000,0,10000}},
		{3801,{8192,0,8192,1000,0,10000}},

		//BETA
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

		//ION
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

		//EXP pin 1
		{10000,{2000,-1000,2000,16000,0,16000}},
		{10004,{2000,-1000,2000,16000,0,16000}},
		{10008,{2000,-1000,2000,16000,0,16000}},
		{10012,{2000,-1000,2000,16000,0,16000}},

		//EXP pin 2
		{10001,{2000,-1000,2000,16000,0,16000}},
		{10005,{2000,-1000,2000,16000,0,16000}},
		{10009,{2000,-1000,2000,16000,0,16000}},
		{10013,{2000,-1000,2000,16000,0,16000}},

		//EXP pin 3
		{10002,{2000,-1000,2000,16000,0,16000}},
		{10006,{2000,-1000,2000,16000,0,16000}},
		{10010,{2000,-1000,2000,16000,0,16000}},
		{10014,{2000,-1000,2000,16000,0,16000}},

		//EXP pin 4
		{10003,{2000,-1000,2000,16000,0,16000}},
		{10007,{2000,-1000,2000,16000,0,16000}},
		{10011,{2000,-1000,2000,16000,0,16000}},
		{10015,{2000,-1000,2000,16000,0,16000}},

		//EXP pin 1
		{11000,{2000,-1000,2000,16000,0,16000}},
		{11004,{2000,-1000,2000,16000,0,16000}},
		{11008,{2000,-1000,2000,16000,0,16000}},
		{11012,{2000,-1000,2000,16000,0,16000}},

		//EXP pin 2
		{11001,{2000,-1000,2000,16000,0,16000}},
		{11005,{2000,-1000,2000,16000,0,16000}},
		{11009,{2000,-1000,2000,16000,0,16000}},
		{11013,{2000,-1000,2000,16000,0,16000}},

		//EXP pin 3
		{11002,{2000,-1000,2000,16000,0,16000}},
		{11006,{2000,-1000,2000,16000,0,16000}},
		{11010,{2000,-1000,2000,16000,0,16000}},
		{11014,{2000,-1000,2000,16000,0,16000}},

		//EXP pin 4
		{11003,{2000,-1000,2000,16000,0,16000}},
		{11007,{2000,-1000,2000,16000,0,16000}},
		{11011,{2000,-1000,2000,16000,0,16000}},
		{11015,{2000,-1000,2000,16000,0,16000}}

	};

	this->beta = "beta";
	this->gamma = "gamma";
	this->implant = "ion";

	this->HasMTAS = false;
	this->HasSIPM = false;
	this->HasVeto = false;
	this->HasPID = false;

	this->FoundFirst = false;
	this->FirstTime = 0.0;
	this->LastTime = 0.0;

	this->NThreads = std::thread::hardware_concurrency()/2;

	this->Reset();
}

[[maybe_unused]] bool e21027Processor::PreProcess(EventHistoryManager* eventhistory, PLOTS::PlotRegistry* hismanager, CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	auto summary = eventhistory->GetCurrentEventSummary();
	if( not this->FoundFirst ){
		this->FoundFirst = true;
		this->FirstTime = 1.0e-9*(summary->GetRawEvents().front().GetTimeStamp());
	}
	this->LastTime = 1.0e-9*(summary->GetRawEvents().front().GetTimeStamp());
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
	const auto numhist = eventhistory->GetMaxHistoryID();
	const auto summary = eventhistory->GetCurrentEventSummary();
	const auto erg = MtasProc->GetTotalEnergy(0);

	const auto hasion = summary->ContainsEventTag(this->implant);
	const auto hasbeta = summary->ContainsEventTag(this->beta);
	const auto hasgamma = summary->ContainsEventTag(this->gamma);
	const auto hasmuon = summary->ContainsEventTag("muon");
	//const auto hasrit = summary->ContainsEventTag("rit");

	if( not hasmuon ){
		//do I need to reject when it's both ion and beta????
		//need to make the below a work item and multithread????
		//need to add locking and mutex to hismanager
		if( hasion and not hasbeta ){
			this->AddIonToCorrelation(eventhistory,hismanager,cutmanager);
		}

		//this function is incredibly slow on real beta-ion data, but why?
		//if( numhist > 1){
		//	this->DoIsomerCorrelation(eventhistory,hismanager,cutmanager);
		//}

		//do I need to reject when it's both ion and beta????
		//need to make the below a work item and multithread????
		//need to add locking and mutex to hismanager
		if( hasbeta and not hasion ){
			this->AddBetaToCorrelation(eventhistory,hismanager,cutmanager);
		}else{
			//these have ions and actual background from the room
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

	hismanager->RegisterPlot<TH2F>("EXP_10000","Pin 1 Energy vs DB3P0A-FP1XP1 Low Gain Dynode Gated; TDiff (ns); Energy (keV)",this->h2dsettings.at(10000));
	hismanager->RegisterPlot<TH2F>("EXP_10001","Pin 2 Energy vs DB3P0A-FP1XP1 Low Gain Dynode Gated; TDiff (ns); Energy (keV)",this->h2dsettings.at(10001));
	hismanager->RegisterPlot<TH2F>("EXP_10002","Pin 3 Energy vs DB3P0A-FP1XP1 Low Gain Dynode Gated; TDiff (ns); Energy (keV)",this->h2dsettings.at(10002));
	hismanager->RegisterPlot<TH2F>("EXP_10003","Pin 4 Energy vs DB3P0A-FP1XP1 Low Gain Dynode Gated; TDiff (ns); Energy (keV)",this->h2dsettings.at(10003));

	hismanager->RegisterPlot<TH2F>("EXP_10004","Pin 1 Energy vs DB3P1A-FP1XP1 Low Gain Dynode Gated; TDiff (ns); Energy (keV)",this->h2dsettings.at(10004));
	hismanager->RegisterPlot<TH2F>("EXP_10005","Pin 2 Energy vs DB3P1A-FP1XP1 Low Gain Dynode Gated; TDiff (ns); Energy (keV)",this->h2dsettings.at(10005));
	hismanager->RegisterPlot<TH2F>("EXP_10006","Pin 3 Energy vs DB3P1A-FP1XP1 Low Gain Dynode Gated; TDiff (ns); Energy (keV)",this->h2dsettings.at(10006));
	hismanager->RegisterPlot<TH2F>("EXP_10007","Pin 4 Energy vs DB3P1A-FP1XP1 Low Gain Dynode Gated; TDiff (ns); Energy (keV)",this->h2dsettings.at(10007));

	hismanager->RegisterPlot<TH2F>("EXP_10008","Pin 1 Energy vs DB3SL-FP1XP1  Low Gain Dynode Gated; TDiff (ns); Energy (keV)",this->h2dsettings.at(10008));
	hismanager->RegisterPlot<TH2F>("EXP_10009","Pin 2 Energy vs DB3SL-FP1XP1  Low Gain Dynode Gated; TDiff (ns); Energy (keV)",this->h2dsettings.at(10009));
	hismanager->RegisterPlot<TH2F>("EXP_10010","Pin 3 Energy vs DB3SL-FP1XP1  Low Gain Dynode Gated; TDiff (ns); Energy (keV)",this->h2dsettings.at(10010));
	hismanager->RegisterPlot<TH2F>("EXP_10011","Pin 4 Energy vs DB3SL-FP1XP1  Low Gain Dynode Gated; TDiff (ns); Energy (keV)",this->h2dsettings.at(10011));

	hismanager->RegisterPlot<TH2F>("EXP_10012","Pin 1 Energy vs DB3SR-FP1XP1  Low Gain Dynode Gated; TDiff (ns); Energy (keV)",this->h2dsettings.at(10012));
	hismanager->RegisterPlot<TH2F>("EXP_10013","Pin 2 Energy vs DB3SR-FP1XP1  Low Gain Dynode Gated; TDiff (ns); Energy (keV)",this->h2dsettings.at(10013));
	hismanager->RegisterPlot<TH2F>("EXP_10014","Pin 3 Energy vs DB3SR-FP1XP1  Low Gain Dynode Gated; TDiff (ns); Energy (keV)",this->h2dsettings.at(10014));
	hismanager->RegisterPlot<TH2F>("EXP_10015","Pin 4 Energy vs DB3SR-FP1XP1  Low Gain Dynode Gated; TDiff (ns); Energy (keV)",this->h2dsettings.at(10015));

	hismanager->RegisterPlot<TH2F>("EXP_11000","Pin 1 Energy vs DB3P0A-FP1XP1 Low Gain Dynode Gated Rear Ion Vetoed; TDiff (ns); Energy (keV)",this->h2dsettings.at(11000));
	hismanager->RegisterPlot<TH2F>("EXP_11001","Pin 2 Energy vs DB3P0A-FP1XP1 Low Gain Dynode Gated Rear Ion Vetoed; TDiff (ns); Energy (keV)",this->h2dsettings.at(11001));
	hismanager->RegisterPlot<TH2F>("EXP_11002","Pin 3 Energy vs DB3P0A-FP1XP1 Low Gain Dynode Gated Rear Ion Vetoed; TDiff (ns); Energy (keV)",this->h2dsettings.at(11002));
	hismanager->RegisterPlot<TH2F>("EXP_11003","Pin 4 Energy vs DB3P0A-FP1XP1 Low Gain Dynode Gated Rear Ion Vetoed; TDiff (ns); Energy (keV)",this->h2dsettings.at(11003));

	hismanager->RegisterPlot<TH2F>("EXP_11004","Pin 1 Energy vs DB3P1A-FP1XP1 Low Gain Dynode Gated Rear Ion Vetoed; TDiff (ns); Energy (keV)",this->h2dsettings.at(11004));
	hismanager->RegisterPlot<TH2F>("EXP_11005","Pin 2 Energy vs DB3P1A-FP1XP1 Low Gain Dynode Gated Rear Ion Vetoed; TDiff (ns); Energy (keV)",this->h2dsettings.at(11005));
	hismanager->RegisterPlot<TH2F>("EXP_11006","Pin 3 Energy vs DB3P1A-FP1XP1 Low Gain Dynode Gated Rear Ion Vetoed; TDiff (ns); Energy (keV)",this->h2dsettings.at(11006));
	hismanager->RegisterPlot<TH2F>("EXP_11007","Pin 4 Energy vs DB3P1A-FP1XP1 Low Gain Dynode Gated Rear Ion Vetoed; TDiff (ns); Energy (keV)",this->h2dsettings.at(11007));

	hismanager->RegisterPlot<TH2F>("EXP_11008","Pin 1 Energy vs DB3SL-FP1XP1  Low Gain Dynode Gated Rear Ion Vetoed; TDiff (ns); Energy (keV)",this->h2dsettings.at(11008));
	hismanager->RegisterPlot<TH2F>("EXP_11009","Pin 2 Energy vs DB3SL-FP1XP1  Low Gain Dynode Gated Rear Ion Vetoed; TDiff (ns); Energy (keV)",this->h2dsettings.at(11009));
	hismanager->RegisterPlot<TH2F>("EXP_11010","Pin 3 Energy vs DB3SL-FP1XP1  Low Gain Dynode Gated Rear Ion Vetoed; TDiff (ns); Energy (keV)",this->h2dsettings.at(11010));
	hismanager->RegisterPlot<TH2F>("EXP_11011","Pin 4 Energy vs DB3SL-FP1XP1  Low Gain Dynode Gated Rear Ion Vetoed; TDiff (ns); Energy (keV)",this->h2dsettings.at(11011));

	hismanager->RegisterPlot<TH2F>("EXP_11012","Pin 1 Energy vs DB3SR-FP1XP1  Low Gain Dynode Gated Rear Ion Vetoed; TDiff (ns); Energy (keV)",this->h2dsettings.at(11012));
	hismanager->RegisterPlot<TH2F>("EXP_11013","Pin 2 Energy vs DB3SR-FP1XP1  Low Gain Dynode Gated Rear Ion Vetoed; TDiff (ns); Energy (keV)",this->h2dsettings.at(11013));
	hismanager->RegisterPlot<TH2F>("EXP_11014","Pin 3 Energy vs DB3SR-FP1XP1  Low Gain Dynode Gated Rear Ion Vetoed; TDiff (ns); Energy (keV)",this->h2dsettings.at(11014));
	hismanager->RegisterPlot<TH2F>("EXP_11015","Pin 4 Energy vs DB3SR-FP1XP1  Low Gain Dynode Gated Rear Ion Vetoed; TDiff (ns); Energy (keV)",this->h2dsettings.at(11015));



	hismanager->RegisterPlot<TH2F>("BETA_3650","High Gain Dynode vs MTAS Total; MTAS Total Energy (keV); Dynode Energy (keV);",this->h2dsettings.at(3650));
	hismanager->RegisterPlot<TH2F>("BETA_3651","High Gain Dynode vs MTAS Center Sum; MTAS Center Energy (keV); Dynode Energy (keV);",this->h2dsettings.at(3651));
	hismanager->RegisterPlot<TH2F>("BETA_3652","High Gain Dynode vs MTAS Center Individual; MTAS Center Crystal Energy (keV); Dynode Energy (keV);",this->h2dsettings.at(3652));
	hismanager->RegisterPlot<TH2F>("BETA_36508","High Gain Dynode vs MTAS Total; MTAS Total Energy (8 keV/bin); Dynode Energy (8 keV/bin);",this->h2dsettings.at(36508));
	hismanager->RegisterPlot<TH2F>("BETA_36518","High Gain Dynode vs MTAS Center Sum; MTAS Center Energy (8 keV/bin); Dynode Energy (8 keV/bin);",this->h2dsettings.at(36518));
	hismanager->RegisterPlot<TH2F>("BETA_36528","High Gain Dynode vs MTAS Center Individual; MTAS Center Crystal Energy (8 keV/bin); Dynode Energy (8 keV/bin);",this->h2dsettings.at(36528));

	hismanager->RegisterPlot<TH1F>("BETA_2000","High Gain Dynode + MTAS Total; Energy (keV)",this->h1dsettings.at(2000));
	hismanager->RegisterPlot<TH1F>("BETA_2010","High Gain Dynode + MTAS Center Sum; Energy (keV)",this->h1dsettings.at(2010));
	hismanager->RegisterPlot<TH1F>("BETA_2020","High Gain Dynode + MTAS Inner Sum; Energy (keV)",this->h1dsettings.at(2020));
	hismanager->RegisterPlot<TH1F>("BETA_2030","High Gain Dynode + MTAS Middle Sum; Energy (keV)",this->h1dsettings.at(2030));
	hismanager->RegisterPlot<TH1F>("BETA_2040","High Gain Dynode + MTAS Outer Sum; Energy (keV)",this->h1dsettings.at(2040));
	hismanager->RegisterPlot<TH1F>("BETA_2015","High Gain Dynode + MTAS Center Ind.; Energy (keV)",this->h1dsettings.at(2015));
	hismanager->RegisterPlot<TH1F>("BETA_2025","High Gain Dynode + MTAS Inner Ind.; Energy (keV)",this->h1dsettings.at(2025));
	hismanager->RegisterPlot<TH1F>("BETA_2035","High Gain Dynode + MTAS Middle Ind.; Energy (keV)",this->h1dsettings.at(2035));
	hismanager->RegisterPlot<TH1F>("BETA_2045","High Gain Dynode + MTAS Outer Ind.; Energy (keV)",this->h1dsettings.at(2045));

	hismanager->RegisterPlot<TH2F>("ION_3750","Low Gain Dynode vs MTAS Total; MTAS Total Energy (keV); Dynode Energy (keV);",this->h2dsettings.at(3750));
	hismanager->RegisterPlot<TH2F>("ION_3751","Low Gain Dynode vs MTAS Center Sum; MTAS Center Energy (keV); Dynode Energy (keV);",this->h2dsettings.at(3751));
	hismanager->RegisterPlot<TH2F>("ION_3752","Low Gain Dynode vs MTAS Center Individual; MTAS Center Crystal Energy (keV); Dynode Energy (keV);",this->h2dsettings.at(3752));

	hismanager->RegisterPlot<TH2F>("ION_37508","Low Gain Dynode vs MTAS Total; MTAS Total Energy (8 keV/bin); Dynode Energy (8 keV/bin);",this->h2dsettings.at(37508));
	hismanager->RegisterPlot<TH2F>("ION_37518","Low Gain Dynode vs MTAS Center Sum; MTAS Center Energy (8 keV/bin); Dynode Energy (8 keV/bin);",this->h2dsettings.at(37518));
	hismanager->RegisterPlot<TH2F>("ION_37528","Low Gain Dynode vs MTAS Center Individual; MTAS Center Crystal Energy (8 keV/bin); Dynode Energy (8 keV/bin);",this->h2dsettings.at(37528));

	hismanager->RegisterPlot<TH1F>("ION_2100","High Gain Dynode + MTAS Total; Energy (keV)",this->h1dsettings.at(2100));
	hismanager->RegisterPlot<TH1F>("ION_2110","High Gain Dynode + MTAS Center Sum; Energy (keV)",this->h1dsettings.at(2110));
	hismanager->RegisterPlot<TH1F>("ION_2120","High Gain Dynode + MTAS Inner Sum; Energy (keV)",this->h1dsettings.at(2120));
	hismanager->RegisterPlot<TH1F>("ION_2130","High Gain Dynode + MTAS Middle Sum; Energy (keV)",this->h1dsettings.at(2130));
	hismanager->RegisterPlot<TH1F>("ION_2140","High Gain Dynode + MTAS Outer Sum; Energy (keV)",this->h1dsettings.at(2140));
	hismanager->RegisterPlot<TH1F>("ION_2115","High Gain Dynode + MTAS Center Ind.; Energy (keV)",this->h1dsettings.at(2115));
	hismanager->RegisterPlot<TH1F>("ION_2125","High Gain Dynode + MTAS Inner Ind.; Energy (keV)",this->h1dsettings.at(2125));
	hismanager->RegisterPlot<TH1F>("ION_2135","High Gain Dynode + MTAS Middle Ind.; Energy (keV)",this->h1dsettings.at(2135));
	hismanager->RegisterPlot<TH1F>("ION_2145","High Gain Dynode + MTAS Outer Ind.; Energy (keV)",this->h1dsettings.at(2145));

	hismanager->RegisterPlot<TH2F>("BETA_8000","Beta Radius vs Energy; Energy (keV); Radius (pixels)",this->h2dsettings.at(8000));
	hismanager->RegisterPlot<TH2F>("BETA_8001","Beta X vs Energy; Energy (keV); X (pixels)",this->h2dsettings.at(8001));
	hismanager->RegisterPlot<TH2F>("BETA_8002","Beta Y vs Energy; Energy (keV); Y (pixels)",this->h2dsettings.at(8002));
	hismanager->RegisterPlot<TH2F>("BETA_8003","Beta Image ; X (pixel); Y (pixels)",this->h2dsettings.at(8003));

	for( size_t ii = 0; ii < this->MTAS_Total_Gates.size(); ++ii ){
		std::string label = "BETA_900"+std::to_string(ii);
		std::string title = "Beta Image Gated on MTAS Total [";
		title += std::to_string(this->MTAS_Total_Gates.at(ii).GetLowerBound())+","+std::to_string(this->MTAS_Total_Gates.at(ii).GetUpperBound());
		title += "; X (pixel); Y (pixel)";
		hismanager->RegisterPlot<TH2F>(label,title,this->h2dsettings.at(9000));
	}

	for( size_t ii = 0; ii < this->isotopetags.size(); ++ii ){
		std::string label = "ION_600"+std::to_string(ii);
		std::string title = "Ion Image Gated on "+this->isotopetags.at(ii)+"; X (pixel); Y (pixel)";
		hismanager->RegisterPlot<TH2F>(label,title,this->h2dsettings.at(6000));

		//decay curves
		label = "DECAY_4000"+std::to_string(ii);
		title = "Beta - Ion TDiff Gated on "+this->isotopetags.at(ii)+"; TDiff (us)";
		hismanager->RegisterPlot<TH1F>(label,title,this->h1dsettings.at(4000));

		label = "DECAY_4001"+std::to_string(ii);
		title = "Beta - Ion TDiff Gated on "+this->isotopetags.at(ii)+"; TDiff (ms)";
		hismanager->RegisterPlot<TH1F>(label,title,this->h1dsettings.at(4001));

		label = "DECAY_4002"+std::to_string(ii);
		title = "Beta - Ion TDiff Gated on "+this->isotopetags.at(ii)+"; TDiff (s)";
		hismanager->RegisterPlot<TH1F>(label,title,this->h1dsettings.at(4002));

		label = "DECAY_4003"+std::to_string(ii);
		title = "Beta - Ion TDiff Gated on "+this->isotopetags.at(ii)+"; TDiff (min)";
		hismanager->RegisterPlot<TH1F>(label,title,this->h1dsettings.at(4003));

		label = "DECAY_8000"+std::to_string(ii);
		title = "Beta - Ion Radius Gated on "+this->isotopetags.at(ii)+"; Radius (arb.)";
		hismanager->RegisterPlot<TH1F>(label,title,this->h1dsettings.at(8000));

		label = "DECAY_5000"+std::to_string(ii);
		title = "Beta - Ion TDiff vs Radius Gated on "+this->isotopetags.at(ii)+"; Radius (arb.); TDiff (us)";
		hismanager->RegisterPlot<TH2F>(label,title,this->h2dsettings.at(5000));                             

		label = "DECAY_5001"+std::to_string(ii);                                                            
		title = "Beta - Ion TDiff vs Radius Gated on "+this->isotopetags.at(ii)+"; Radius (arb.); TDiff (ms)";
		hismanager->RegisterPlot<TH2F>(label,title,this->h2dsettings.at(5001));                             

		label = "DECAY_5002"+std::to_string(ii);                                                            
		title = "Beta - Ion TDiff vs Radius Gated on "+this->isotopetags.at(ii)+"; Radius (arb.); TDiff (s)";
		hismanager->RegisterPlot<TH2F>(label,title,this->h2dsettings.at(5002));                             

		label = "DECAY_5003"+std::to_string(ii);                                                            
		title = "Beta - Ion TDiff vs Radius Gated on "+this->isotopetags.at(ii)+"); Radius (arb.); TDiff (min)";
		hismanager->RegisterPlot<TH2F>(label,title,this->h2dsettings.at(5003));

		label = "DECAY_7000"+std::to_string(ii);
		title = "Beta - Ion Radius vs Beta Energy Gated on "+this->isotopetags.at(ii)+"; Energy (keV); Radius (arb.)";
		hismanager->RegisterPlot<TH2F>(label,title,this->h2dsettings.at(7000));

		label = "DECAY_7001"+std::to_string(ii);
		title = "Beta - Ion Radius vs Ion Energy Gated on "+this->isotopetags.at(ii)+"; Energy (keV); Radius (arb.)";
		hismanager->RegisterPlot<TH2F>(label,title,this->h2dsettings.at(7001));
	}
	//this->isotopetags = this->PidProc->GetIsotopeTags();

	hismanager->RegisterPlot<TH2F>("ION_8005","Ion Radius vs Energy; Energy (keV); Radius (pixels)",this->h2dsettings.at(8005));
	hismanager->RegisterPlot<TH2F>("ION_8006","Ion X vs Energy; Energy (keV); X (pixels)",this->h2dsettings.at(8006));
	hismanager->RegisterPlot<TH2F>("ION_8007","Ion Y vs Energy; Energy (keV); Y (pixels)",this->h2dsettings.at(8007));
	hismanager->RegisterPlot<TH2F>("ION_8008","Ion Image ; X (pixel); Y (pixels)",this->h2dsettings.at(8008));

	hismanager->RegisterPlot<TH2F>("ISOMER_3700","Mtas prev-#beta curr-no-#beta; Energy (keV); Time (ns)",this->h2dsettings.at(3700));
	hismanager->RegisterPlot<TH2F>("ISOMER_3701","Mtas prev-#beta curr-no-#beta; Energy (keV); Time (us)",this->h2dsettings.at(3701));

	hismanager->RegisterPlot<TH2F>("ISOMER_3800","Mtas prev-#gamma curr-#beta; Energy (keV); Time (ns)",this->h2dsettings.at(3800));
	hismanager->RegisterPlot<TH2F>("ISOMER_3801","Mtas prev-#gamma curr-#beta; Energy (keV); Time (us)",this->h2dsettings.at(3801));

	for( const auto& t : this->isotopetags ){
		this->console->info("Found Isotope Tag : {}",t);
		this->implant_isotopes[t] = 0;
		this->rit_vetoed_isotopes[t] = 0;
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

e21027Processor::~e21027Processor(){
	auto tdiff = this->LastTime - this->FirstTime;
	auto cumulative_total = 0;
	auto cumulative_implant = 0;
	auto cumulative_rit_veto = 0;
	for( const auto& t : this->isotopetags ){
		auto total = this->PidProc->GetNumIsotopes(t);
		cumulative_total += total;
		auto total_rate = static_cast<double>(total)/tdiff;

		auto implant = this->implant_isotopes[t];
		cumulative_implant += implant;
		double implant_eff = static_cast<double>(implant)/static_cast<double>(total);
		auto implant_rate = static_cast<double>(implant)/tdiff;

		auto rit_veto = this->rit_vetoed_isotopes[t];
		cumulative_rit_veto += rit_veto;
		double rit_veto_eff = static_cast<double>(rit_veto)/static_cast<double>(total);
		auto rit_veto_rate = static_cast<double>(rit_veto)/tdiff;

		this->console->info("{} Counts : Total/Implant/RitVeto  : {}/{}/{} ",t,total,implant,rit_veto);
		this->console->info("{} Efficiency : Implant/RitVeto : {:.4f}/{:.4f}",t,implant_eff,rit_veto_eff);
		this->console->info("{} Rate (pps) : Total/Implant/RitVeto : {:.4f}/{:.4f}/{:.4f}",t,total_rate,implant_rate,rit_veto_rate);
		this->console->info("{} Rate (pph) : Total/Implant/RitVeto : {:.4f}/{:.4f}/{:.4f}",t,total_rate*60,implant_rate*60,rit_veto_rate*60);
	}
}


void e21027Processor::AddIonToCorrelation(EventHistoryManager* eventhistory,PLOTS::PlotRegistry* hismanager,CUTS::CutRegistry* cutmanager){
	auto numhist = eventhistory->GetMaxHistoryID();
	auto summary = eventhistory->GetCurrentEventSummary();
	auto hasrit = summary->ContainsEventTag("rit");

	for (size_t iPins = 0 ; iPins < this->PidProc->GetNumFP1Pins(); ++iPins){
		hismanager->Fill("EXP_" + std::to_string(10000 +iPins ), this->PidProc->GetFP1Tof(0),this->PidProc->GetFP1PinEnergy(iPins));
		hismanager->Fill("EXP_" + std::to_string(10004 +iPins ), this->PidProc->GetFP1Tof(2),this->PidProc->GetFP1PinEnergy(iPins));
		hismanager->Fill("EXP_" + std::to_string(10008 +iPins ), this->PidProc->GetFP1Tof(4),this->PidProc->GetFP1PinEnergy(iPins));
		hismanager->Fill("EXP_" + std::to_string(10012 +iPins ), this->PidProc->GetFP1Tof(6),this->PidProc->GetFP1PinEnergy(iPins));
	}

	for( size_t ii = 0; ii < this->isotopetags.size(); ++ii ){
		if( summary->ContainsEventTag(this->isotopetags.at(ii)) ){
			++(this->implant_isotopes[this->isotopetags[ii]]);
		}
	}

	if( not hasrit ){
		for (size_t iPins = 0 ; iPins < this->PidProc->GetNumFP1Pins(); ++iPins){
			hismanager->Fill("EXP_" + std::to_string(11000 +iPins ), this->PidProc->GetFP1Tof(0),this->PidProc->GetFP1PinEnergy(iPins));
			hismanager->Fill("EXP_" + std::to_string(11004 +iPins ), this->PidProc->GetFP1Tof(2),this->PidProc->GetFP1PinEnergy(iPins));
			hismanager->Fill("EXP_" + std::to_string(11008 +iPins ), this->PidProc->GetFP1Tof(4),this->PidProc->GetFP1PinEnergy(iPins));
			hismanager->Fill("EXP_" + std::to_string(11012 +iPins ), this->PidProc->GetFP1Tof(6),this->PidProc->GetFP1PinEnergy(iPins));
		}

		for( size_t ii = 0; ii < this->isotopetags.size(); ++ii ){
			if( summary->ContainsEventTag(this->isotopetags.at(ii)) ){
				++(this->rit_vetoed_isotopes[this->isotopetags[ii]]);
			}
		}


		auto total = this->MtasProc->GetTotalEnergy(0);
		auto ctotal = this->MtasProc->GetTotalEnergy(1);
		auto itotal = this->MtasProc->GetTotalEnergy(2);
		auto mtotal = this->MtasProc->GetTotalEnergy(3);
		auto ototal = this->MtasProc->GetTotalEnergy(4);
		auto dynode = this->ImplantProc->GetLowGainImage().dynode;
		hismanager->Fill("ION_3750",total,dynode);
		hismanager->Fill("ION_3751",ctotal,dynode);
		hismanager->Fill("ION_37508",total,dynode);
		hismanager->Fill("ION_37518",ctotal,dynode);
		hismanager->Fill("ION_2100",dynode+total);
		hismanager->Fill("ION_2110",dynode+ctotal);
		hismanager->Fill("ION_2120",dynode+itotal);
		hismanager->Fill("ION_2130",dynode+mtotal);
		hismanager->Fill("ION_2140",dynode+ototal);
		for( size_t ii = 0; ii < 6; ++ii ){
			hismanager->Fill("ION_3752",this->MtasProc->GetCrystalEnergy(ii),dynode);
			hismanager->Fill("ION_37528",this->MtasProc->GetCrystalEnergy(ii),dynode);

			hismanager->Fill("ION_2115",dynode+this->MtasProc->GetCrystalEnergy(ii));
			hismanager->Fill("ION_2125",dynode+this->MtasProc->GetCrystalEnergy(ii+6));
			hismanager->Fill("ION_2135",dynode+this->MtasProc->GetCrystalEnergy(ii+12));
			hismanager->Fill("ION_2145",dynode+this->MtasProc->GetCrystalEnergy(ii+18));
		}

		auto ionx = summary->GetEventObservable("ION_X").value();
		auto iony = summary->GetEventObservable("ION_Y").value();
		auto ionr = std::sqrt(ionx*ionx + iony*iony);
		hismanager->Fill("ION_8005",dynode,ionr);
		hismanager->Fill("ION_8006",dynode,ionx);
		hismanager->Fill("ION_8007",dynode,iony);
		hismanager->Fill("ION_8008",ionx,iony);
		for( size_t ii = 0; ii < this->isotopetags.size(); ++ii ){
			if( summary->ContainsEventTag(this->isotopetags.at(ii)) ){
				std::string title = "ION_600"+std::to_string(ii);
				hismanager->Fill(title,ionx,iony);
			}
		}
		////found new ion, need to add it to the limit list
		////and then correlate it with all known betas
		//const auto ion_idx = static_cast<unsigned long long>(summary->GetEventObservable("Event_idx").value());
		auto ion_ts = summary->GetEventObservable("ION_TS").value();
		//use the boost::circular_buffer to queue the things
		//search through the summaries previous and we'll grab their idx
		if( numhist > 1 ){
			//if( numhist <= 1000 ){
				this->IonCorrelationHelper(eventhistory,hismanager,cutmanager,1,numhist,summary,dynode,ion_ts,ionx,iony);
			//}else{
			//	//have a shit load, need to split into parallel operations
			//	//over NThread workers
			//	std::vector<std::thread> Workers;
			//	for( size_t n = 0; n < this->NThreads; ++ n ){
			//		//0 : 0 - min(numhist/NThreads,numhist)
			//		//1 : numhist/NThreads + 1 - min(2*numhist/NThreads,numhist)
			//		//.... 
			//		//n : n*numhist/NThreads + 1 - min((n+1)*numhist/NThreads,numhist)
			//		size_t startidx = n*(numhist/NThreads);
			//		size_t stopidx = std::min((n+1)*(numhist/NThreads),numhist);
			//		//likely need a mutex put into the histogram filling
			//		Workers.push_back(std::thread(&e21027Processor::IonCorrelationHelper,this,eventhistory,hismanager,cutmanager,startidx,stopidx,summary,dynode,ion_ts,ionx,iony));
			//	}
			//	for( auto&& w : Workers ){
			//		w.join();
			//	}

			//}
		}
	}
}

void e21027Processor::AddBetaToCorrelation(EventHistoryManager* eventhistory,PLOTS::PlotRegistry* hismanager,CUTS::CutRegistry* cutmanager){
	auto summary = eventhistory->GetCurrentEventSummary();
	auto numhist = eventhistory->GetMaxHistoryID();
	this->MtasProc->FillBetaPlots(hismanager);
	this->MtasProc->FillNoLogicBetaPlots(hismanager);
	auto total = this->MtasProc->GetTotalEnergy(0);
	auto ctotal = this->MtasProc->GetTotalEnergy(1);
	auto itotal = this->MtasProc->GetTotalEnergy(2);
	auto mtotal = this->MtasProc->GetTotalEnergy(3);
	auto ototal = this->MtasProc->GetTotalEnergy(4);
	auto dynode = this->ImplantProc->GetHighGainImage().dynode;
	hismanager->Fill("BETA_3650",total,dynode);
	hismanager->Fill("BETA_3651",ctotal,dynode);
	hismanager->Fill("BETA_36508",total,dynode);
	hismanager->Fill("BETA_36518",ctotal,dynode);
	hismanager->Fill("BETA_2000",dynode+total);
	hismanager->Fill("BETA_2010",dynode+ctotal);
	hismanager->Fill("BETA_2020",dynode+itotal);
	hismanager->Fill("BETA_2030",dynode+mtotal);
	hismanager->Fill("BETA_2040",dynode+ototal);
	for( size_t ii = 0; ii < 6; ++ii ){
		hismanager->Fill("BETA_3652",this->MtasProc->GetCrystalEnergy(ii),dynode);
		hismanager->Fill("BETA_36528",this->MtasProc->GetCrystalEnergy(ii),dynode);

		hismanager->Fill("BETA_2015",dynode+this->MtasProc->GetCrystalEnergy(ii));
		hismanager->Fill("BETA_2025",dynode+this->MtasProc->GetCrystalEnergy(ii+6));
		hismanager->Fill("BETA_2035",dynode+this->MtasProc->GetCrystalEnergy(ii+12));
		hismanager->Fill("BETA_2045",dynode+this->MtasProc->GetCrystalEnergy(ii+18));
	}

	auto betax = summary->GetEventObservable("BETA_X").value();
	auto betay = summary->GetEventObservable("BETA_Y").value();
	auto betar = std::sqrt(betax*betax + betay*betay);
	//this->console->info("X:{}, Y:{}, R:{}",betax,betay,betar); 
	hismanager->Fill("BETA_8000",dynode,betar);
	hismanager->Fill("BETA_8001",dynode,betax);
	hismanager->Fill("BETA_8002",dynode,betay);
	hismanager->Fill("BETA_8003",betax,betay);
	for( size_t ii = 0; ii < this->MTAS_Total_Gates.size(); ++ii ){
		if( this->MTAS_Total_Gates.at(ii).IsWithin(total) ){
			std::string label = "BETA_900"+std::to_string(ii);
			hismanager->Fill(label,betax,betay);
		}
	}
	//found new beta, need to go through the known ion list and correlate it with us
	//and update their secondary
	const auto beta_ts = summary->GetEventObservable("BETA_TS").value();
	if( numhist > 1 ){
		//if( numhist <= 1000 ){
		      this->BetaCorrelationHelper(eventhistory,hismanager,cutmanager,1,numhist,dynode,beta_ts,betax,betay);
		//}else{
		//      //have a shit load, need to split into parallel operations
		//      //over NThread workers
		//      //give each thread numhist/NThread tasks? or batch out 1k to each?
		//      std::vector<std::thread> Workers;
		//      for( size_t n = 0; n < this->NThreads; ++ n ){
		//	      //0 : 0 - min(numhist/NThreads,numhist)
		//	      //1 : numhist/NThreads + 1 - min(2*numhist/NThreads,numhist)
		//	      //.... 
		//	      //n : n*numhist/NThreads + 1 - min((n+1)*numhist/NThreads,numhist)
		//	      size_t startidx = n*(numhist/NThreads);
		//	      size_t stopidx = std::min((n+1)*(numhist/NThreads),numhist);
		//	      //likely need a mutex put into the histogram filling
		//	      Workers.push_back(std::thread(&e21027Processor::BetaCorrelationHelper,this,eventhistory,hismanager,cutmanager,startidx,stopidx,dynode,beta_ts,betax,betay));
		//      }
		//      for( auto&& w : Workers ){
		//	      w.join();
		//      }
		//}
	}
}

void e21027Processor::DoIsomerCorrelation(EventHistoryManager* eventhistory,PLOTS::PlotRegistry* hismanager,CUTS::CutRegistry* cutmanager){
	const auto numhist = eventhistory->GetMaxHistoryID();
	const auto summary = eventhistory->GetCurrentEventSummary();
	const auto erg = MtasProc->GetTotalEnergy(0);

	const auto hasbeta = summary->ContainsEventTag(this->beta);
	const auto hasgamma = summary->ContainsEventTag(this->gamma);

	//search through the old indices to find the delayed gamma from a beta
	for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
		const auto prevsummary = eventhistory->GetPreviousEventSummary(ii);
		const auto prevmuon = prevsummary->ContainsEventTag("muon");
		if( prevmuon ){
			continue;
		}
		const auto prevbeta = prevsummary->ContainsEventTag(this->beta);
		const auto prevgamma = prevsummary->ContainsEventTag(this->gamma);
		if( not hasbeta and hasgamma and prevbeta ){
			const auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
			hismanager->Fill("ISOMER_3700",erg,isomer_tdiff);
			hismanager->Fill("ISOMER_3701",erg,isomer_tdiff*1.0e-3);
			break;
		}
	}
	for( size_t ii = 1; ii < eventhistory->GetMaxHistoryID(); ++ii ){
		const auto prevsummary =  eventhistory->GetPreviousEventSummary(ii);
		const auto prevmuon = prevsummary->ContainsEventTag("muon");
		if( prevmuon ){
			continue;
		}
		const auto prevbeta = prevsummary->ContainsEventTag(this->beta);
		const auto prevgamma = prevsummary->ContainsEventTag(this->gamma);

		//this looks for a gamma decay into a delayed beta
		//i.e. beam isomer, but need mtas energy for this old event
		if( hasbeta and not prevbeta and prevgamma ){
			const auto isomer_tdiff = summary->GetRawEvents().front().GetTimeStamp() - prevsummary->GetRawEvents().front().GetTimeStamp();
			const auto olderg = prevsummary->GetEventObservable("MTAS_Total").value();
			hismanager->Fill("ISOMER_3800",olderg,isomer_tdiff);
			hismanager->Fill("ISOMER_3801",olderg,isomer_tdiff*1.0e-3);
			break;
		}
	}
}

void e21027Processor::IonCorrelationHelper(EventHistoryManager* eventhistory,PLOTS::PlotRegistry* hismanager,CUTS::CutRegistry* cutmanager,size_t startidx,size_t stopidx,const EventSummary* summary,double dynode,double ion_ts,double ionx,double iony){
	for( size_t ii = startidx; ii < stopidx; ++ii ){
		const auto prevsummary = eventhistory->GetPreviousEventSummary(ii);
		//const auto preveventidx = static_cast<unsigned long long>(prevsummary->GetEventObservable("Event_idx").value());
		const auto isprevbeta = prevsummary->ContainsEventTag(this->beta);
		if( isprevbeta ){
			const auto beta_erg = prevsummary->GetEventObservable("BETA_Energy").value();
			const auto beta_ts = prevsummary->GetEventObservable("BETA_TS").value();
			const auto beta_ion_tdiff_s = 1.0e-9*(beta_ts - ion_ts);
			const auto betax = prevsummary->GetEventObservable("BETA_X").value();
			const auto betay = prevsummary->GetEventObservable("BETA_Y").value();
			const auto xdiff = ionx - betax;
			const auto ydiff = iony - betay;
			const auto beta_ion_radius = std::sqrt(xdiff*xdiff + ydiff*ydiff);
			//determine which tdiff plot to fill
			//these are all the negative time portions of the tdiff
			for( size_t jj = 0; jj < this->isotopetags.size(); ++jj ){
				if( summary->ContainsEventTag(this->isotopetags[jj]) ){
					std::string label = "DECAY_4000"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_tdiff_s*1.0e6);

					label = "DECAY_4001"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_tdiff_s*1.0e3);

					label = "DECAY_4002"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_tdiff_s);

					label = "DECAY_4003"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_tdiff_s/60.0);

					label = "DECAY_8000"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_radius);

					label = "DECAY_5000"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_radius,beta_ion_tdiff_s*1.0e6);

					label = "DECAY_5001"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_radius,beta_ion_tdiff_s*1.0e3);

					label = "DECAY_5002"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_radius,beta_ion_tdiff_s);

					label = "DECAY_5003"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_radius,beta_ion_tdiff_s/60.0);

					//dynode contains the ion in this case
					label = "DECAY_7001"+std::to_string(jj);
					hismanager->Fill(label,dynode,beta_ion_radius);

					label = "DECAY_7000"+std::to_string(jj);
					hismanager->Fill(label,beta_erg,beta_ion_radius);
				}
			}
		}
	}

}

void e21027Processor::BetaCorrelationHelper(EventHistoryManager* eventhistory,PLOTS::PlotRegistry* hismanager,CUTS::CutRegistry* cutmanager,size_t beginidx,size_t stopidx,double dynode,double beta_ts,double betax,double betay){
	for( size_t ii = beginidx; ii < stopidx; ++ii ){
		const auto prevsummary = eventhistory->GetPreviousEventSummary(ii);
		//const auto preveventidx = static_cast<unsigned long long>(prevsummary->GetEventObservable("Event_idx").value());
		const auto isprevion = prevsummary->ContainsEventTag(this->implant);
		const auto isprevrit = prevsummary->ContainsEventTag("rit");
		if( isprevion and not isprevrit ){
			const auto ion_erg = prevsummary->GetEventObservable("ION_Energy").value();
			const auto ion_ts = prevsummary->GetEventObservable("ION_TS").value();
			const auto beta_ion_tdiff_s = 1.0e-9*(beta_ts - ion_ts);
			const auto ionx = prevsummary->GetEventObservable("ION_X").value();
			const auto iony = prevsummary->GetEventObservable("ION_Y").value();
			const auto xdiff = ionx - betax;
			const auto ydiff = iony - betay;
			const auto beta_ion_radius = std::sqrt(xdiff*xdiff + ydiff*ydiff);
			//determine which tdiff plot to fill
			//these are all the negative time portions of the tdiff
			for( size_t jj = 0; jj < this->isotopetags.size(); ++jj ){
				if( prevsummary->ContainsEventTag(this->isotopetags[jj]) ){
					std::string label = "DECAY_4000"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_tdiff_s*1.0e6);

					label = "DECAY_4001"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_tdiff_s*1.0e3);

					label = "DECAY_4002"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_tdiff_s);

					label = "DECAY_4003"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_tdiff_s/60.0);

					label = "DECAY_8000"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_radius);

					label = "DECAY_5000"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_radius,beta_ion_tdiff_s*1.0e6);

					label = "DECAY_5001"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_radius,beta_ion_tdiff_s*1.0e3);

					label = "DECAY_5002"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_radius,beta_ion_tdiff_s);

					label = "DECAY_5003"+std::to_string(jj);
					hismanager->Fill(label,beta_ion_radius,beta_ion_tdiff_s/60.0);

					//dynode contains the beta in this case
					label = "DECAY_7000"+std::to_string(jj);
					hismanager->Fill(label,dynode,beta_ion_radius);

					label = "DECAY_7001"+std::to_string(jj);
					hismanager->Fill(label,ion_erg,beta_ion_radius);

				}
			}
		}
	}
}

//void e21027Processor::IsomerCorrelationHelper(EventHistoryManager* eventhistory,PLOTS::PlotRegistry* hismanager,CUTS::CutRegistry* cutmanager){
//}
