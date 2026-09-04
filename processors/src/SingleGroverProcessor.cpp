#include "SingleGroverProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "GroverStruct.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>
#include <string>

SingleGroverProcessor::SingleGroverProcessor(const std::string& log)
	: Processor(log, "SingleGroverProcessor", {"grover"}) {
	this->h2dsettings = {
		{1000, {65536, 0, 65536, 4, 0, 4}},
		{1001, {65536, 0, 65536, 4, 0, 4}},
		{2000, {4096, 0, 4096, 4096, 0, 4096}},
		{20008, {2048, 0, 16384, 2048, 0, 16384}},
		{2001, {4096, 0, 4096, 4096, 0, 4096}},
		{20018, {2048, 0, 16384, 2048, 0, 16384}}};

	this->HighGain = std::vector<GroverInfo>(4, {.energy = 0.0, .timestamp = -1.0, .hits = 0, .saturate = false, .pileup = false});
	this->LowGain = std::vector<GroverInfo>(4, {.energy = 0.0, .timestamp = -1.0, .hits = 0, .saturate = false, .pileup = false});

	this->HG = std::vector<ProcessorStruct::GroverLeaf>(4, ProcessorStruct::DEFAULT_GROVER_LEAF_STRUCT);
	this->LG = std::vector<ProcessorStruct::GroverLeaf>(4, ProcessorStruct::DEFAULT_GROVER_LEAF_STRUCT);

	this->highgaintag = "highgain";
	this->lowgaintag = "lowgain";
}

[[maybe_unused]] bool SingleGroverProcessor::PreProcess(EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	Processor::PreProcess();

	auto summary = eventhistory->GetCurrentEventSummary();
	summary->GetDetectorSummary(this->AllDefaultRegex["grover"], this->SummaryData);
	for (const auto& evt : this->SummaryData) {
		const auto group = std::stoi(evt->GetGroup());
		auto ishighgain = evt->HasTag(this->highgaintag);
		auto islowgain = evt->HasTag(this->lowgaintag);

		if ((not ishighgain and not islowgain) or (ishighgain and islowgain)) {
			this->console->error("evt {} has neither highgain of lowgain or both highgain and lowgain tags", *evt);
			throw std::runtime_error("invalid xml config");
		}

		const auto erg = evt->GetEnergy();
		const auto timestamp = evt->GetTimeStamp();
		const auto pileup = evt->GetPileup();
		const auto saturate = evt->GetSaturation();
		if (ishighgain) {
			++(this->HighGain[group].hits);
			this->HighGain[group].energy = erg;
			this->HighGain[group].timestamp = timestamp;
			this->HighGain[group].pileup = pileup;
			this->HighGain[group].saturate = saturate;

			this->HG[group].energy = erg;
			this->HG[group].timestamp = timestamp;
			this->HG[group].pileup = pileup;
			this->HG[group].saturate = saturate;
			this->HG[group].leafid = group;
		} else {
			++(this->LowGain[group].hits);
			this->LowGain[group].energy = erg;
			this->LowGain[group].timestamp = timestamp;
			this->LowGain[group].pileup = pileup;
			this->LowGain[group].saturate = saturate;

			this->LG[group].energy = erg;
			this->LG[group].timestamp = timestamp;
			this->LG[group].pileup = pileup;
			this->LG[group].saturate = saturate;
			this->LG[group].leafid = group;
		}
	}

	auto HPGE_1000 = hismanager->GetPlot<TH2*>("HPGE_1000");
	auto HPGE_1001 = hismanager->GetPlot<TH2*>("HPGE_1001");
	auto HPGE_2000 = hismanager->GetPlot<TH2*>("HPGE_2000");
	auto HPGE_20008 = hismanager->GetPlot<TH2*>("HPGE_20008");
	auto HPGE_2001 = hismanager->GetPlot<TH2*>("HPGE_2001");
	auto HPGE_20018 = hismanager->GetPlot<TH2*>("HPGE_20018");
	for (size_t ii = 0; ii < 4; ++ii) {
		HPGE_1000->Fill(this->LowGain[ii].energy, ii);
		HPGE_1001->Fill(this->HighGain[ii].energy, ii);
		for (size_t jj = ii + 1; jj < 4; ++jj) {
			HPGE_2000->Fill(this->LowGain[ii].energy, this->LowGain[jj].energy);
			HPGE_2000->Fill(this->LowGain[jj].energy, this->LowGain[ii].energy);

			HPGE_20008->Fill(this->LowGain[ii].energy, this->LowGain[jj].energy);
			HPGE_20008->Fill(this->LowGain[jj].energy, this->LowGain[ii].energy);

			HPGE_2001->Fill(this->HighGain[ii].energy, this->HighGain[jj].energy);
			HPGE_2001->Fill(this->HighGain[jj].energy, this->HighGain[ii].energy);

			HPGE_20018->Fill(this->HighGain[ii].energy, this->HighGain[jj].energy);
			HPGE_20018->Fill(this->HighGain[jj].energy, this->HighGain[ii].energy);
		}
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool SingleGroverProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	return true;
}

[[maybe_unused]] bool SingleGroverProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	this->Reset();

	return true;
}

