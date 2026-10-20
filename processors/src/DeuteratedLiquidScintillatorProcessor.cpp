#include "DeuteratedLiquidScintillatorProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>
#include <string>

DeuteratedLiquidScintillatorProcessor::DeuteratedLiquidScintillatorProcessor(const std::string& log)
	: Processor(log, "DeuteratedLiquidScintillatorProcessor", {"dls"}) {

	this->h2dsettings = {
		// Energy plots (we ignore the yaxis limits)
		{1000, {4096, 0, 4096, -1, -1, -1}},
		// PSD plots (we ignore the yaxis limits)
		{2000, {1000, 0, 1.0, -1, -1, -1}},
		// Trace Max plots (we ignore the yaxis limits)
		{4000, {16384,0, 16384, -1, -1, -1}},
		// PSD plots
		{3000, {8192, 0, 8192, 1000, 0, 1.0}},
		};


}

[[maybe_unused]] bool DeuteratedLiquidScintillatorProcessor::PreProcess(EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	Processor::PreProcess();

	auto summary = eventhistory->GetCurrentEventSummary();
	summary->GetDetectorSummary(this->AllDefaultRegex["dls"], this->SummaryData);
	for (const auto& evt : this->SummaryData) {
		auto detpos = std::stoi(evt->GetGroup());
		// should set the PSD to be based on the relevant portions of the baseline subtracted trace ....
		// but fall back to the values from digitizer?
		this->data[detpos] = {(1.0 - evt->GetQDC(this->psd_idx) / evt->GetEnergy()),evt->GetEnergy(),evt->GetTimeStamp(), std::abs(evt->GetBaselineSubtractedPSDBoundedMaxValue()), evt->GetSaturation(), evt->GetPileup()};
	}

	for (size_t ii = 0; ii < this->data.size(); ++ii) {
		hismanager->Fill("DLS_1000", this->data[ii].energy, ii);
		hismanager->Fill("DLS_2000", this->data[ii].psd, ii);
		hismanager->Fill("DLS_4000", this->data[ii].tmax, ii);
		std::string name = "DLS_3000_" + std::to_string(ii);
		hismanager->Fill(name,this->data[ii].energy,this->data[ii].psd);
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool DeuteratedLiquidScintillatorProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	return true;
}

[[maybe_unused]] bool DeuteratedLiquidScintillatorProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	this->Reset();

	return true;
}

void DeuteratedLiquidScintillatorProcessor::Init(const pugi::xml_node& config) {
	this->console->info("Init called with pugi::xml_node");

	this->NumScintillator = config.attribute("number").as_int(1);
	this->psd_idx = config.attribute("psd_idx").as_ullong(0);

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void DeuteratedLiquidScintillatorProcessor::Finalize() {
	this->data = std::vector<DLSData>(this->NumScintillator,{-999.0,0.0,-1.0,false,false});
	this->console->info("{} has been finalized", this->ProcessorName);
}

void DeuteratedLiquidScintillatorProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) {

	auto override_settings = [](auto& settings, const auto& ns) {
		settings.nbinsy = ns;
		settings.ylow = 0;
		settings.yhigh = ns;
	};

	override_settings(this->h2dsettings.at(1000), this->NumScintillator);
	override_settings(this->h2dsettings.at(2000), this->NumScintillator);
	override_settings(this->h2dsettings.at(4000), this->NumScintillator);

	// DeuteratedLiquidScintillator diagnostic plots, always want these no matter what
	hismanager->RegisterPlot<TH2F>("DLS_1000", "Scintillator Energies; Energy (keV); Scintillator number (arb.)", this->h2dsettings.at(1000));
	hismanager->RegisterPlot<TH2F>("DLS_2000", "Scintillator PSDs; PSD (arb.); Scintillator number (arb.)", this->h2dsettings.at(2000));
	hismanager->RegisterPlot<TH2F>("DLS_4000", "Scintillator Trace Max; Trace Max (arb.); Scintillator number (arb.)", this->h2dsettings.at(4000));
	for ( int ii = 0; ii < this->NumScintillator; ++ii ) {
		std::string name = "DLS_3000_"+std::to_string(ii);
		std::string title = "Scintillator " + std::to_string(ii) + " PSD v Energy; Energy (keV); PSD (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3000));
	}
	this->console->info("Finished Declaring Plots");
}

void DeuteratedLiquidScintillatorProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string, TTree*>& outputtrees) {
}

void DeuteratedLiquidScintillatorProcessor::CleanupTree() {
}

void DeuteratedLiquidScintillatorProcessor::Reset() {
	for (int ii = 0; ii < this->NumScintillator; ++ii) {
		this->data[ii] = {-999.0,0.0,-1.0,-1.0,false,false};
	}
}

double DeuteratedLiquidScintillatorProcessor::GetPSD(int idx) const {
	return this->data[idx].psd;
}

double DeuteratedLiquidScintillatorProcessor::GetEnergy(int idx) const {
	return this->data[idx].energy;
}

double DeuteratedLiquidScintillatorProcessor::GetScintillatorFireTime(int idx) const {
	return this->data[idx].ts;
}

double DeuteratedLiquidScintillatorProcessor::GetTraceMax(int idx) const {
	return this->data[idx].tmax;
}

bool DeuteratedLiquidScintillatorProcessor::DidScintillatorPileup(int idx) const {
	return this->data[idx].pileup;
}

bool DeuteratedLiquidScintillatorProcessor::DidScintillatorSaturate(int idx) const {
	return this->data[idx].saturate;
}

int DeuteratedLiquidScintillatorProcessor::GetNumScintillators() const {
	return this->NumScintillator;
}
