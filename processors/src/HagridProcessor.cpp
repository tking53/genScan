#include "HagridProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

HagridProcessor::HagridProcessor(const std::string& log) : Processor(log,"HagridProcessor",{"hagrid"}){
	this->upstreamtag = "upstream";
	this->downstreamtag = "downstream";
	this->FoundFirstEvt = false;
	this->FirstEvtTime = 0.0;
}

[[maybe_unused]] bool HagridProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	summary.GetDetectorSummary(this->AllDefaultRegex["hagrid"],this->SummaryData);
	for( const auto& evt : this->SummaryData ){
		auto currgroup = evt->GetGroup();
		int groupnum = std::stoi(currgroup);
		bool isupstream = evt->HasTag(upstreamtag);
		bool isdownstream = evt->HasTag(downstreamtag);
		this->CurrEvt.RealEvent = true;

		if( (isupstream and isdownstream) or (not isupstream and not isdownstream) ){
			throw std::runtime_error("malformed xml for hagrid, has both upstream and downstream tag or neither");
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

		int detloc = 2*groupnum + isdownstream;

		if( detloc >= this->NumHagrid ){
			throw std::runtime_error("found hagrid detector location that is higher than the known number of hagrid");
		}
		if( not this->FoundFirstEvt ){
			this->FoundFirstEvt = true;
			this->FirstEvtTime = this->CurrEvt.FirstTimeStamp;
		}

		if( not this->Hits[detloc] ){
			++this->Hits[detloc];
			this->CurrEvt.Energy[detloc] += evt->GetEnergy();
			this->CurrEvt.TimeStamp[detloc] = evt->GetTimeStamp();
			this->TimeStamps.push_back(evt->GetTimeStamp());
		}else{
			++this->Hits[detloc];
		}
	}

	if( (not this->CurrEvt.Saturate) and (not this->CurrEvt.Pileup) ){
		int idx = 0;
		this->CurrEvt.FirstTimeStamp = *(std::min_element(this->TimeStamps.begin(),this->TimeStamps.end()));
		this->CurrEvt.FinalTimeStamp = *(std::max_element(this->TimeStamps.begin(),this->TimeStamps.end()));
		double upstreamsum = 0.0;
		double downstreamsum = 0.0;
		for( const auto& erg : this->CurrEvt.Energy ){
			this->CurrEvt.TotalEnergy += erg;
			hismanager->Fill("HAGRID_4000",erg,idx);
			if( idx%2 ){
				downstreamsum += erg;
				hismanager->Fill("HAGRID_5025",erg);
				hismanager->Fill("HAGRID_5025_TIME",erg,(this->CurrEvt.FirstTimeStamp-this->FirstEvtTime)*1.0e-9);
			}else{
				upstreamsum += erg;
				hismanager->Fill("HAGRID_5015",erg);
				hismanager->Fill("HAGRID_5015_TIME",erg,(this->CurrEvt.FirstTimeStamp-this->FirstEvtTime)*1.0e-9);
			}
			hismanager->Fill("HAGRID_5005",erg);
			hismanager->Fill("HAGRID_5005_TIME",erg,(this->CurrEvt.FirstTimeStamp-this->FirstEvtTime)*1.0e-9);

			++idx;
		}

		hismanager->Fill("HAGRID_6000",(this->CurrEvt.FirstTimeStamp-this->FirstEvtTime)*1.0e-9);

		hismanager->Fill("HAGRID_5010",upstreamsum);
		hismanager->Fill("HAGRID_5010_TIME",upstreamsum,(this->CurrEvt.FirstTimeStamp-this->FirstEvtTime)*1.0e-9);
		hismanager->Fill("HAGRID_5020",downstreamsum);
		hismanager->Fill("HAGRID_5020_TIME",downstreamsum,(this->CurrEvt.FirstTimeStamp-this->FirstEvtTime)*1.0e-9);
		hismanager->Fill("HAGRID_5000",this->CurrEvt.TotalEnergy);
		hismanager->Fill("HAGRID_5000_TIME",this->CurrEvt.TotalEnergy,(this->CurrEvt.FirstTimeStamp-this->FirstEvtTime)*1.0e-9);
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool HagridProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool HagridProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->Reset();

	return true;
}

void HagridProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
	this->NumHagrid = 16;
}

void HagridProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
	this->NumHagrid = 16;
}

void HagridProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
	this->NumHagrid = 16;
}
		
void HagridProcessor::Finalize(){
	this->InitHelpers();
	this->console->info("{} has been finalized, {} Hagrids exist",this->ProcessorName,this->NumHagrid);
}

void HagridProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	hismanager->RegisterPlot<TH2F>("HAGRID_4000","Hagrid Energy; Energy (keV); Hagrid Number (arb.)",16384,0,16384,this->NumHagrid,0,this->NumHagrid);
	hismanager->RegisterPlot<TH1F>("HAGRID_5000","Hagrid sum; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH1F>("HAGRID_5005","Hagrid stack; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH1F>("HAGRID_5010","Upstream Hagrid sum; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH1F>("HAGRID_5015","Upstream Hagrid stack; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH1F>("HAGRID_5020","Downstream Hagrid sum; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH1F>("HAGRID_5025","Downstream Hagrid stack; Energy (keV)",16384,0,16384);
	
	hismanager->RegisterPlot<TH2F>("HAGRID_5000_TIME","Hagrid sum decay; Energy (keV)",16384,0,16384,1024,0,1024);
	hismanager->RegisterPlot<TH2F>("HAGRID_5005_TIME","Hagrid stack decay; Energy (keV)",16384,0,16384,1024,0,1024);
	hismanager->RegisterPlot<TH2F>("HAGRID_5010_TIME","Upstream Hagrid sum decay; Energy (keV)",16384,0,16384,1024,0,1024);
	hismanager->RegisterPlot<TH2F>("HAGRID_5015_TIME","Upstream Hagrid stack decay; Energy (keV)",16384,0,16384,1024,0,1024);
	hismanager->RegisterPlot<TH2F>("HAGRID_5020_TIME","Downstream Hagrid sum decay; Energy (keV)",16384,0,16384,1024,0,1024);
	hismanager->RegisterPlot<TH2F>("HAGRID_5025_TIME","Downstream Hagrid stack decay; Energy (keV)",16384,0,16384,1024,0,1024);
	
	hismanager->RegisterPlot<TH1F>("HAGRID_6000","Hagrid decay; Energy (keV)",65536,0,65536);

	this->console->info("Finished Declaring Plots");
}

void HagridProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
}

void HagridProcessor::CleanupTree(){
}

void HagridProcessor::Reset(){
	this->PrevEvt = this->CurrEvt;
	this->CurrEvt = this->NewEvt;
	this->Hits = std::vector<int>(this->NumHagrid,0);
	this->TimeStamps = std::vector<double>();
}

void HagridProcessor::InitHelpers(){
	this->NewEvt = {
		.Energy = std::vector<double>(this->NumHagrid,0.0),
		.TimeStamp = std::vector<double>(this->NumHagrid,0.0),
		.TotalEnergy = 0.0,
		.FirstTimeStamp = 0.0,
		.FinalTimeStamp = 0.0,
		.BetaTriggered = false,
		.IonTriggered = false,
		.Saturate = false,
		.Pileup = false,
		.RealEvent = false
	};
	this->Reset();
}

void HagridProcessor::SetIsBeta(){
	this->CurrEvt.BetaTriggered = true;
}

void HagridProcessor::SetIsIon(){
	this->CurrEvt.IonTriggered = true;
}

HagridProcessor::EventInfo& HagridProcessor::GetCurrEvt(){
	return this->CurrEvt;
}

HagridProcessor::EventInfo& HagridProcessor::GetPrevEvt(){
	return this->PrevEvt;
}
