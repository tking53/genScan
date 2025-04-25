#include "MtasSSDProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>
#include <string>

MtasSSDProcessor::MtasSSDProcessor(const std::string& log) : Processor(log,"MtasSSDProcessor",{"silicon"}){
	this->h1dsettings = { 
				{1000 , {16384,0.0,16384}},
				{1500 , {14,0,14}}
			    };

	this->h2dsettings = {
				{2000 , {8192,0,8192,14,0,14}},
				{2500 , {8192,0,8192,14,0,14}}
			    };
	
	this->Maxidx = -1;
	this->MaxErg = 0.0;

	this->TopSiHits = std::vector<int>(7,0);
	this->TopSi = std::vector<double>(7,0.0);

	this->BottomSiHits = std::vector<int>(7,0);
	this->BottomSi = std::vector<double>(7,0.0);
}

[[maybe_unused]] bool MtasSSDProcessor::PreProcess(EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	auto summary =  eventhistory->GetCurrentEventSummary();
	summary->GetDetectorSummary(this->AllDefaultRegex["silicon"],this->SummaryData);
	for( const auto& evt : this->SummaryData ){
		const auto group = std::stoi(evt->GetGroup());
		if( group < 0 or group > 7 ){
			this->console->error("evt : {}, has group outside [0-6]",*evt);
			throw std::runtime_error("misconfigured xml");
		}	
		bool IsTop = evt->GetSubType().compare("top") == 0;
		bool IsBottom = evt->GetSubType().compare("bottom") == 0;

		const auto erg = evt->GetEnergy();
		if( erg > this->MaxErg ){
			this->MaxErg = erg;
			this->Maxidx = group + 7*IsBottom;
		}


		if( IsTop ){
			++(this->TopSiHits[group]);
			if( erg > this->TopSi[group] ){
				this->TopSi[group] = erg;
			}
		}else if( IsBottom ){
			++(this->BottomSiHits[group]);
			if( erg > this->BottomSi[group] ){
				this->BottomSi[group] = erg;
			}
		}else{
			this->console->error("evt : {}, has neither top or bottom subtype",*evt);
			throw std::runtime_error("misconfigured xml");
		}
	}

	hismanager->Fill("SILICON_1000",this->MaxErg);
	hismanager->Fill("SILICON_1500",this->Maxidx);
	hismanager->Fill("SILICON_2500",this->MaxErg,this->Maxidx);

	for( size_t ii = 0; ii < 7; ++ii ){
		hismanager->Fill("SILICON_2000",this->TopSi[ii],ii);
		hismanager->Fill("SILICON_2000",this->BottomSi[ii],ii+7);
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool MtasSSDProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool MtasSSDProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->Reset();

	return true;
}

void MtasSSDProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}
		
void MtasSSDProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void MtasSSDProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager){
	//MtasSSD diagnostic plots, always want these no matter what
	hismanager->RegisterPlot<TH1F>("SILICON_1000","Max Si Energy; Energy (keV)",this->h1dsettings.at(1000));
	hismanager->RegisterPlot<TH1F>("SILICON_1500","Largest Si Position (Top - Bottom); Strip (arb.)",this->h1dsettings.at(1500));
	hismanager->RegisterPlot<TH2F>("SILICON_2000","Si Energy; Energy (keV); Strip (arb.)",this->h2dsettings.at(2000));
	hismanager->RegisterPlot<TH2F>("SILICON_2500","Max Si Energy; Energy (keV); Strip (arb.)",this->h2dsettings.at(2500));
	this->console->info("Finished Declaring Plots");
}

void MtasSSDProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
}

void MtasSSDProcessor::CleanupTree(){
}

void MtasSSDProcessor::Reset(){
	this->Maxidx = -1;
	this->MaxErg = 0.0;

	for( int ii = 0; ii < 7; ++ii ){
		this->TopSiHits[ii] = 0;
		this->BottomSiHits[ii] = 0;
		this->TopSi[ii] = 0.0;
		this->BottomSi[ii] = 0.0;
	}
}

double MtasSSDProcessor::GetMaxEnergy() const{
	return this->MaxErg;
}

double MtasSSDProcessor::GetTopEnergy(int idx) const{
	return this->TopSi[idx];
}

double MtasSSDProcessor::GetBottomEnergy(int idx) const{
	return this->BottomSi[idx];
}
