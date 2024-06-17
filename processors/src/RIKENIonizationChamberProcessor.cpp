#include "RIKENIonizationChamberProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>
#include <string>
#include <vector>

RIKENIonizationChamberProcessor::RIKENIonizationChamberProcessor(const std::string& log) : Processor(log,"RIKENIonizationChamberProcessor",{"ionchamber"}){
}

[[maybe_unused]] bool RIKENIonizationChamberProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	summary.GetDetectorSummary(this->AllDefaultRegex["ionchamber"],this->SummaryData);
	for( const auto& evt : this->SummaryData ){
		auto subtype = evt->GetSubType();
		if( subtype.compare("anode") != 0 ){
			throw std::runtime_error("invalid subtype for ionchamber object, valid is anode");
		}
		auto currgroup = evt->GetGroup();
		int detloc = std::stoi(currgroup);
		this->CurrEvt.RealEvent = true;

		if( evt->GetPileup() or evt->GetSaturation() ){
			//ignore the saturated channel, but keep everything else in this current event
			if( evt->GetPileup() ){
				this->CurrEvt.Pileup = true;
			}
			if( evt->GetSaturation() ){
				this->CurrEvt.Saturate = true;
			}
			continue;
		}
		this->CurrEvt.RealEvent = true;

		if( not this->firstevt ){
			this->firstevt = true;
			this->CurrEvt.FirstTimeStamp = evt->GetTimeStamp();
		}else{
			this->CurrEvt.FinalTimeStamp = evt->GetTimeStamp();
		}

		if( detloc > this->NumAnode ){
			throw std::runtime_error("found anode group larger than known number of anodes");
		}else{
			if( not this->AnodeHits[detloc] ){
				++this->AnodeHits[detloc];
				this->CurrEvt.AnodeEnergy[detloc] += evt->GetEnergy();
				this->CurrEvt.AnodeTimeStamp[detloc] = evt->GetTimeStamp();
			}else{
				++this->AnodeHits[detloc];
			}
		}

	}

	if( (not this->CurrEvt.Saturate) and (not this->CurrEvt.Pileup) ){
		double sum = 0.0;
		for( const auto& e : this->CurrEvt.AnodeEnergy ){
			sum += e;
			if( e > this->CurrEvt.MaxAnodeEnergy ){
				this->CurrEvt.MaxAnodeEnergy = e;
			}
		}
		this->CurrEvt.TotalAnodeEnergy = sum;

		this->CurrEvt.FirstPSD = this->CurrEvt.AnodeEnergy[0]/this->CurrEvt.TotalAnodeEnergy;
		this->CurrEvt.MaxPSD = this->CurrEvt.MaxAnodeEnergy/this->CurrEvt.TotalAnodeEnergy;

		for( int ii = 0; ii < this->NumAnode; ++ii ){
			hismanager->Fill("IONCHAMBER_7000",this->CurrEvt.AnodeEnergy[ii],ii);
			for( int jj = 0; jj < this->NumAnode; ++jj ){
				if( ii != jj ){
					hismanager->Fill("IONCHAMBER_7050",this->CurrEvt.AnodeEnergy[ii],this->CurrEvt.AnodeEnergy[jj]);
				}
			}
		}

		hismanager->Fill("IONCHAMBER_7010",this->CurrEvt.TotalAnodeEnergy,this->CurrEvt.FirstPSD);
		hismanager->Fill("IONCHAMBER_7020",this->CurrEvt.TotalAnodeEnergy,this->CurrEvt.MaxPSD);
		hismanager->Fill("IONCHAMBER_7030",this->CurrEvt.MaxAnodeEnergy,this->CurrEvt.FirstPSD);
		hismanager->Fill("IONCHAMBER_7040",this->CurrEvt.MaxAnodeEnergy,this->CurrEvt.MaxPSD);
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool RIKENIonizationChamberProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool RIKENIonizationChamberProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->Reset();

	return true;
}

void RIKENIonizationChamberProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
	this->NumAnode = 6;
}

void RIKENIonizationChamberProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
	this->NumAnode = 6;
}

void RIKENIonizationChamberProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
	this->NumAnode = 6;
}
		
void RIKENIonizationChamberProcessor::Finalize(){
	this->InitHelpers();
	this->console->info("{} has been finalized, {} Anodes exist",this->ProcessorName,this->NumAnode);
}

void RIKENIonizationChamberProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	//First Cathode
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_7000","Anode Position vs Anode Energy; Energy (arb.); Position (arb.)",65536,0,65536,this->NumAnode,0,this->NumAnode);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_7010","First PSD (A/C) vs Total Anode",8192,0,8192,1024,0,1.0);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_7020","Max PSD (A/C) vs Total Anode",8192,0,8192,1024,0,1.0);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_7030","First PSD (A/C) vs Max Anode",8192,0,8192,1024,0,1.0);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_7040","Max PSD (A/C) vs Max Anode",8192,0,8192,1024,0,1.0);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_7050","Anode vs Anode; Energy (arb.); Energy (arb.)",8192,0,8192,8192,0,8192);

	this->console->info("Finished Declaring Plots");
}

void RIKENIonizationChamberProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
}

void RIKENIonizationChamberProcessor::CleanupTree(){
}

void RIKENIonizationChamberProcessor::Reset(){
	this->PrevEvt = this->CurrEvt;
	this->CurrEvt = this->NewEvt;
	this->AnodeHits = std::vector<int>(this->NumAnode,0);
	this->firstevt = false;
}

void RIKENIonizationChamberProcessor::InitHelpers(){
	this->NewEvt = {
		.AnodeEnergy = std::vector<double>(this->NumAnode,0.0),
		.AnodeTimeStamp = std::vector<double>(this->NumAnode,0.0),
		.TotalAnodeEnergy = 0.0,
		.MaxAnodeEnergy = 0.0,
		.FirstPSD = 0.0,
		.MaxPSD = 0.0,
		.FirstTimeStamp = 0.0,
		.FinalTimeStamp = 0.0,
		.Saturate = false,
		.Pileup = false,
		.RealEvent = false
	};
	this->Reset();
}

RIKENIonizationChamberProcessor::EventInfo& RIKENIonizationChamberProcessor::GetCurrEvt(){
	return this->CurrEvt;
}

RIKENIonizationChamberProcessor::EventInfo& RIKENIonizationChamberProcessor::GetPrevEvt(){
	return this->PrevEvt;
}
