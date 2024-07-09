#include "PuckProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <numeric>
#include <string>

PuckProcessor::PuckProcessor(const std::string& log) : Processor(log,"PuckProcessor",{"puck"}){
	this->NewEvt = EventInfo();
	this->PrevEvt = this->NewEvt;
	this->CurrEvt = this->NewEvt;

	this->fronttag = "front";
	this->backtag = "back";
	this->foundfirstevt = false;
	this->globalfirsttime = 0.0;

	this->h1dsettings = { 
				{3600 , {16384,0.0,16384}},
				{3601 , {16384,0.0,16384}},
				{3602 , {16384,0.0,16384}},
				{3610 , {16384,0.0,16384}},
				{3611 , {16384,0.0,16384}},
				{3612 , {16384,0.0,16384}}
			    };

	this->h2dsettings = {
				{3650 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36508 , {2048,0.0,16384.0,2048,0.0,16384.0}},
				{3651 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36518 , {2048,0.0,16384.0,2048,0.0,16384.0}},
				{3652 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36528 , {2048,0.0,16384.0,2048,0.0,16384.0}}
			    };
	
	this->PuckHits = std::vector<int>(2,0);
}

[[maybe_unused]] bool PuckProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	summary.GetDetectorSummary(this->AllDefaultRegex["puck"],this->SummaryData);
	for( const auto& evt : this->SummaryData ){
		auto subtype = evt->GetSubType();
		auto group = evt->GetGroup();

		auto isfront = evt->HasTag(fronttag);
		auto isback = evt->HasTag(backtag);

		int detectorposition = isback;

		if( (not isfront and not isback) or (isfront and isback) ){
			throw std::runtime_error("evt in MtasProcessor is malformed in xml, and has either both front and back tag or neither");
		}

		if( not foundfirstevt ){
			foundfirstevt = true;
			globalfirsttime = evt->GetTimeStamp();
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

		if( !this->PuckHits[detectorposition] ){
			this->CurrEvt.Individual[detectorposition] = evt->GetEnergy();
			this->TimeStamps.push_back(evt->GetTimeStamp());
			++this->PuckHits[detectorposition];
		}else{
			++this->PuckHits[detectorposition];
		}
	}
	
	this->CurrEvt.FirstTime = *(std::min_element(this->TimeStamps.begin(),this->TimeStamps.end()));
	this->CurrEvt.LastTime = *(std::max_element(this->TimeStamps.begin(),this->TimeStamps.end()));

	this->CurrEvt.TotalEnergy = std::accumulate(this->CurrEvt.Individual.begin(),this->CurrEvt.Individual.end(),0.0);

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool PuckProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool PuckProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->Reset();

	return true;
}

void PuckProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void PuckProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void PuckProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}
		
void PuckProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void PuckProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	//Puck diagnostic plots, always want these no matter what
	hismanager->RegisterPlot<TH1F>("Puck_3600","Puck Total; Energy (keV)",this->h1dsettings.at(3600));
	hismanager->RegisterPlot<TH1F>("Puck_3601","Puck Total No MTAS; Energy (keV)",this->h1dsettings.at(3601));
	hismanager->RegisterPlot<TH1F>("Puck_3602","Puck Total + MTAS Total; Energy (keV)",this->h1dsettings.at(3602));
	hismanager->RegisterPlot<TH1F>("Puck_3610","Puck Total; Energy (keV)",this->h1dsettings.at(3610));
	hismanager->RegisterPlot<TH1F>("Puck_3611","Puck Total [MTAS Pileup]; Energy (keV)",this->h1dsettings.at(3611));
	hismanager->RegisterPlot<TH1F>("Puck_3612","Puck Total [MTAS Saturate]; Energy (keV)",this->h1dsettings.at(3612));

	hismanager->RegisterPlot<TH2F>("Puck_3650","Puck Total vs MTAS Total; MTAS Total Energy (keV); Puck Energy (keV)",this->h2dsettings.at(3650));
	hismanager->RegisterPlot<TH2F>("Puck_36508","Puck Total vs MTAS Total; MTAS Total Energy (8 keV/bin); Puck Energy (8 keV/bin)",this->h2dsettings.at(36508));
	hismanager->RegisterPlot<TH2F>("Puck_3651","Puck Front vs MTAS Total; MTAS Total Energy (keV); Puck Energy (keV)",this->h2dsettings.at(3651));
	hismanager->RegisterPlot<TH2F>("Puck_36518","Puck Front vs MTAS Total; MTAS Total Energy (8 keV/bin); Puck Energy (8 keV/bin)",this->h2dsettings.at(36518));
	hismanager->RegisterPlot<TH2F>("Puck_3652","Puck Back vs MTAS Total; MTAS Total Energy (keV); Puck Energy (keV)",this->h2dsettings.at(3652));
	hismanager->RegisterPlot<TH2F>("Puck_36528","Puck Back vs MTAS Total; MTAS Total Energy (8 keV/bin); Puck Energy (8 keV/bin)",this->h2dsettings.at(36528));

	this->console->info("Finished Declaring Plots");
}

void PuckProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
}

void PuckProcessor::CleanupTree(){
}

void PuckProcessor::Reset(){
	this->PrevEvt = this->CurrEvt;
	this->CurrEvt = this->NewEvt;
	this->TimeStamps.clear();
	this->PuckHits = std::vector<int>(12,0);
}

PuckProcessor::EventInfo& PuckProcessor::GetCurrEvt(){
	return this->CurrEvt;
}

PuckProcessor::EventInfo& PuckProcessor::GetPrevEvt(){
	return this->PrevEvt;
}
