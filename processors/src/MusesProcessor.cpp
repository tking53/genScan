#include "MusesProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "MusesStruct.hpp"
#include <TTree.h>
#include <stdexcept>
#include <string>

MusesProcessor::MusesProcessor(const std::string& log)
	: Processor(log, "MusesProcessor", {"muses"}) {
	this->h1dsettings = {
		{1000, {16384, 0.0, 16384}},
		{1500, {9, 0, 9}}};

	this->h2dsettings = {
		{2000, {8192, 0, 8192, 9, 0, 9}},
		{2500, {8192, 0, 8192, 9, 0, 9}},
		{3000, {4096, 0, 4096, 4096, 0, 4096}},
		{30008, {2048, 0, 16384, 2048, 0, 16384}},
		{4000, {9, -2, 2, 9, -2, 2}}};

	this->Maxidx = -1;
	this->MaxErg = 0.0;

	this->FirstTime = -1.0;
	this->LastTime = -1.0;

	this->PixelHits = std::vector<int>(9, 0);
	this->Pixels = std::vector<double>(9, 0.0);
	this->Pix = std::vector<ProcessorStruct::MusesPixel>(9, ProcessorStruct::DEFAULT_MUSES_PIXEL_STRUCT);
	this->Positions = {
		{-0.5, 0.5},
		{0.0, 0.5},
		{0.5, 0.5},
		{-0.5, 0.0},
		{0.0, 0.0},
		{0.5, 0.0},
		{-0.5, -0.5},
		{0.0, -0.5},
		{0.5, -0.5},
	};
}

[[maybe_unused]] bool MusesProcessor::PreProcess(EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	Processor::PreProcess();

	auto summary = eventhistory->GetCurrentEventSummary();
	summary->GetDetectorSummary(this->AllDefaultRegex["muses"], this->SummaryData);
	for (const auto& evt : this->SummaryData) {
		const auto group = std::stoi(evt->GetGroup());
		if (group < 0 or group > 8) {
			this->console->error("evt : {}, has group outside [0-8]", *evt);
			throw std::runtime_error("misconfigured xml");
		}

		const auto erg = evt->GetEnergy();
		if (erg > this->MaxErg) {
			this->MaxErg = erg;
			this->Maxidx = group;
		}

		++(this->PixelHits[group]);
		if (erg > this->Pixels[group]) {
			this->Pixels[group] = erg;
			this->TimeStamps.push_back(evt->GetTimeStamp());
			this->Pix[group].timestamp = evt->GetTimeStamp();
			this->Pix[group].energy = erg;
			this->Pix[group].pileup = evt->GetPileup();
			this->Pix[group].saturate = evt->GetSaturation();
			this->Pix[group].pixelid = group;
		}
	}

	if (this->TimeStamps.size() > 0) {
		this->FirstTime = *(std::min_element(this->TimeStamps.begin(), this->TimeStamps.end()));
		this->LastTime = *(std::max_element(this->TimeStamps.begin(), this->TimeStamps.end()));
	}

	hismanager->Fill("SILICON_1000", this->MaxErg);
	hismanager->Fill("SILICON_1500", this->Maxidx);
	hismanager->Fill("SILICON_2500", this->MaxErg, this->Maxidx);

	auto SILICON_2000 = hismanager->GetPlot<TH2*>("SILICON_2000");
	auto SILICON_3000 = hismanager->GetPlot<TH2*>("SILICON_3000");
	auto SILICON_30008 = hismanager->GetPlot<TH2*>("SILICON_30008");
	auto SILICON_4000 = hismanager->GetPlot<TH2*>("SILICON_4000");
	for (size_t ii = 0; ii < 9; ++ii) {
		SILICON_2000->Fill(this->Pixels[ii], ii);
		if (this->Pixels[ii] > 1.0) {
			auto pos = this->Positions[ii];
			// this->console->info("{} : {},{}",ii,pos.first,pos.second);
			SILICON_4000->Fill(pos.first, pos.second);
		}
		for (size_t jj = ii + 1; jj < 9; ++jj) {
			SILICON_3000->Fill(this->Pixels[ii], this->Pixels[jj]);
			SILICON_3000->Fill(this->Pixels[jj], this->Pixels[ii]);

			SILICON_30008->Fill(this->Pixels[ii], this->Pixels[jj]);
			SILICON_30008->Fill(this->Pixels[jj], this->Pixels[ii]);
		}
	}

	summary->AddEventObservable("SiMax", this->MaxErg);

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool MusesProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	return true;
}

[[maybe_unused]] bool MusesProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	this->Reset();

	return true;
}

void MusesProcessor::Init(const pugi::xml_node& config) {
	this->console->info("Init called with pugi::xml_node");
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void MusesProcessor::Finalize() {
	this->console->info("{} has been finalized", this->ProcessorName);
}

void MusesProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) {
	// Muses diagnostic plots, always want these no matter what
	hismanager->RegisterPlot<TH1F>("SILICON_1000", "Max Si Energy; Energy (keV)", this->h1dsettings.at(1000));
	hismanager->RegisterPlot<TH1F>("SILICON_1500", "Largest Si Position ; Pixel (arb.)", this->h1dsettings.at(1500));

	hismanager->RegisterPlot<TH2F>("SILICON_2000", "Si Energy; Energy (keV); Pixel (arb.)", this->h2dsettings.at(2000));
	hismanager->RegisterPlot<TH2F>("SILICON_2500", "Max Si Energy; Energy (keV); Pixel (arb.)", this->h2dsettings.at(2500));
	hismanager->RegisterPlot<TH2F>("SILICON_3000", "Si-Si Matrix; Energy (keV); Energy (keV)", this->h2dsettings.at(3000));
	hismanager->RegisterPlot<TH2F>("SILICON_30008", "Si-Si Matrix; Energy (keV); Energy (keV)", this->h2dsettings.at(30008));
	hismanager->RegisterPlot<TH2F>("SILICON_4000", "Hit Map; Pixel (arb.); Pixel (arb.)", this->h2dsettings.at(4000));
	this->console->info("Finished Declaring Plots");
}

void MusesProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string, TTree*>& outputtrees) {
	this->OutputTree = new TTree("Muses", "Muses Processor output");
	this->OutputTree->Branch("pixel", &(this->Pix));
	outputtrees[this->ProcessorName] = this->OutputTree;
}

void MusesProcessor::CleanupTree() {
	for (auto& p : this->Pix) {
		p = ProcessorStruct::DEFAULT_MUSES_PIXEL_STRUCT;
	}
}

void MusesProcessor::Reset() {
	this->Maxidx = -1;
	this->MaxErg = 0.0;

	this->TimeStamps.clear();

	for (int ii = 0; ii < 9; ++ii) {
		this->PixelHits[ii] = 0;
		this->Pixels[ii] = 0.0;
	}
}

double MusesProcessor::GetMaxEnergy() const {
	return this->MaxErg;
}

double MusesProcessor::GetPixelEnergy(int idx) const {
	return this->Pixels[idx];
}

const double& MusesProcessor::GetFirstFireTime() const {
	return this->FirstTime;
}

const double& MusesProcessor::GetLastFireTime() const {
	return this->LastTime;
}
