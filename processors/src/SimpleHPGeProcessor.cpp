#include "SimpleHPGeProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>
#include <string>

SimpleHPGeProcessor::SimpleHPGeProcessor(const std::string& log) : Processor(log,"SimpleHPGeProcessor",{"hpge"}){
}

[[maybe_unused]] bool SimpleHPGeProcessor::PreProcess(EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	auto summary = eventhistory->GetCurrentEventSummary();
	summary->GetDetectorSummary(this->AllDefaultRegex["hpge"],this->SummaryData);
	for( const auto& evt : this->SummaryData ){
		auto detpos = std::stoi(evt->GetGroup());
		this->Energies[detpos] = evt->GetEnergy();
	}

	for( size_t ii = 0; ii < this->Energies.size(); ++ii ){
		hismanager->Fill("HPGE_1000",this->Energies[ii],ii);
	}
	
	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool SimpleHPGeProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool SimpleHPGeProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->Reset();

	return true;
}

void SimpleHPGeProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");

	this->NumHPGe = config.attribute("number").as_int(1);

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}
		
void SimpleHPGeProcessor::Finalize(){
	this->Energies = std::vector<double>(this->NumHPGe,0.0);
	this->console->info("{} has been finalized",this->ProcessorName);
}

void SimpleHPGeProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager){
	//SimpleHPGe diagnostic plots, always want these no matter what
	hismanager->RegisterPlot<TH2F>("HPGE_1000","HPGe Energies; Energy (keV); HPGe number (arb.)",16384,0,16384,this->NumHPGe,0,this->NumHPGe);
	this->console->info("Finished Declaring Plots");
}

void SimpleHPGeProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
}

void SimpleHPGeProcessor::CleanupTree(){
}

void SimpleHPGeProcessor::Reset(){
	for( int ii = 0; ii < this->NumHPGe; ++ii ){
		this->Energies[ii] = 0.0;
	}
}

double SimpleHPGeProcessor::GetEnergy(int idx) const{
	return this->Energies[idx];
}
