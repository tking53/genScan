#include "IonizationChamberProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>
#include <string>
#include <vector>

IonizationChamberProcessor::IonizationChamberProcessor(const std::string& log) : Processor(log,"IonizationChamberProcessor",{"ionchamber"}){
	this->currsubtype = SUBTYPE::UNKNOWN;
}

[[maybe_unused]] bool IonizationChamberProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	this->Reset();

	summary.GetDetectorSummary(this->AllDefaultRegex["ionchamber"],this->SummaryData);
	for( const auto& evt : this->SummaryData ){
		auto subtype = evt->GetSubType();
		if( subtype.compare("anode") == 0 ){
			this->currsubtype = SUBTYPE::ANODE;
		}else if( subtype.compare("cathode") == 0 ){
			this->currsubtype = SUBTYPE::CATHODE;
		}else{
			throw std::runtime_error("invalid subtype for ionchamber object, valid is either anode or cathode");
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

		if( this->currsubtype == SUBTYPE::ANODE ){
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
		}else if( this->currsubtype == SUBTYPE::CATHODE ){
			if( detloc > this->NumCathode ){
				throw std::runtime_error("found anode group larger than known number of anodes");
			}else{
				if( not this->CathodeHits[detloc] ){
					++this->CathodeHits[detloc];
					this->CurrEvt.CathodeEnergy[detloc] += evt->GetEnergy();
					this->CurrEvt.CathodeTimeStamp[detloc] = evt->GetTimeStamp();
				}else{
					++this->CathodeHits[detloc];
				}
			}
		}else{
			//no-op
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

		sum = 0.0;
		for( const auto& e : this->CurrEvt.CathodeEnergy ){
			sum += e;
			if( e > this->CurrEvt.MaxCathodeEnergy ){
				this->CurrEvt.MaxCathodeEnergy = e;
			}
		}
		this->CurrEvt.TotalCathodeEnergy = sum;

		this->CurrEvt.FirstPSD = this->CurrEvt.AnodeEnergy[0]/this->CurrEvt.CathodeEnergy[0];
		this->CurrEvt.TotalPSD = this->CurrEvt.TotalAnodeEnergy/this->CurrEvt.TotalCathodeEnergy;
		this->CurrEvt.MaxPSD = this->CurrEvt.MaxAnodeEnergy/this->CurrEvt.MaxCathodeEnergy;

		hismanager->Fill("IONCHAMBER_7000",this->CurrEvt.CathodeEnergy[0],this->CurrEvt.AnodeEnergy[0]);
		hismanager->Fill("IONCHAMBER_7010",this->CurrEvt.CathodeEnergy[0],this->CurrEvt.FirstPSD);
		hismanager->Fill("IONCHAMBER_7020",this->CurrEvt.CathodeEnergy[0],this->CurrEvt.TotalPSD);
		hismanager->Fill("IONCHAMBER_7030",this->CurrEvt.CathodeEnergy[0],this->CurrEvt.MaxPSD);

		hismanager->Fill("IONCHAMBER_8000",this->CurrEvt.TotalCathodeEnergy,this->CurrEvt.TotalAnodeEnergy);
		hismanager->Fill("IONCHAMBER_8010",this->CurrEvt.TotalCathodeEnergy,this->CurrEvt.TotalPSD);
		hismanager->Fill("IONCHAMBER_8020",this->CurrEvt.TotalCathodeEnergy,this->CurrEvt.FirstPSD);
		hismanager->Fill("IONCHAMBER_8030",this->CurrEvt.TotalCathodeEnergy,this->CurrEvt.MaxPSD);

		hismanager->Fill("IONCHAMBER_9000",this->CurrEvt.MaxCathodeEnergy,this->CurrEvt.MaxAnodeEnergy);
		hismanager->Fill("IONCHAMBER_9010",this->CurrEvt.MaxCathodeEnergy,this->CurrEvt.TotalPSD);
		hismanager->Fill("IONCHAMBER_9020",this->CurrEvt.MaxCathodeEnergy,this->CurrEvt.FirstPSD);
		hismanager->Fill("IONCHAMBER_9030",this->CurrEvt.MaxCathodeEnergy,this->CurrEvt.MaxPSD);
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool IonizationChamberProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool IonizationChamberProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

void IonizationChamberProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
	this->NumAnode = 5;
	this->NumCathode = 1;
}

void IonizationChamberProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
	this->NumAnode = 5;
	this->NumCathode = 1;
}

void IonizationChamberProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
	this->NumAnode = 5;
	this->NumCathode = 1;
}
		
