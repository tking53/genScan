#include "e21027Processor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>

e21027Processor::e21027Processor(const std::string& log) : Processor(log,"e21027Processor",{}){
	this->MtasProc = std::make_shared<MtasProcessor>(log);
	this->ImplantProc = std::make_shared<MtasImplantProcessor>(log);
	this->PidProc = std::make_shared<PidProcessor>(log);

	for( const auto& type : this->MtasProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	for( const auto& type : this->ImplantProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	for( const auto& type : this->PidProc->GetKnownTypes() ){
		this->AssociateType(type);
	}

	// this->h1dsettings = {
	// };

	this->h2dsettings = {
		{10000,{10000,-1000,10000,16000,0,16000}}
	};

	this->beta = "beta";
	this->gamma = "gamma";
	this->implant = "implant";

	this->HasMTAS = false;
	this->HasSIPM = false;

	this->ImplantThreshold = 0.0;

	this->Reset();
}

[[maybe_unused]] bool e21027Processor::PreProcess(EventHistoryManager* eventhistory, PLOTS::PlotRegistry* hismanager, CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	auto summary = eventhistory->GetCurrentEventSummary();
	auto types = summary->GetKnownTypes();

	this->HasMTAS = (types.find("mtas") != types.end());
	this->HasSIPM = (types.find("mtasimplant") != types.end()); 
	this->HasPID = (types.find("pid") != types.end()); 

	if( this->HasSIPM ){
		this->ImplantProc->PreProcess(eventhistory,hismanager,cutmanager);
		auto lgImage = this->ImplantProc->GetLowGainImage();
		if( lgImage.anodesum > this->ImplantThreshold ){
			summary->AddEventTag(this->implant);
		}
	}

	if( this->HasPID ){
		this->PidProc->PreProcess(eventhistory,hismanager,cutmanager);
	}

	if( this->HasMTAS ){
		this->MtasProc->PreProcess(eventhistory,hismanager,cutmanager);
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

	if (this->HasSIPM){	
		if (this->ImplantProc->GetLowGainImage().dynode > this->ImplantThreshold){
				for (unsigned int iPins = 0 ; iPins < this->PidProc->GetFP1().pin.size(); ++iPins){
					hismanager->Fill("EXP_" + std::to_string(10000 +iPins ), this->PidProc->GetFP1Tofs().at(0),this->PidProc->GetFP1().pin.at(iPins).energy);
					hismanager->Fill("EXP_" + std::to_string(10004 +iPins ), this->PidProc->GetFP1Tofs().at(2),this->PidProc->GetFP1().pin.at(iPins).energy);
					hismanager->Fill("EXP_" + std::to_string(10008 +iPins ), this->PidProc->GetFP1Tofs().at(4),this->PidProc->GetFP1().pin.at(iPins).energy);
					hismanager->Fill("EXP_" + std::to_string(10012 +iPins ), this->PidProc->GetFP1Tofs().at(6),this->PidProc->GetFP1().pin.at(iPins).energy);
			}
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
		}else{
			throw std::runtime_error("unknown subprocessor declared in e21027Processor");
		}
	}

	this->ImplantThreshold = config.attribute("implantthresh").as_double(0.0);

	if( not this->HasPID ){
		throw std::runtime_error("missing PidProcessor in e21027Processor");
	}

	if( not this->HasMTAS ){
		throw std::runtime_error("missing MtasProcessor in e21027Processor");
	}

	if( not this->HasSIPM ){
		throw std::runtime_error("missing MtasImplantProcessor in e21027Processor");
	}

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}
		
void e21027Processor::Finalize(){
	this->MtasProc->Finalize();
	this->ImplantProc->Finalize();

	this->console->info("{} has been finalized",this->ProcessorName);
}

void e21027Processor::DeclarePlots(PLOTS::PlotRegistry* hismanager){
	this->MtasProc->DeclarePlots(hismanager);
	this->ImplantProc->DeclarePlots(hismanager);
	this->PidProc->DeclarePlots(hismanager);

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

	this->console->info("Finished Declaring Plots");
}

void e21027Processor::RegisterTree(std::unordered_map<std::string,TTree*>& outputtrees){
	this->MtasProc->RegisterTree(outputtrees);
	this->ImplantProc->RegisterTree(outputtrees);
}

void e21027Processor::CleanupTree(){
	this->MtasProc->CleanupTree();
	this->ImplantProc->CleanupTree();
}

void e21027Processor::Reset(){
}
