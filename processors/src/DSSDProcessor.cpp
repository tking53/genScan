#include "DSSDProcessor.hpp"

#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"

#include <TTree.h>

#include <stdexcept>

DSSDProcessor::DSSDProcessor(const std::string& log)
	: Processor(log, "DSSDProcessor", {"dssd"}) {

	
	// Defining the standard 7 historgrams. We generate 1 copy of each for each DSSD 
	// XML Histogram settings match THESE ids not the ones actually in the root file.
	this->h2dsettings = {
		{0, {16384, 0.0, 16384.0, 4, 0, 4}},
		{1, {64, 0, 64, 64, 0, 64}},
		{2, {64, 0, 64, 64, 0, 64}},
		{10, {64, 0, 64, 3, 0, 3}},
		{11, {16384, 0, 16384, 64, 0, 64}},
		{12, {16384, 0, 16384, 64, 0, 64}},
		{13, {16384, 0, 16384, 64, 0, 64}},
		{14, {16384, 0, 16384, 64, 0, 64}}

	};
}

[[maybe_unused]] bool DSSDProcessor::PreProcess(EventHistoryManager* eventhistory, PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	Processor::PreProcess();

	auto summary = eventhistory->GetCurrentEventSummary();
	summary->GetDetectorSummary(this->AllDefaultRegex["dssd"], this->SummaryData);
	

	for (const auto& evt : this->SummaryData) {
		
		// Skip saturated/pileup events 
		if (evt->GetSaturation() || evt->GetPileup()) {
			//TODO: consider histogram of pipeup/saturation mult per event
			continue;
		}
		this->currStrip = stoi(evt->GetGroup());
		
		for (int i = 0; i < this->numDSSDs; ++i) {
			if (evt->HasTag("dssd" + std::to_string(i))) {
				this->currDSSD = i;
				break;
			}
		}

		const double currEnergy = evt->GetEnergy();

		if (this->currDSSD < 0 || this->currDSSD > this->numDSSDs || this->currStrip < 0 || this->currStrip > this->numstrips) {
			this->console->warn("Skipping out-of-range DSSD mapping (crate={}, module={}, channel={}, det={}, strip={}) for event {}", evt->GetCrate(), evt->GetModule(), evt->GetChannel(), this->currDSSD, this->currStrip, *evt);
			continue;
		}

		if (evt->GetSubType().compare("LL") == 0) {
			this->LL_Mult[this->currDSSD]++;
			// Beta branch of the LinLogs is expected to be below the LL_Branch_Energy_Split, Ion branch above. 
			if (currEnergy < this->LL_Branch_Energy_Split) {
				if (this->LL_maxEnEvt[this->currDSSD].first == nullptr || currEnergy > this->LL_maxEnEvt[this->currDSSD].first->GetEnergy()) {
					this->LL_maxEnEvt[this->currDSSD].first = evt;
				};
			} else {
				if (this->LL_maxEnEvt[this->currDSSD].second == nullptr || currEnergy > this->LL_maxEnEvt[this->currDSSD].second->GetEnergy()) {
					this->LL_maxEnEvt[this->currDSSD].second = evt;
				};
			}
		} else if (evt->GetSubType().compare("HG") == 0) {
			this->HG_Mult[this->currDSSD]++;
			if (this->HG_maxEnEvt[this->currDSSD] == nullptr || currEnergy > this->HG_maxEnEvt[this->currDSSD]->GetEnergy()) {
				this->HG_maxEnEvt[this->currDSSD] = evt;
			};
		} else if (evt->GetSubType().compare("LG") == 0) {
			this->LG_Mult[this->currDSSD]++;
			if (this->LG_maxEnEvt[this->currDSSD] == nullptr || currEnergy > this->LG_maxEnEvt[this->currDSSD]->GetEnergy()) {
				this->LG_maxEnEvt[this->currDSSD] = evt;
			};
		} else {
			this->console->warn("Unknown DSSD gain tag '{}' for Crate {}::Module {}::Channel {}, skipping", evt->GetSubType(), evt->GetCrate(), evt->GetModule(), evt->GetChannel());
			continue;
		}		
	}

	for (int i = 0; i < this->numDSSDs; ++i) {
		hismanager->Fill("DSSD_" + std::to_string(i) + "10", LL_Mult[i], 0);
		hismanager->Fill("DSSD_" + std::to_string(i) + "10", HG_Mult[i], 1);
		hismanager->Fill("DSSD_" + std::to_string(i) + "10", LG_Mult[i], 2);

		if (LL_maxEnEvt[i].first != nullptr) {
			BETA_DSSDData_vec[i].front.energy = LL_maxEnEvt[i].first->GetEnergy();
			BETA_DSSDData_vec[i].front.time = LL_maxEnEvt[i].first->GetTimeStamp();
			BETA_DSSDData_vec[i].front.stripnum = stoi(LL_maxEnEvt[i].first->GetGroup());
			hismanager->Fill("DSSD_" + std::to_string(i) + "00", LL_maxEnEvt[i].first->GetEnergy(), 0); // LL-B
		}
		if (LL_maxEnEvt[i].second != nullptr) {
			ION_DSSDData_vec[i].front.energy = LL_maxEnEvt[i].second->GetEnergy();
			ION_DSSDData_vec[i].front.time = LL_maxEnEvt[i].second->GetTimeStamp();
			ION_DSSDData_vec[i].front.stripnum = stoi(LL_maxEnEvt[i].second->GetGroup());
			hismanager->Fill("DSSD_" + std::to_string(i) + "00", LL_maxEnEvt[i].second->GetEnergy(), 1); // LL-I
		}
		if (HG_maxEnEvt[i] != nullptr) {
			BETA_DSSDData_vec[i].back.energy = HG_maxEnEvt[i]->GetEnergy();
			BETA_DSSDData_vec[i].back.time = HG_maxEnEvt[i]->GetTimeStamp();
			BETA_DSSDData_vec[i].back.stripnum = stoi(HG_maxEnEvt[i]->GetGroup());
			hismanager->Fill("DSSD_" + std::to_string(i) + "00", HG_maxEnEvt[i]->GetEnergy(), 2); // HG
		}
		if (LG_maxEnEvt[i] != nullptr) {
			ION_DSSDData_vec[i].back.energy = LG_maxEnEvt[i]->GetEnergy();
			ION_DSSDData_vec[i].back.time = LG_maxEnEvt[i]->GetTimeStamp();
			ION_DSSDData_vec[i].back.stripnum = stoi(LG_maxEnEvt[i]->GetGroup());
			hismanager->Fill("DSSD_" + std::to_string(i) + "00", LG_maxEnEvt[i]->GetEnergy(), 3); // LG
		}

		if (BETA_DSSDData_vec[i].front.stripnum >= 0 && BETA_DSSDData_vec[i].back.stripnum >= 0) {
			summary->AddEventTag("beta"); // rough general beta tag.
			hismanager->Fill("DSSD_" + std::to_string(i) + "01", BETA_DSSDData_vec[i].front.stripnum, BETA_DSSDData_vec[i].back.stripnum); // LL-B
		}
		if (ION_DSSDData_vec[i].front.stripnum >= 0 && ION_DSSDData_vec[i].back.stripnum >= 0) {
			summary->AddEventTag("implant"); // rough implant tag
			hismanager->Fill("DSSD_" + std::to_string(i) + "02", ION_DSSDData_vec[i].front.stripnum, ION_DSSDData_vec[i].back.stripnum); // LL-I
		}
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool DSSDProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {

	return true;
}

[[maybe_unused]] bool DSSDProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	this->Reset();

	return true;
}

void DSSDProcessor::Init(const pugi::xml_node& config) {
	this->console->info("Init called with pugi::xml_node");
    this->numstrips = config.attribute("NumStrips").as_int(32);
    this->numDSSDs = config.attribute("NumDSSDs").as_int(1);

	// This is the energy value above which we expect events to be in the Ion branch of the LinLog preamps
	// TODO: Fix the default value once calibration is seen. Then rely on the XML value
	this->LL_Branch_Energy_Split = config.attribute("LinLogSplit").as_int(10000); 

	// Expect uid style: DSSD:<DSSD number>:<strip number>:(preamp-gain)
	BETA_DSSDData_vec = std::vector<ProcessorStruct::DSSD>(this->numDSSDs, ProcessorStruct::DEFAULT_DSSD_STRUCT);
	ION_DSSDData_vec  = std::vector<ProcessorStruct::DSSD>(this->numDSSDs, ProcessorStruct::DEFAULT_DSSD_STRUCT);

	LL_maxEnEvt = std::vector<std::pair<PhysicsData*, PhysicsData*>>(this->numDSSDs, std::make_pair(nullptr, nullptr));
	LG_maxEnEvt = std::vector<PhysicsData*>(this->numDSSDs, nullptr);
	HG_maxEnEvt = std::vector<PhysicsData*>(this->numDSSDs, nullptr);

	LL_Mult = std::vector<int>(this->numDSSDs, 0);
	LG_Mult = std::vector<int>(this->numDSSDs, 0);
	HG_Mult = std::vector<int>(this->numDSSDs, 0);

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void DSSDProcessor::Finalize() {
	this->console->info("{} has been finalized", this->ProcessorName);
}

void DSSDProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) {
	std::string prefix = "DSSD_";
	for (int i = 0; i < this->numDSSDs; ++i) {

		// XML Histogram overrides need the `id="?"` of the node to match the `.at(?)` id not the root ID
		hismanager->RegisterPlot<TH2F>(prefix + std::to_string(i) + "00", "DSSD max energy per gain; Energy (keV); Gain (LL-B, LL-I, HG, LG)", this->h2dsettings.at(0));
		hismanager->RegisterPlot<TH2F>(prefix + std::to_string(i) + "01", "DSSD Beta Image (Highest Energy); X (Strip); Y (Strip)", this->h2dsettings.at(1));
		hismanager->RegisterPlot<TH2F>(prefix + std::to_string(i) + "02", "DSSD Ion Image (Highest Energy); X (Strip); Y (Strip)", this->h2dsettings.at(2));
		hismanager->RegisterPlot<TH2F>(prefix + std::to_string(i) + "10", "DSSD multiplicity; Multiplicity; Gain (LL,HG,LG)", this->h2dsettings.at(10));
		hismanager->RegisterPlot<TH2F>(prefix + std::to_string(i) + "11", "DSSD energy vs strip; Energy (keV); Strip (LL-B)", this->h2dsettings.at(11));
		hismanager->RegisterPlot<TH2F>(prefix + std::to_string(i) + "12", "DSSD energy vs strip; Energy (keV); Strip (LL-I)", this->h2dsettings.at(12));
		hismanager->RegisterPlot<TH2F>(prefix + std::to_string(i) + "13", "DSSD energy vs strip; Energy (keV); Strip (HG)", this->h2dsettings.at(13));
		hismanager->RegisterPlot<TH2F>(prefix + std::to_string(i) + "14", "DSSD energy vs strip; Energy (keV); Strip (LG)", this->h2dsettings.at(14));
	}

	this->console->info("Finished Declaring Plots");
}

void DSSDProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string, TTree*>& outputtrees) {
	this->OutputTree = new TTree("DSSD", "DSSD Processor Output");
	for (int i = 0; i < this->numDSSDs; ++i) {
		this->OutputTree->Branch(("DSSD_" + std::to_string(i)+ "_ION").c_str(), &(ION_DSSDData_vec.at(i)));
		this->OutputTree->Branch(("DSSD_" + std::to_string(i)+ "_BETA").c_str(), &(BETA_DSSDData_vec.at(i)));
	}
	outputtrees[this->ProcessorName] = this->OutputTree;
}

void DSSDProcessor::CleanupTree() {
	for (auto& it : ION_DSSDData_vec) {
		it = ProcessorStruct::DEFAULT_DSSD_STRUCT;
	}
	for (auto& it : BETA_DSSDData_vec) {
		it = ProcessorStruct::DEFAULT_DSSD_STRUCT;
	}
}

void DSSDProcessor::Reset() {

	for (int i = 0; i < this->numDSSDs; ++i) {
	LL_maxEnEvt[i] = std::make_pair(nullptr, nullptr);
	LG_maxEnEvt[i] = nullptr;
	HG_maxEnEvt[i] = nullptr;

	LL_Mult[i] = 0;
	LG_Mult[i] = 0;
	HG_Mult[i] = 0;
	}
	
}