void SingleGroverProcessor::Init(const pugi::xml_node& config) {
	this->console->info("Init called with pugi::xml_node");

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void SingleGroverProcessor::Finalize() {
	this->console->info("{} has been finalized", this->ProcessorName);
}

void SingleGroverProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) {
	// SingleGrover diagnostic plots, always want these no matter what
	hismanager->RegisterPlot<TH2F>("HPGE_1000", "Low Gain Energies; Energy (keV); Leaf (arb.)", this->h2dsettings.at(1000));
	hismanager->RegisterPlot<TH2F>("HPGE_1001", "High Gain Energies; Energy (keV); Leaf (arb.)", this->h2dsettings.at(1001));

	hismanager->RegisterPlot<TH2F>("HPGE_2000", "Low Gain #gamma-#gamma Matrix; Energy (keV); Energy (keV)", this->h2dsettings.at(2000));
	hismanager->RegisterPlot<TH2F>("HPGE_20008", "Low Gain #gamma-#gamma Matrix; Energy (8 keV/bin); Energy (8 keV/bin)", this->h2dsettings.at(20008));

	hismanager->RegisterPlot<TH2F>("HPGE_2001", "High Gain #gamma-#gamma Matrix; Energy (keV); Energy (keV)", this->h2dsettings.at(2000));
	hismanager->RegisterPlot<TH2F>("HPGE_20018", "High Gain #gamma-#gamma Matrix; Energy (8 keV/bin); Energy (8 keV/bin)", this->h2dsettings.at(20008));
	this->console->info("Finished Declaring Plots");
}

void SingleGroverProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string, TTree*>& outputtrees) {
	this->OutputTree = new TTree("Grover", "SingleGrover Processor output");
	this->OutputTree->Branch("highgain", &(this->HG));
	this->OutputTree->Branch("lowgain", &(this->LG));
	outputtrees[this->ProcessorName] = this->OutputTree;
}

void SingleGroverProcessor::CleanupTree() {
	for (auto& g : this->HG) {
		g = ProcessorStruct::DEFAULT_GROVER_LEAF_STRUCT;
	}
	for (auto& g : this->LG) {
		g = ProcessorStruct::DEFAULT_GROVER_LEAF_STRUCT;
	}
}

void SingleGroverProcessor::ResetInfo(GroverInfo& info) {
	info.energy = 0.0;
	info.hits = 0;
	info.timestamp = -1.0;
	info.pileup = false;
	info.saturate = false;
}

void SingleGroverProcessor::Reset() {
	for (auto& g : this->HighGain) {
		this->ResetInfo(g);
	}
	for (auto& g : this->LowGain) {
		this->ResetInfo(g);
	}
}

double SingleGroverProcessor::GetEnergy(int idx, bool ishighgain) const {
	return ishighgain ? this->HighGain[idx].energy : this->LowGain[idx].energy;
}

double SingleGroverProcessor::GetCrystalFireTime(int idx, bool ishighgain) const {
	return ishighgain ? this->HighGain[idx].timestamp : this->LowGain[idx].timestamp;
}

bool SingleGroverProcessor::DidCrystalPileup(int idx, bool ishighgain) const {
	return ishighgain ? this->HighGain[idx].pileup : this->LowGain[idx].pileup;
}

bool SingleGroverProcessor::DidCrystalSaturate(int idx, bool ishighgain) const {
	return ishighgain ? this->HighGain[idx].saturate : this->LowGain[idx].saturate;
}
