#include "CloverProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>
#include <string>
#include <cctype>
#include "boost/algorithm/string.hpp"
#include "boost/regex.hpp"

CloverProcessor::CloverProcessor(const std::string& log)
	: Processor(log, "CloverProcessor", {"clover"}) {
	this->h2dsettings = {
		{100, {16384, 0.0, 16384.0, 96, 0, 96}},
		{101, {16384, 0.0, 16384.0, 24, 0.0, 24}},
		{101, {16384, 0.0, 16384.0, 24, 0.0, 24}},
	};
};

[[maybe_unused]] bool CloverProcessor::PreProcess(EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	Processor::PreProcess();

	auto summary = eventhistory->GetCurrentEventSummary();
	summary->GetDetectorSummary(this->AllDefaultRegex["clover"], this->SummaryData);
	// Expect uid style: clover:<clover number>:leaf:<leaf name>:(tags...)
	for (const auto& evt : this->SummaryData) {
		auto cloverNum = evt->GetSubType();
		auto leaf = evt->GetGroup();
		int cloveridx = std::stoi(cloverNum) - 1;
		if (cloveridx > this->NumClover) {
			this->console->warn("Clover index {} is out of bounds for NumClover {}", cloveridx, this->NumClover);
			throw std::runtime_error("Clover index out of range. XML error");
		}
		if (leaf.compare("U") == 0) {
			FillCloverLeafStruc(evt, this->CloverDataVec.at(cloveridx).U, cloveridx);
		} else if (leaf.compare("B") == 0) {
			FillCloverLeafStruc(evt, this->CloverDataVec.at(cloveridx).B, cloveridx);
		} else if (leaf.compare("G") == 0) {
			FillCloverLeafStruc(evt, this->CloverDataVec.at(cloveridx).G, cloveridx);
		} else if (leaf.compare("R") == 0) {
			FillCloverLeafStruc(evt, this->CloverDataVec.at(cloveridx).R, cloveridx);
		} else {
			this->console->warn("Unknown leaf {} for Clover {}", leaf, cloverNum);
			throw std::runtime_error("Unknown Clover:Leaf Specificiation. XML error");
		};
	};

	auto CLOVER_100 = hismanager->GetPlot<TH2*>("CLOVER_100");
	auto CLOVER_101 = hismanager->GetPlot<TH2*>("CLOVER_101");
	auto CLOVER_102 = hismanager->GetPlot<TH2*>("CLOVER_102");

	for (int i = 0; i < this->CloverDataVec.size(); ++i) {
		auto& clover = this->CloverDataVec[i];
		// This is a very simple in clover addback.
		if (clover.U.energy > 0.0 && !clover.U.pileup && !clover.U.saturation) {
			clover.addbackEn += clover.U.energy;
			CLOVER_100->Fill(clover.U.energy, i * LeavesPerClover + 0);
			CLOVER_101->Fill(clover.U.energy, i);
			if (clover.addbackTS.first > clover.U.time) {
				clover.addbackTS.first = clover.U.time;
			}
			if (clover.addbackTS.second < clover.U.time) {
				clover.addbackTS.second = clover.U.time;
			}
		}
		if (clover.B.energy > 0.0 && !clover.B.pileup && !clover.B.saturation) {
			clover.addbackEn += clover.B.energy;
			CLOVER_100->Fill(clover.B.energy, i * LeavesPerClover + 1);
			CLOVER_101->Fill(clover.B.energy, i);
			if (clover.addbackTS.first > clover.B.time) {
				clover.addbackTS.first = clover.B.time;
			}
			if (clover.addbackTS.second < clover.B.time) {
				clover.addbackTS.second = clover.B.time;
			}
		}
		if (clover.G.energy > 0.0 && !clover.G.pileup && !clover.G.saturation) {
			clover.addbackEn += clover.G.energy;
			CLOVER_100->Fill(clover.G.energy, i * LeavesPerClover + 2);
			CLOVER_101->Fill(clover.G.energy, i);
			if (clover.addbackTS.first > clover.G.time) {
				clover.addbackTS.first = clover.G.time;
			}
			if (clover.addbackTS.second < clover.G.time) {
				clover.addbackTS.second = clover.G.time;
			}
		}
		if (clover.R.energy > 0.0 && !clover.R.pileup && !clover.R.saturation) {
			clover.addbackEn += clover.R.energy;
			CLOVER_100->Fill(clover.R.energy, i * LeavesPerClover + 3);
			CLOVER_101->Fill(clover.R.energy, i);
			if (clover.addbackTS.first > clover.R.time) {
				clover.addbackTS.first = clover.R.time;
			}
			if (clover.addbackTS.second < clover.R.time) {
				clover.addbackTS.second = clover.R.time;
			}
		}
		if (clover.addbackEn > 0.0) {
			CLOVER_102->Fill(clover.addbackEn, i);
		}
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool CloverProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	return true;
}

[[maybe_unused]] bool CloverProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	this->Reset();

	return true;
}

