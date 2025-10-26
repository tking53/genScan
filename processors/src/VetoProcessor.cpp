#include "VetoProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "Gates.hpp"
#include "HistogramManager.hpp"
#include "VetoStruct.hpp"
#include <TTree.h>
#include <stdexcept>

VetoProcessor::VetoProcessor(const std::string& log) : Processor(log,"VetoProcessor",{"veto"}){
	this->h1dsettings = {
		{1000,{65536,0,65536}},
		{1010,{65536,0,65536}},
		{2000,{65536,0,65536}},
		{2010,{65536,0,65536}}
	};

	this->h2dsettings = {
		{3000,{4096,0,4096,4096,0,4096}},
		{30008,{4096,0,4096,4096,0,4096}},
		{4000,{4096,0,65536,4096,-15,15}}
	};

	this->rit = 0.0;
	this->fit = 0.0;
	this->currsubtype = SUBTYPE::UNKNOWN;

	this->rit_root = ProcessorStruct::DEFAULT_VETO_STRUCT;
	this->fit_root = ProcessorStruct::DEFAULT_VETO_STRUCT;
}

[[maybe_unused]] bool VetoProcessor::PreProcess(EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();
	auto summary = 	eventhistory->GetCurrentEventSummary();
	summary->GetDetectorSummary(this->AllDefaultRegex["veto"],this->SummaryData);

	for( const auto& evt : this->SummaryData ){
		auto subtype = evt->GetSubType();
		if( subtype.compare("rit") == 0 ){
			this->currsubtype = SUBTYPE::RIT;
		}else if( subtype.compare("fit") == 0 ){
			this->currsubtype = SUBTYPE::FIT;
		}else{
			throw std::runtime_error("xml has invalid veto subtype, valid are rit or fit");
		}
		auto currgroup = evt->GetGroup();
		int detloc = std::stoi(currgroup);
		if( detloc > 1 ){
			throw std::runtime_error("xml has malformed group for a veto, only 0,1 are allowed");
		}
		
		if( this->currsubtype == SUBTYPE::FIT ){
			if( evt->GetEnergy() > this->fit ){
				this->fit = evt->GetEnergy();
				auto [head,tail,total] = evt->GetTraceFixedPSD();
				this->fit_psd = head/tail; 
				this->fit_root.energy = evt->GetEnergy();
				this->fit_root.pileup = evt->GetPileup();
				this->fit_root.saturate = evt->GetSaturation();
				this->fit_root.timestamp = evt->GetTimeStamp();
				this->fit_root.head = head;
				this->fit_root.tail = tail;
			}
			hismanager->Fill("VETO_1010",evt->GetEnergy());
		}else if( this->currsubtype == SUBTYPE::RIT ){
			if( evt->GetEnergy() > this->rit ){
				this->rit = evt->GetEnergy();
				auto [head,tail,total] = evt->GetTraceFixedPSD();
				this->rit_psd = head/tail; 
				this->rit_root.energy = evt->GetEnergy();
				this->rit_root.pileup = evt->GetPileup();
				this->rit_root.saturate = evt->GetSaturation();
				this->rit_root.timestamp = evt->GetTimeStamp();
				this->rit_root.head = head;
				this->rit_root.tail = tail;
			}
			hismanager->Fill("VETO_2010",evt->GetEnergy());
		}else{
			//no-op
		}
	}


	for( const auto& g : this->FitReject ){
		if( g.IsWithin(this->fit) ){
			summary->AddEventTag("fit");
			break;
		}
	}
	
	for( const auto& g : this->RitReject ){
		if( g.IsWithin(this->rit) ){
			summary->AddEventTag("rit");
			break;
		}
	}
	
	hismanager->Fill("VETO_1000",this->fit);
	hismanager->Fill("VETO_2000",this->rit);
	hismanager->Fill("VETO_3000",this->rit,this->fit);
	hismanager->Fill("VETO_30008",this->rit,this->fit);
	hismanager->Fill("VETO_4000",this->rit,this->rit_psd);

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool VetoProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool VetoProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->Reset();
	return true;
}

void VetoProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");

	for( pugi::xml_node gate = config.child("Gate"); gate; gate = gate.next_sibling("Gate") ){
		std::string label = gate.attribute("label").as_string("");
		if( label.compare("fit") == 0 ){
			this->FitReject.push_back(Gate<double>(gate.attribute("lowerbound").as_double(-1.0),gate.attribute("upperbound").as_double(-1.0)));
		}else if( label.compare("rit") == 0 ){
			this->RitReject.push_back(Gate<double>(gate.attribute("lowerbound").as_double(-1.0),gate.attribute("upperbound").as_double(-1.0)));
		}else{
			this->console->error("Only accepted gates are label=\"rit\" or label=\"fit\", and these are the regions which will be tagged, multiple are allowed");
			throw std::runtime_error("Unknown Gate Tag label");
		}
	}
	
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}
		
void VetoProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void VetoProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager){
	hismanager->RegisterPlot<TH1F>("VETO_1000","Max Front Veto; Energy (arb.);",this->h1dsettings.at(1000));
	hismanager->RegisterPlot<TH1F>("VETO_1010","Max Front Veto; Energy (arb.);",this->h1dsettings.at(1010));

	hismanager->RegisterPlot<TH1F>("VETO_2000","Max Rear Veto; Energy (arb.);",this->h1dsettings.at(2000));
	hismanager->RegisterPlot<TH1F>("VETO_2010","Max Rear Veto; Energy (arb.);",this->h1dsettings.at(2010));

	hismanager->RegisterPlot<TH2F>("VETO_3000","Max Front Veto vs Max Rear Veto; Energy (arb.); Energy (arb.)",this->h2dsettings.at(3000));
	hismanager->RegisterPlot<TH2F>("VETO_30008","Max Front Veto vs Max Rear Veto; Energy (arb.); Energy (arb.)",this->h2dsettings.at(30008));

	hismanager->RegisterPlot<TH2F>("VETO_4000","Max Rear Veto PSD; Energy (arb.); PSD (arb.);",this->h2dsettings.at(4000));

	this->console->info("Finished Declaring Plots");
}

void VetoProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
	this->OutputTree = new TTree("Veto","Veto Processor output");
	this->OutputTree->Branch("fit",&fit_root);
	this->OutputTree->Branch("rit",&rit_root);
	outputtrees[this->ProcessorName] = this->OutputTree;
}

void VetoProcessor::CleanupTree(){
	this->rit_root = ProcessorStruct::DEFAULT_VETO_STRUCT;
	this->fit_root = ProcessorStruct::DEFAULT_VETO_STRUCT;
}

void VetoProcessor::Reset(){
	this->rit = 0.0;
	this->fit = 0.0;
	this->rit_psd = -999.0;
	this->fit_psd = -999.0;
}

const double& VetoProcessor::GetRIT() const{
	return this->rit;
}

const double& VetoProcessor::GetFIT() const{
	return this->fit;
}
