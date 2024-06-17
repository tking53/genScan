#include "VetoProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>

VetoProcessor::VetoProcessor(const std::string& log) : Processor(log,"VetoProcessor",{"veto"}){
	this->NewEvt = {
		.FrontErg = std::vector<double>(2,0.0),
		.FrontTimeStamp = std::vector<double>(2,0.0),
		.RearErg = std::vector<double>(2,0.0),
		.RearTimeStamp = std::vector<double>(2,0.0),
		.MaxFrontErg = 0.0,
		.MaxFrontTimeStamp = 0.0,
		.MaxRearErg = 0.0,
		.MaxRearTimeStamp = 0.0,
		.Pileup = false,
		.Saturate = false,
		.RealEvent = false
	};
	this->Reset();
	this->currsubtype = SUBTYPE::UNKNOWN;
}

[[maybe_unused]] bool VetoProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();
	summary.GetDetectorSummary(this->AllDefaultRegex["veto"],this->SummaryData);

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

		if( this->currsubtype == SUBTYPE::FIT ){
			if( not this->FrontHits[detloc] ){
				++this->FrontHits[detloc];
				this->CurrEvt.FrontErg[detloc] += evt->GetEnergy();
				this->CurrEvt.FrontTimeStamp[detloc] = evt->GetTimeStamp();
			}else{
				++this->FrontHits[detloc];
			}
		}else if( this->currsubtype == SUBTYPE::RIT ){
			if( not this->RearHits[detloc] ){
				++this->RearHits[detloc];
				this->CurrEvt.RearErg[detloc] += evt->GetEnergy();
				this->CurrEvt.RearTimeStamp[detloc] = evt->GetTimeStamp();
			}else{
				++this->RearHits[detloc];
			}
		}else{
			//no-op
		}
	}
	
	if( (not this->CurrEvt.Saturate) and (not this->CurrEvt.Pileup) ){
		for( size_t ii = 0; ii < 2; ++ii ){
			hismanager->Fill("VETO_1000",this->CurrEvt.FrontErg[ii],ii);
			hismanager->Fill("VETO_2000",this->CurrEvt.RearErg[ii],ii);

			if( this->CurrEvt.FrontErg[ii] > this->CurrEvt.MaxFrontErg ){
				this->CurrEvt.MaxFrontErg = this->CurrEvt.FrontErg[ii];
				this->CurrEvt.MaxFrontTimeStamp = this->CurrEvt.FrontTimeStamp[ii];
			}

			if( this->CurrEvt.RearErg[ii] > this->CurrEvt.MaxRearErg ){
				this->CurrEvt.MaxRearErg = this->CurrEvt.RearErg[ii];
				this->CurrEvt.MaxRearTimeStamp = this->CurrEvt.RearTimeStamp[ii];
			}

			for( size_t jj = 0; jj < 2; ++jj ){
				hismanager->Fill("VETO_3000",this->CurrEvt.RearErg[jj],this->CurrEvt.FrontErg[ii]);
			}
		}
		hismanager->Fill("VETO_1010",this->CurrEvt.MaxFrontErg);
		hismanager->Fill("VETO_2010",this->CurrEvt.MaxRearErg);
		hismanager->Fill("VETO_3010",this->CurrEvt.MaxRearErg,this->CurrEvt.MaxFrontErg);
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool VetoProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool VetoProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->Reset();
	return true;
}

void VetoProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
}

void VetoProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
}

void VetoProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
}
		
void VetoProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void VetoProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	hismanager->RegisterPlot<TH2F>("VETO_1000","Front Veto Singles; Energy (arb.); Position (arb.)",65536,0,65536,2,0,2);
	hismanager->RegisterPlot<TH1F>("VETO_1010","Max Front Veto; Energy (arb.);",65536,0,65536);
	hismanager->RegisterPlot<TH2F>("VETO_2000","Rear Veto Singles; Energy (arb.); Position (arb.)",65536,0,65536,2,0,2);
	hismanager->RegisterPlot<TH1F>("VETO_2010","Max Rear Veto; Energy (arb.);",65536,0,65536);
	hismanager->RegisterPlot<TH2F>("VETO_3000","Front Veto vs Rear Veto; Energy (arb.); Energy (arb.)",8192,0,65536,8192,0,65536);
	hismanager->RegisterPlot<TH2F>("VETO_3010","Max Front Veto vs Max Rear Veto; Energy (arb.); Energy (arb.)",8192,0,65536,8192,0,65536);
	this->console->info("Finished Declaring Plots");
}

void VetoProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
}

void VetoProcessor::CleanupTree(){
}

void VetoProcessor::Reset(){
	this->FrontHits = std::vector<int>(2,0);
	this->RearHits = std::vector<int>(2,0);
	this->PrevEvt = this->CurrEvt;
	this->CurrEvt = this->NewEvt;
}

VetoProcessor::EventInfo& VetoProcessor::GetCurrEvt(){
	return this->CurrEvt;
}

VetoProcessor::EventInfo& VetoProcessor::GetPrevEvt(){
	return this->PrevEvt;
}
