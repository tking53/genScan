#include "RIKENPidProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>
#include <vector>

RIKENPidProcessor::RIKENPidProcessor(const std::string& log) : Processor(log,"RIKENPidProcessor",{"pid"}){
	this->NewEvt = {
		.F7Analog = 0.0,
		.F7AnalogTimeStamp = 0.0,
		.F7Logic = 0.0,
		.F7LogicTimeStamp = 0.0,
		.F11LeftRight = std::vector<double>(2,0.0),
		.F11LeftRightTimeStamp = std::vector<double>(2,0.0),
		.tdiff = std::vector<double>(4,0.0),
		.Pileup = false,
		.Saturate = false,
		.RealEvent = false
	};
	this->Reset();

	this->currsubtype = SUBTYPE::UNKNOWN;
	this->analogtag = "analog";
	this->logictag = "logic";
	this->lefttag = "left";
	this->righttag = "right";
}

[[maybe_unused]] bool RIKENPidProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();
	summary.GetDetectorSummary(this->AllDefaultRegex["pid"],this->SummaryData);

	for( const auto& evt : this->SummaryData ){
		auto subtype = evt->GetSubType();
		if( subtype.compare("f7") == 0 ){
			this->currsubtype = SUBTYPE::F7;
		}else if( subtype.compare("f11") == 0 ){
			this->currsubtype = SUBTYPE::F11;
		}else{
			throw std::runtime_error("malformed xml subtype for pid, needs either f7 or f11");
		}

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

		if( this->currsubtype == SUBTYPE::F7 ){
			if( evt->HasTag(this->analogtag) ){
				if( not this->F7AnalogHits ){
					++this->F7AnalogHits;
					this->CurrEvt.F7Analog += evt->GetEnergy();
					this->CurrEvt.F7AnalogTimeStamp = evt->GetTimeStamp();
				}else{
					++this->F7AnalogHits;
				}
			}else if( evt->HasTag(this->logictag) ){
				if( not this->F7LogicHits ){
					++this->F7LogicHits;
					this->CurrEvt.F7Logic += evt->GetEnergy();
					this->CurrEvt.F7LogicTimeStamp = evt->GetTimeStamp();
				}else{
					++this->F7LogicHits;
				}
			}else{
				throw std::runtime_error("malformed xml tag for pid:f7 requires either analog or logic tag");
			}
		}else if( this->currsubtype == SUBTYPE::F11 ){
			if( evt->HasTag(lefttag) ){
				if( not this->F11Hits[0] ){
					++this->F11Hits[0];
					this->CurrEvt.F11LeftRight[0] += evt->GetEnergy();
					this->CurrEvt.F11LeftRightTimeStamp[0] = evt->GetTimeStamp();
				}else{
					++this->F11Hits[0];
				}
			}else if( evt->HasTag(righttag) ){
				if( not this->F11Hits[1] ){
					++this->F11Hits[1];
					this->CurrEvt.F11LeftRight[1] += evt->GetEnergy();
					this->CurrEvt.F11LeftRightTimeStamp[1] = evt->GetTimeStamp();
				}else{
					++this->F11Hits[1];
				}
			}else{
				throw std::runtime_error("malformed xml tag for pid:f11 requires either left or right tag");
			}
		}else{
			//no-op 
		}

	}
	
	if( (not this->CurrEvt.Saturate) and (not this->CurrEvt.Pileup) ){
		this->CurrEvt.tdiff[0] = this->CurrEvt.F11LeftRightTimeStamp[0] - this->CurrEvt.F7AnalogTimeStamp;
		this->CurrEvt.tdiff[1] = this->CurrEvt.F11LeftRightTimeStamp[1] - this->CurrEvt.F7AnalogTimeStamp;
		this->CurrEvt.tdiff[2] = this->CurrEvt.F11LeftRightTimeStamp[0] - this->CurrEvt.F7LogicTimeStamp;
		this->CurrEvt.tdiff[3] = this->CurrEvt.F11LeftRightTimeStamp[1] - this->CurrEvt.F7LogicTimeStamp;

		hismanager->Fill("PID_4000",this->CurrEvt.tdiff[0]);
		hismanager->Fill("PID_4001",this->CurrEvt.tdiff[1]);
		hismanager->Fill("PID_4010",this->CurrEvt.tdiff[2]);
		hismanager->Fill("PID_4011",this->CurrEvt.tdiff[3]);
	}
	
	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool RIKENPidProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool RIKENPidProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->Reset();
	return true;
}

void RIKENPidProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
}

void RIKENPidProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
}

void RIKENPidProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
}
		
void RIKENPidProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void RIKENPidProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	hismanager->RegisterPlot<TH1F>("PID_4000","TDiff F11_Left - F7_Analog; tdiff (ns)",1000,-20,20);
	hismanager->RegisterPlot<TH1F>("PID_4001","TDiff F11_Right - F7_Analog; tdiff (ns)",1000,-20,20);
	hismanager->RegisterPlot<TH1F>("PID_4010","TDiff F11_Left - F7_Logic; tdiff (ns)",1000,-20,20);
	hismanager->RegisterPlot<TH1F>("PID_4011","TDiff F11_Right - F7_Logic; tdiff (ns)",1000,-20,20);
	this->console->info("Finished Declaring Plots");
}

void RIKENPidProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
}

void RIKENPidProcessor::CleanupTree(){
}

RIKENPidProcessor::EventInfo& RIKENPidProcessor::GetCurrEvt(){
	return this->CurrEvt;
}

RIKENPidProcessor::EventInfo& RIKENPidProcessor::GetPrevEvt(){
	return this->PrevEvt;
}

void RIKENPidProcessor::Reset(){
	this->PrevEvt = this->CurrEvt;
	this->CurrEvt = this->NewEvt;
	this->F7AnalogHits = 0;
	this->F7LogicHits = 0;
	this->F11Hits = std::vector<int>(2,0);
}
