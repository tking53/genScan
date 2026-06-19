#include "EBSSProcessor.hpp"
#include "EBSSStruct.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>

EBSSProcessor::EBSSProcessor(const std::string& log)
	: Processor(log, "EBSSProcessor", {"nai"}) {
	this->h2dsettings = {
		{100, {16384, 0, 16384, this->NumPaddles, static_cast<double>(this->firstPaddleGroup), static_cast<double>(this->NumPaddles + this->firstPaddleGroup)}},
		{300, {8192, 0, 8192, 8192, 0, 8192}}};
	this->h1dsettings = {
		{200, {16384, 0, 16384}},
	};
}

[[maybe_unused]] bool EBSSProcessor::PreProcess(EventHistoryManager* eventhistory, PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	Processor::PreProcess();

	auto summary = eventhistory->GetCurrentEventSummary();
	summary->GetDetectorSummary(this->AllDefaultRegex["nai"], this->SummaryData);

	auto EBSS_1 = hismanager->GetPlot<TH2*>("EBSS_1");
	auto EBSS_2 = hismanager->GetPlot<TH1*>("EBSS_2");
	auto EBSS_3 = hismanager->GetPlot<TH2*>("EBSS_3");

	for (const auto& evt : this->SummaryData) {
		int detpos = std::stoi(evt->GetGroup());
		if (detpos < firstPaddleGroup || detpos >= (NumPaddles + firstPaddleGroup)) {
			this->console->warn("EBSSProcessor: paddle group {} out of range [{} ,{}), skipping", detpos, firstPaddleGroup, NumPaddles + firstPaddleGroup);
			continue;
		}
		if (PaddleData[detpos].paddle_id != -1) {
			this->console->warn("EBSSProcessor: multiple hits in paddle {}, overwriting previous hit", detpos);
		}
		PaddleData[detpos].timestamp = evt->GetTimeStamp();
		PaddleData[detpos].energy = evt->GetEnergy();
		PaddleData[detpos].paddle_id = detpos;
		PaddleData[detpos].saturate = evt->GetSaturation();
		PaddleData[detpos].pileup = evt->GetPileup();

		EBSS_1->Fill(evt->GetEnergy(), detpos);
	}

	int multi = 0;
	for (const auto& evt : this->PaddleData) {
		if (evt.energy > 0 && !evt.saturate && !evt.pileup) {
			TotalEventEnergy += evt.energy;
			++multi;
		}
	}
	EBSS_2->Fill(TotalEventEnergy);

	TotalData.sumenergy = TotalEventEnergy;
	TotalData.multiplicy = multi;

	for (const auto& evt : this->PaddleData) {
		if (evt.energy > 0 && !evt.saturate && !evt.pileup) {
			EBSS_3->Fill(TotalEventEnergy, evt.energy);
		}
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool EBSSProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	return true;
}

[[maybe_unused]] bool EBSSProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	this->Reset();
	return true;
}

void EBSSProcessor::Init(const pugi::xml_node& config) {
	this->console->info("Init called with pugi::xml_node");
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);

	PaddleData = std::vector<ProcessorStruct::EBSSPaddle>((this->NumPaddles + this->firstPaddleGroup), ProcessorStruct::DEFAULT_EBSS_PADDLE_STRUCT);
	TotalData = ProcessorStruct::DEFAULT_EBSS_TOTAL_STRUCT;
}

void EBSSProcessor::Finalize() {
	this->console->info("{} has been finalized", this->ProcessorName);
}

void EBSSProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) {
	hismanager->RegisterPlot<TH2F>("EBSS_1", "NaI Paddle Calibrated; Energy (keV); Paddle Index", this->h2dsettings.at(100));
	hismanager->RegisterPlot<TH1F>("EBSS_2", "NaI Paddle Total Event Energy; Energy (keV); Counts", this->h1dsettings.at(200));
	hismanager->RegisterPlot<TH2F>("EBSS_3", "NaI Paddle vs Total; Total Energy (keV); Energy (keV)", this->h2dsettings.at(300));

	this->console->info("Finished Declaring Plots");
}

void EBSSProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string, TTree*>& outputtrees) {
	this->OutputTree = new TTree("EBSS", "EBSS NaI Paddle Processor output");
	// Starting from 3 because this matches the labels / group numbers of the paddles in reality (cables 0-2 are at ANL for the BSM)
	for (int i = firstPaddleGroup; i < (NumPaddles + firstPaddleGroup); ++i) {
		std::string branchname = "paddle" + std::to_string(i);
		this->OutputTree->Branch(branchname.c_str(), &(this->PaddleData[i]));
	}
	this->OutputTree->Branch("total", &(this->TotalData));
	outputtrees[this->ProcessorName] = this->OutputTree;
}

void EBSSProcessor::CleanupTree() {
	for (auto& pad : this->PaddleData) {
		pad = ProcessorStruct::DEFAULT_EBSS_PADDLE_STRUCT;
	}
	TotalData = ProcessorStruct::DEFAULT_EBSS_TOTAL_STRUCT;
}

void EBSSProcessor::Reset() {
	TotalEventEnergy = 0.0;
	TotalEventEnergyDirty = 0.0;
}
