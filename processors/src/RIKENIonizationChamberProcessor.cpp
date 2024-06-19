#include "RIKENIonizationChamberProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

RIKENIonizationChamberProcessor::RIKENIonizationChamberProcessor(const std::string& log) : Processor(log,"RIKENIonizationChamberProcessor",{"ionchamber"}){
	this->FoundFirstEvt = false;
	this->FirstEvtTime = 0.0;
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

		if( not this->FoundFirstEvt ){
			this->FoundFirstEvt = true;
			this->FirstEvtTime = evt->GetTimeStamp();
		}

		if( detloc > this->NumAnode ){
			throw std::runtime_error("found anode group larger than known number of anodes");
		}else{
			if( evt->GetEnergy() > this->IC[detloc].first ){
				this->IC[detloc].first = evt->GetEnergy();
				this->IC[detloc].second = evt->GetTimeStamp();
			}
		}

	}

	if( (not this->CurrEvt.Saturate) and (not this->CurrEvt.Pileup) ){
		int idx = 0;
		double sum = 0.0;
		double curranodenum = 0.0;
		for( const auto& ic : this->IC ){
			if( ic.first > 0.0 ){
				this->TimeStamps.push_back(ic.second);
				this->CurrEvt.AnodeEnergy[idx] = std::log10(ic.first);
				this->CurrEvt.AverageEnergy += this->CurrEvt.AnodeEnergy[idx];
				curranodenum += 1.0;
			}
			if( ic.first > this->CurrEvt.MaxAnodeEnergy ){
				this->CurrEvt.MaxAnodeEnergy = std::log10(ic.first);
			}
			hismanager->Fill("IONCHAMBER_7000",this->CurrEvt.AnodeEnergy[idx],idx);
			sum += this->CurrEvt.AnodeEnergy[idx];
			++idx;
		}
		this->CurrEvt.AverageEnergy/=curranodenum;
		this->CurrEvt.AverageEnergy = std::pow(10,this->CurrEvt.AverageEnergy);
		this->CurrEvt.TotalAnodeEnergy = std::pow(10,sum);

		this->CurrEvt.FirstTimeStamp = *(std::min_element(this->TimeStamps.begin(),this->TimeStamps.end()));
		this->CurrEvt.FinalTimeStamp = *(std::max_element(this->TimeStamps.begin(),this->TimeStamps.end()));

		this->CurrEvt.FirstPSD = this->CurrEvt.AnodeEnergy[0]/this->CurrEvt.TotalAnodeEnergy;
		this->CurrEvt.MaxPSD = this->CurrEvt.MaxAnodeEnergy/this->CurrEvt.TotalAnodeEnergy;

		for( int ii = 0; ii < this->NumAnode; ++ii ){
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

		hismanager->Fill("IONCHAMBER_8000",this->CurrEvt.MaxAnodeEnergy);
		hismanager->Fill("IONCHAMBER_8010",this->CurrEvt.TotalAnodeEnergy);
		hismanager->Fill("IONCHAMBER_8020",this->CurrEvt.AverageEnergy);

		double time_s = (this->CurrEvt.FirstTimeStamp - this->FirstEvtTime)*1.0e-9;
		double time_m = time_s/60.0;
		double time_h = time_m/60.0;
		
		hismanager->Fill("IONCHAMBER_8000_TIME_S",this->CurrEvt.MaxAnodeEnergy,time_s);
		hismanager->Fill("IONCHAMBER_8010_TIME_S",this->CurrEvt.TotalAnodeEnergy,time_s);
		hismanager->Fill("IONCHAMBER_8020_TIME_S",this->CurrEvt.AverageEnergy,time_s);
		
		hismanager->Fill("IONCHAMBER_8000_TIME_M",this->CurrEvt.MaxAnodeEnergy,time_m);
		hismanager->Fill("IONCHAMBER_8010_TIME_M",this->CurrEvt.TotalAnodeEnergy,time_m);
		hismanager->Fill("IONCHAMBER_8020_TIME_M",this->CurrEvt.AverageEnergy,time_m);

		hismanager->Fill("IONCHAMBER_8000_TIME_H",this->CurrEvt.MaxAnodeEnergy,time_h);
		hismanager->Fill("IONCHAMBER_8010_TIME_H",this->CurrEvt.TotalAnodeEnergy,time_h);
		hismanager->Fill("IONCHAMBER_8020_TIME_H",this->CurrEvt.AverageEnergy,time_h);
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
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_7000","Anode Position vs Anode Energy; Energy (arb.); Position (arb.)",8192,0,4,this->NumAnode,0,this->NumAnode);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_7010","First PSD (A/C) vs Total Anode",8192,0,32,1024,0,1.0);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_7020","Max PSD (A/C) vs Total Anode",8192,0,32,1024,0,1.0);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_7030","First PSD (A/C) vs Max Anode",8192,0,4,1024,0,1.0);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_7040","Max PSD (A/C) vs Max Anode",8192,0,4,1024,0,1.0);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_7050","Anode vs Anode; Energy (arb.); Energy (arb.)",8192,0,4,8192,0,4);

	hismanager->RegisterPlot<TH1F>("IONCHAMBER_8000","Max Anode Energy; Energy (arb.)",8192,0,4);
	hismanager->RegisterPlot<TH1F>("IONCHAMBER_8010","Total Anode Energy; Energy (arb.)",8192,0,32);
	hismanager->RegisterPlot<TH1F>("IONCHAMBER_8020","Average Anode Energy; Energy (arb.)",8192,0,4);

	hismanager->RegisterPlot<TH2F>("IONCHAMBER_8000_TIME_S","Max Anode Energy; Energy (arb.); Time since first evt (s)",8192,0,4,1024,0,1024);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_8010_TIME_S","Total Anode Energy; Energy (arb.); Time since first evt (s)",8192,0,32,1024,0,1024);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_8020_TIME_S","Average Anode Energy; Energy (arb.); Time since first evt (s)",8192,0,4,1024,0,1024);

	hismanager->RegisterPlot<TH2F>("IONCHAMBER_8000_TIME_M","Max Anode Energy; Energy (arb.); Time since first evt (min)",8192,0,4,1024,0,1024);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_8010_TIME_M","Total Anode Energy; Energy (arb.); Time since first evt (min)",8192,0,32,1024,0,1024);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_8020_TIME_M","Average Anode Energy; Energy (arb.); Time since first evt (min)",8192,0,4,1024,0,1024);

	hismanager->RegisterPlot<TH2F>("IONCHAMBER_8000_TIME_H","Max Anode Energy; Energy (arb.); Time since first evt (hr)",8192,0,4,1024,0,1024);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_8010_TIME_H","Total Anode Energy; Energy (arb.); Time since first evt (hr)",8192,0,32,1024,0,1024);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_8020_TIME_H","Average Anode Energy; Energy (arb.); Time since first evt (hr)",8192,0,4,1024,0,1024);

	this->console->info("Finished Declaring Plots");
}

void RIKENIonizationChamberProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
}

void RIKENIonizationChamberProcessor::CleanupTree(){
}

void RIKENIonizationChamberProcessor::Reset(){
	this->PrevEvt = this->CurrEvt;
	this->CurrEvt = this->NewEvt;
	this->IC = { {0.0,0.0}
		    ,{0.0,0.0}
		    ,{0.0,0.0}
		    ,{0.0,0.0}
		    ,{0.0,0.0}
		    ,{0.0,0.0}
	};
	this->TimeStamps = std::vector<double>();
}

void RIKENIonizationChamberProcessor::InitHelpers(){
	this->NewEvt = {
		.AnodeEnergy = std::vector<double>(this->NumAnode,0.0),
		.AnodeTimeStamp = std::vector<double>(this->NumAnode,0.0),
		.TotalAnodeEnergy = 0.0,
		.MaxAnodeEnergy = 0.0,
		.AverageEnergy = 0.0,
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