void IonizationChamberProcessor::Finalize(){
	this->InitHelpers();
	this->console->info("{} has been finalized, {} Anodes exist and {} Cathodes exist",this->ProcessorName,this->NumAnode,this->NumCathode);
}

void IonizationChamberProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	//First Cathode
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_7000","First Anode vs First Cathode",8192,0,8192,8192,0,8192);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_7010","First PSD (A/C) vs First Cathode",8192,0,8192,1024,0,1.0);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_7020","Total PSD (A/C) vs First Cathode",8192,0,8192,1024,0,1.0);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_7030","Max PSD (A/C) vs First Cathode",8192,0,8192,1024,0,1.0);

	//Total Cathode
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_8000","Anode Sum vs Cathode Sum",8192,0,8192,8192,0,8192);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_8010","Total PSD (A/C) vs Cathode Sum",8192,0,8192,1024,0,1.0);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_8020","First PSD (A/C) vs Cathode Sum",8192,0,8192,1024,0,1.0);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_8030","Max PSD (A/C) vs Cathode Sum",8192,0,8192,1024,0,1.0);

	//Max Cathode
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_9000","Max Anode vs Max Cathode",8192,0,8192,8192,0,8192);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_9010","Total PSD (A/C) vs Max Cathode",8192,0,8192,1024,0,1.0);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_9020","First PSD (A/C) vs Max Cathode",8192,0,8192,1024,0,1.0);
	hismanager->RegisterPlot<TH2F>("IONCHAMBER_9030","Max PSD (A/C) vs Max Cathode",8192,0,8192,1024,0,1.0);

	this->console->info("Finished Declaring Plots");
}

void IonizationChamberProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
}

void IonizationChamberProcessor::CleanupTree(){
}

void IonizationChamberProcessor::Reset(){
	this->PrevEvt = this->CurrEvt;
	this->CurrEvt = this->NewEvt;
	this->AnodeHits = std::vector<int>(this->NumAnode,0);
	this->CathodeHits = std::vector<int>(this->NumCathode,0);
	this->firstevt = false;
}

void IonizationChamberProcessor::InitHelpers(){
	this->NewEvt = {
		.AnodeEnergy = std::vector<double>(this->NumAnode,0.0),
		.AnodeTimeStamp = std::vector<double>(this->NumAnode,0.0),
		.TotalAnodeEnergy = 0.0,
		.MaxAnodeEnergy = 0.0,
		.CathodeEnergy = std::vector<double>(this->NumCathode,0.0),
		.CathodeTimeStamp = std::vector<double>(this->NumCathode,0.0),
		.TotalCathodeEnergy = 0.0,
		.MaxCathodeEnergy = 0.0,
		.FirstPSD = 0.0,
		.TotalPSD = 0.0,
		.MaxPSD = 0.0,
		.FirstTimeStamp = 0.0,
		.FinalTimeStamp = 0.0,
		.Saturate = false,
		.Pileup = false,
		.RealEvent = false
	};
	this->CurrEvt = this->NewEvt;
	this->PrevEvt = this->CurrEvt;

}

IonizationChamberProcessor::EventInfo& IonizationChamberProcessor::GetCurrEvt(){
	return this->CurrEvt;
}

IonizationChamberProcessor::EventInfo& IonizationChamberProcessor::GetPrevEvt(){
	return this->PrevEvt;
}
