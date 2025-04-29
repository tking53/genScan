#include "MtasTapeProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>
#include <string>

MtasTapeProcessor::MtasTapeProcessor(const std::string& log) : Processor(log,"MtasTapeProcessor",{"tape"}){

	this->h2dsettings = {
		{1000,{1024,0,1024,16,0,16}}
	};

	this->CycleStartTime = 0.0;

	this->CycleCount = 0;
	this->CurrState = TAPE::CycleState::UNKNOWN;
	this->PrevState = TAPE::CycleState::UNKNOWN;

	this->isTriggerOn = false;
	this->isIrradOn = false;
	this->isIrradOff = false;
	this->isLightPulseOn = false;
	this->isLightPulseOff = false;
	this->isTapeMoveOn = false;
	this->isTapeMoveOff = false;
	this->isBkgOn = false;
	this->isBkgOff = false;
	this->isMeasureOn = false;
	this->isMeasureOff = false;

	this->logicSignalValue = 0;

	std::string tapemove = "tapemove";
	std::string measure = "measure";
	std::string background = "background";
	std::string irradiation = "irradiation";
	std::string lightpulser = "lightpulser";
	std::string unknown = "unknown";
}

[[maybe_unused]] bool MtasTapeProcessor::PreProcess(EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	auto summary = eventhistory->GetCurrentEventSummary();
	summary->GetDetectorSummary(this->AllDefaultRegex["tape"],this->SummaryData);
	for( const auto& evt : this->SummaryData ){
		auto subtype = evt->GetSubType();
		auto isUp = (evt->GetGroup().compare("up") == 0);
		auto isDown = (evt->GetGroup().compare("down") == 0);
		if( isUp == isDown ){
			this->console->error("evt: {} has both up and down group",*evt);
			throw std::runtime_error("invalid xml config");
		}
		if( subtype.compare("trigger") == 0 ){
			this->CycleStartTime = evt->GetTimeStamp()*1.0e9;
			this->isTriggerOn = true;
			++(this->CycleCount);
			this->logicSignalValue += 1;
		}else if( subtype.compare("irradiation") == 0 ){
			if( isUp ){
				this->isIrradOn = true;
				this->logicSignalValue +=2;
			}else{
				this->isIrradOff = true;
				this->logicSignalValue +=4;
			}
		}else if( subtype.compare("light") == 0 ){
			if( isUp ){
				this->isLightPulseOn = true;
				this->logicSignalValue += 8;
			}else{
				this->isLightPulseOff = true;
				this->logicSignalValue += 16;
			}
		}else if( subtype.compare("move") == 0 ){
			if( isUp ){
				this->isTapeMoveOn = true;
				this->logicSignalValue += 32;
			}else{
				this->isTapeMoveOff = true;
				this->logicSignalValue += 64;
			}
		}else if( subtype.compare("background") == 0 ){
			if( isUp ){
				this->isBkgOn = true;
				this->logicSignalValue += 128;
			}else{
				this->isBkgOff = true;
				this->logicSignalValue += 256;
			}
		}else if( subtype.compare("measure") == 0 ){
			if( isUp ){
				this->isMeasureOn = true;
				this->logicSignalValue += 512;
			}else{
				this->isMeasureOff = true;
				this->logicSignalValue += 1024;
			}
		}else if( subtype.compare("MTC") == 0 ){
			//no-op
			continue;
		}else{
			//have MTC, stop and LPT
			//from old map file
			//don't know what the hell they were though
			this->console->error("evt: {} does not have trigger/irradiation/light/move/background/measure as type",*evt);
			throw std::runtime_error("invalid xml config");
		}
	}

	if( (isMeasureOn and isMeasureOff) or (isBkgOff and isBkgOn) or (isLightPulseOff and isLightPulseOn) or (isTapeMoveOff and isTapeMoveOff) or (isIrradOff and isIrradOn) ){
		//this->PrevState = this->CurrState;
		//this->CurrState = TAPE::CycleState::UNKNOWN;
		//summary->AddEventTag(this->unknown);
		//this->console->info("confused signaling");
	}else{
		if( isTapeMoveOn ){
			this->PrevState = this->CurrState;
			this->CurrState = TAPE::CycleState::TAPEMOVE;
			summary->AddEventTag(this->tapemove);
			//this->console->info("TapeMove");
		}else{
			if( isMeasureOn ){
				this->PrevState = this->CurrState;
				this->CurrState = TAPE::CycleState::MEASURE;
				summary->AddEventTag(this->measure);
				//this->console->info("Measure");
			}else if( isBkgOn ){
				this->PrevState = this->CurrState;
				this->CurrState = TAPE::CycleState::BACKGROUND;
				summary->AddEventTag(this->background);
				//this->console->info("bkg");
			}else if( isIrradOn ){
				this->PrevState = this->CurrState;
				this->CurrState = TAPE::CycleState::IRRADIATION;
				summary->AddEventTag(this->irradiation);
				//this->console->info("irrad");
			}else if( isLightPulseOn ){
				this->PrevState = this->CurrState;
				this->CurrState = TAPE::CycleState::LIGHTPULSER;
				summary->AddEventTag(this->lightpulser);
				//this->console->info("light");
			}else{
				//possibly is MTC signaling
				//this->PrevState = this->CurrState;
				//this->CurrState = TAPE::CycleState::UNKNOWN;
				//summary->AddEventTag(this->unknown);
				//this->console->info("unhandled case");
			}
		}
	}

	auto logictime = this->SummaryData.front()->GetTimeStamp()*1.0e9;
	logictime -= this->CycleStartTime;
	hismanager->WeightedFill("TAPE_1000",logictime,this->CycleCount,this->logicSignalValue);
	
	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool MtasTapeProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool MtasTapeProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->Reset();

	return true;
}

void MtasTapeProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}
		
void MtasTapeProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void MtasTapeProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager){
	//MtasTape diagnostic plots, always want these no matter what
	hismanager->RegisterPlot<TH2F>("TAPE_1000","Cycle Number vs Logic Signals; Logic Value (arb.); Cycle Number (arb.)",this->h2dsettings.at(1000));
	this->console->info("Finished Declaring Plots");
}

void MtasTapeProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
}

void MtasTapeProcessor::CleanupTree(){
}

void MtasTapeProcessor::Reset(){
	this->isTriggerOn = false;
	this->isIrradOn = false;
	this->isIrradOff = false;
	this->isLightPulseOn = false;
	this->isLightPulseOff = false;
	this->isTapeMoveOn = false;
	this->isTapeMoveOff = false;
	this->isBkgOn = false;
	this->isBkgOff = false;
	this->isMeasureOn = false;
	this->isMeasureOff = false;

	this->logicSignalValue = 0;
}

unsigned int MtasTapeProcessor::GetCurrentCycleNumber() const{
	return this->CycleCount;
}

TAPE::CycleState MtasTapeProcessor::GetCurrentCycleState() const{
	return this->CurrState;
}

void MtasTapeProcessor::IncrementCycleNumber(){
	++(this->CycleCount);
}

double MtasTapeProcessor::GetCycleTimeInSeconds() const{
	return this->CycleStartTime;
}