void CloverProcessor::Init(const pugi::xml_node& config) {
	this->console->info("Init called with pugi::xml_node");

	this->NumClover = config.attribute("NumClover").as_int(24);

	// FDSi Clovers are 1 counting. Im going to handle that during PreProcess so hopefully its blind
	CloverDataVec = std::vector<ProcessorStruct::Clover>(this->NumClover, ProcessorStruct::DEFAULT_CLOVER_STRUCT);

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void CloverProcessor::Finalize() {
	this->console->info("{} has been finalized", this->ProcessorName);
}

void CloverProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) {
	// Clover diagnostic plots
	hismanager->RegisterPlot<TH2F>("CLOVER_100", "Clover Energies: Port(UBGR); Energy (keV); Leaf index: (arb.)", 16384, 0, 16384, this->NumClover * LeavesPerClover, 0, this->NumClover * LeavesPerClover);
	hismanager->RegisterPlot<TH2F>("CLOVER_101", "Clover Energies: Leafs Stacked; Energy (keV); Clover index: (arb.)", 16384, 0, 16384, this->NumClover, 0, this->NumClover);
	hismanager->RegisterPlot<TH2F>("CLOVER_102", "Clover Energies: In CloverAddback; Energy (keV); Clover index (arb.)", 16384, 0, 16384, this->NumClover, 0, this->NumClover);

	//! Ideas for later need some clever math to avoid giant if tree of or statments. this is probably fine, but maybe letting the compiler optimize it to a hash table is better?
	// hismanager->RegisterPlot<TH2F>("CLOVER_103", "2Pi Clover Rings (stacked); Clover Energy (keV); Ring ID (arb.)", 16384, 0, 16384, 8, 0, 8);
	// hismanager->RegisterPlot<TH2F>("CLOVER_104", "4Pi Clover Rings (stacked); Clover Energy (keV); Ring ID (arb.)", 16384, 0, 16384, 4, 0, 4);

	this->console->info("Finished Declaring Plots");
}

void CloverProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string, TTree*>& outputtrees) {
	this->OutputTree = new TTree("Clover", "Clover Processor Output");
	for (int i = 0; i < this->NumClover; ++i) {
		std::string name = "C" + std::to_string(i + 1);
		this->OutputTree->Branch(name.c_str(), &(this->CloverDataVec.at(i)));
	}
	outputtrees[this->ProcessorName] = this->OutputTree;
}

void CloverProcessor::CleanupTree() {
	for (auto& it : CloverDataVec) {
		it = ProcessorStruct::DEFAULT_CLOVER_STRUCT;
	}
}

void CloverProcessor::Reset() {
}

int CloverProcessor::GetNumCrystals() const {
	return (this->NumClover * this->LeavesPerClover);
}

void CloverProcessor::FillCloverLeafStruc(PhysicsData* data, ProcessorStruct::Leaf& leaf, const int& cloveridx) {
	leaf.energy = data->GetEnergy();
	leaf.time = data->GetTimeStamp();
	leaf.pileup = data->GetPileup();
	leaf.saturation = data->GetSaturation();
	leaf.theta = this->GetLeafAngles(cloveridx, data->GetGroup()).first;
	leaf.phi = this->GetLeafAngles(cloveridx, data->GetGroup()).second;
}

std::pair<double, double> CloverProcessor::GetLeafAngles(const int& cloveridx, const std::string& leafname) {
	// Placeholder function, need to parse from XML
	// For now we just return the default values
	return {-999.0, -999.0};
}
