#include "e21027Processor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>

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

	// this->h1dsettings = {
	// };

	this->h2dsettings = {
		{10000,{10000,-1000,10000,16000,0,16000}},
		{10001,{10000,-1000,10000,16000,0,16000}},
		{10002,{10000,-1000,10000,16000,0,16000}},
		{10003,{10000,-1000,10000,16000,0,16000}},
		{10004,{10000,-1000,10000,16000,0,16000}},
		{10005,{10000,-1000,10000,16000,0,16000}},
		{10006,{10000,-1000,10000,16000,0,16000}},
		{10007,{10000,-1000,10000,16000,0,16000}},
		{10008,{10000,-1000,10000,16000,0,16000}},
		{10009,{10000,-1000,10000,16000,0,16000}},
		{10010,{10000,-1000,10000,16000,0,16000}},
		{10011,{10000,-1000,10000,16000,0,16000}},
		{10012,{10000,-1000,10000,16000,0,16000}},
		{10013,{10000,-1000,10000,16000,0,16000}},
		{10014,{10000,-1000,10000,16000,0,16000}},
		{10015,{10000,-1000,10000,16000,0,16000}}
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

	if( hasion ){
		for (unsigned int iPins = 0 ; iPins < this->PidProc->GetFP1().pin.size(); ++iPins){
			hismanager->Fill("EXP_" + std::to_string(10000 +iPins ), this->PidProc->GetFP1Tofs().at(0),this->PidProc->GetFP1().pin.at(iPins).energy);
			hismanager->Fill("EXP_" + std::to_string(10004 +iPins ), this->PidProc->GetFP1Tofs().at(2),this->PidProc->GetFP1().pin.at(iPins).energy);
			hismanager->Fill("EXP_" + std::to_string(10008 +iPins ), this->PidProc->GetFP1Tofs().at(4),this->PidProc->GetFP1().pin.at(iPins).energy);
			hismanager->Fill("EXP_" + std::to_string(10012 +iPins ), this->PidProc->GetFP1Tofs().at(6),this->PidProc->GetFP1().pin.at(iPins).energy);
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

	//if( hasgamma and not hasbeta ){
	//	//search through the old indices to find the delayed gamma from a beta
	//}
	
	if( hasbeta ){
		this->MtasProc->FillBetaPlots(hismanager);
		this->MtasProc->FillNoLogicBetaPlots(hismanager);
		//found new beta, need to go through the known ion list and correlate it with us
		//and update their secondary
	}else{
		this->MtasProc->FillNonBetaPlots(hismanager);
		this->MtasProc->FillNoLogicNonBetaPlots(hismanager);
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
