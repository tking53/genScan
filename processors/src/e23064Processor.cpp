#include "e23064Processor.hpp"

#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"

#include <TTree.h>

#include <stdexcept>
#include <string>

e23064Processor::e23064Processor(const std::string& log)
	: Processor(log, "e23064Processor", {}){
	this->DSSDProc = std::make_shared<DSSDProcessor>(log);
	this->PidProc = std::make_shared<PidProcessor>(log);
	this->CloverProc = std::make_shared<CloverProcessor>(log);
    this->VetoProc = std::make_shared<VetoProcessor>(log);

	for (const auto& type : this->DSSDProc->GetKnownTypes()) {
		this->AssociateType(type);
	}
	for (const auto& type : this->PidProc->GetKnownTypes()) {
		this->AssociateType(type);
	}
	for (const auto& type : this->CloverProc->GetKnownTypes()) {
		this->AssociateType(type);
	}
	for (const auto& type : this->VetoProc->GetKnownTypes()) {
		this->AssociateType(type);
	}


	this->h1dsettings = {
		{1000, {16000, 0.0, 160000}},
		{1001, {16384, 0.0, 65536.0}},
		{1002, {16384, 0.0, 65536.0}},
		{1100, {10000, 0.0, 10000.0}},
		{2000, {16384, 0.0, 65536.0}},
	};

	this->h2dsettings = {
		{1200, {4096, 0.0, 16384.0, 4096, 0.0, 16384.0}},
		{1300, {16000, 0.0, 160000, 4096, 0.0, 16384.0}},
		{1301, {16000, 0.0, 160000, 4096, 0.0, 16384.0}},
		{1302, {16000, 0.0, 160000, 4096, 0.0, 16384.0}},
		{1303, {16000, 0.0, 160000, 4096, 0.0, 16384.0}},
	};

    this->ConfigHasDSSD = false;
	this->ConfigHasPid = false;
	this->ConfigHasClover = false;
    this->ConfigHasVeto = false;

	this->EventHasDSSD = false;
	this->EventHasPid = false;
	this->EventHasClover = false;
	this->EventHasVeto = false;

    this->PIDPlot = -1;
	this->ImplantDSSDIndex = 0;
	this->DSSDThreshold = 0.0;
	this->PinThreshold = 0.0;
	this->RequireAllPins = true;

	this->Reset();
}

[[maybe_unused]] bool e23064Processor::PreProcess(EventHistoryManager* eventhistory, PLOTS::PlotRegistry* hismanager, CUTS::CutRegistry* cutmanager) {
	Processor::PreProcess();

	auto summary = eventhistory->GetCurrentEventSummary();
	auto types = summary->GetKnownTypes();

	this->EventHasDSSD = (types.find("dssd") != types.end());
	this->EventHasPid = (types.find("pid") != types.end());
	this->EventHasClover = (types.find("clover") != types.end());
    this->EventHasVeto = (types.find("veto") != types.end());

	if (this->EventHasDSSD) {
		this->DSSDProc->PreProcess(eventhistory, hismanager, cutmanager);
	}
	if (this->EventHasPid) {
		this->PidProc->PreProcess(eventhistory, hismanager, cutmanager);
	}
	if (this->EventHasClover) {
		this->CloverProc->PreProcess(eventhistory, hismanager, cutmanager);
	}
	if (this->EventHasVeto) {
		this->VetoProc->PreProcess(eventhistory, hismanager, cutmanager);
	}

	

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool e23064Processor::Process(EventHistoryManager* eventhistory, PLOTS::PlotRegistry* hismanager, CUTS::CutRegistry* cutmanager) {
	Processor::Process();
	if (this->EventHasDSSD) {
		this->DSSDProc->Process(eventhistory, hismanager, cutmanager);
	}
	if (this->EventHasPid) {
		this->PidProc->Process(eventhistory, hismanager, cutmanager);
	}
	if (this->EventHasClover) {
		this->CloverProc->Process(eventhistory, hismanager, cutmanager);
	}
	if (this->EventHasVeto) {
		this->VetoProc->Process(eventhistory, hismanager, cutmanager);
	}

    
    auto summary = eventhistory->GetCurrentEventSummary();
    double tke = 0.0;
	bool has_tke = false;
	if (this->EventHasDSSD && this->EventHasPid) {
		bool valid_pin_sum = false;
		const auto dssd_lg_total = summary->GetEventObservable("DSSD_" + std::to_string(this->ImplantDSSDIndex) + "_LG_TOTAL").value_or(0.0);
		const auto pin_sum = this->ComputePinEnergySum(valid_pin_sum);
		if (dssd_lg_total > this->DSSDThreshold && valid_pin_sum) {
			tke = dssd_lg_total + pin_sum;
			has_tke = true;
			summary->AddEventTag("ion_tke");
			summary->AddEventObservable("ION_TKE", tke);
			summary->AddEventObservable("ION_DSSD_IMPLANT_INDEX", static_cast<double>(this->ImplantDSSDIndex));
			summary->AddEventObservable("ION_DSSD_LG_SELECTED", dssd_lg_total);
			summary->AddEventObservable("ION_PIN_SUM", pin_sum);
			summary->AddEventObservable("ION_PIN_SOURCE", 1.0);
			hismanager->Fill("EXP_1000", tke);
			hismanager->Fill("EXP_1001", dssd_lg_total);
			hismanager->Fill("EXP_1002", pin_sum);
			hismanager->Fill("EXP_1200", dssd_lg_total, pin_sum);
			hismanager->Fill("EXP_1300", tke, this->PidProc->GetFP1PinEnergy(0));
			hismanager->Fill("EXP_1301", tke, this->PidProc->GetFP1PinEnergy(1));
			hismanager->Fill("EXP_1302", tke, this->PidProc->GetFP1PinEnergy(2));
			hismanager->Fill("EXP_1303", tke, this->PidProc->GetFP1PinEnergy(3));
			this->EXPData.tke = tke;
			this->EXPData.pin_sum = pin_sum;
		}
	}

	if (this->EventHasClover) {
		auto EXP_1000 = hismanager->GetPlot<TH1*>("EXP_1000");
		std::vector<PhysicsData*> clover_data;
		summary->GetDetectorSummary(this->AllDefaultRegex.at("clover"), clover_data);
		for (const auto* evt : clover_data) {
			if (evt->GetSaturation() || evt->GetPileup()) {
				continue;
			}
			const auto clover_energy = evt->GetEnergy();
			if (clover_energy <= 0.0) {
				continue;
			}

			EXP_1000->Fill(clover_energy);
			// if (has_tke && this->EventHasPid) {
			// 	for (size_t ii = 0; ii < this->IsotopeCuts.size(); ++ii) {
			// 		if (this->IsWithinPidGate(summary, this->IsotopeCuts[ii].second, cutmanager)) {
			// 			summary->AddEventTag(this->IsotopeCuts[ii].first);
			// 			hismanager->Fill("EXP_" + std::to_string(2000 + static_cast<int>(ii)), clover_energy);
			// 		}
			// 	}
			// }
		}
	}
	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool e23064Processor::PostProcess(EventHistoryManager* eventhistory, PLOTS::PlotRegistry* hismanager, CUTS::CutRegistry* cutmanager) {
	Processor::PostProcess();
	if (this->EventHasDSSD) {
		this->DSSDProc->PostProcess(eventhistory, hismanager, cutmanager);
	}
	if (this->EventHasPid) {
		this->PidProc->PostProcess(eventhistory, hismanager, cutmanager);
	}
	if (this->EventHasClover) {
		this->CloverProc->PostProcess(eventhistory, hismanager, cutmanager);
	}
	if (this->EventHasVeto) {
		this->VetoProc->PostProcess(eventhistory, hismanager, cutmanager);
	}   
	this->Reset();
	Processor::EndProcess();
	return true;
}

void e23064Processor::Finalize() {
	this->DSSDProc->Finalize();
	this->PidProc->Finalize();
	this->CloverProc->Finalize();
	this->console->info("{} has been finalized", this->ProcessorName);
}

void e23064Processor::Init(const pugi::xml_node& config) {
	this->IsotopeCuts.clear();
	this->IsotopeTags.clear();

	for (pugi::xml_node proc = config.child("Processor"); proc; proc = proc.next_sibling("Processor")) {
		std::string name = proc.attribute("name").as_string();
		if (name.compare("DSSDProcessor") == 0) {
			this->DSSDProc->Init(proc);
			this->ConfigHasDSSD = true;
		} else if (name.compare("PidProcessor") == 0) {
			this->PidProc->Init(proc);
			this->ConfigHasPid = true;
			this->PIDPlot = proc.attribute("PIDPlot").as_int(-1);
		} else if (name.compare("CloverProcessor") == 0) {
			this->CloverProc->Init(proc);
			this->ConfigHasClover = true;
        } else if (name.compare("VetoProcessor") == 0) {
            this->VetoProc->Init(proc);
            this->ConfigHasVeto = true;
        } else {
			throw std::runtime_error("unknown subprocessor declared in e23064Processor");
		}
	}

	if (!this->ConfigHasDSSD) {
		throw std::runtime_error("missing DSSDProcessor in e23064Processor");
	}
	if (!this->ConfigHasPid) {
		throw std::runtime_error("missing PidProcessor in e23064Processor");
	}
	if (!this->ConfigHasClover) {
		throw std::runtime_error("missing CloverProcessor in e23064Processor");
	}
    if (!this->ConfigHasVeto) {
        throw std::runtime_error("missing VetoProcessor in e23064Processor");
    }

	this->DSSDThreshold = config.attribute("DSSDThreshold").as_double(0.0);
	this->PinThreshold = config.attribute("PinThreshold").as_double(0.0);
	this->RequireAllPins = config.attribute("RequireAllPins").as_bool(true);
	this->ImplantDSSDIndex = config.attribute("ImplantDSSDIndex").as_int(0);

	for (pugi::xml_node isotope = config.child("Isotope"); isotope; isotope = isotope.next_sibling("Isotope")) {
		std::string tagname = isotope.attribute("name").as_string("");
		std::string cutid = isotope.attribute("cutid").as_string("");
		if (!tagname.empty() && !cutid.empty()) {
			this->IsotopeCuts.push_back({tagname, cutid});
			this->IsotopeTags.push_back(tagname);
		}
	}

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void e23064Processor::DeclarePlots(PLOTS::PlotRegistry* hismanager) {
	this->DSSDProc->DeclarePlots(hismanager);
	this->PidProc->DeclarePlots(hismanager);
	this->CloverProc->DeclarePlots(hismanager);
	this->VetoProc->DeclarePlots(hismanager);

	hismanager->RegisterPlot<TH1F>("EXP_1000", "E23064::Ion TKE; Energy (arb))", this->h1dsettings.at(1000));
	hismanager->RegisterPlot<TH1F>("EXP_1001", "E23064::DSSD low-gain sum; Energy (keV)", this->h1dsettings.at(1001));
	hismanager->RegisterPlot<TH1F>("EXP_1002", "E23064::4-pin sum; Energy (keV)", this->h1dsettings.at(1002));
	hismanager->RegisterPlot<TH1F>("EXP_1100", "E23064::Clover singles ungated; Energy (keV)", this->h1dsettings.at(1100));
	hismanager->RegisterPlot<TH2F>("EXP_1200", "E23064::DSSD LG vs pin sum; DSSD LG (keV); Pin sum (keV)", this->h2dsettings.at(1200));
	hismanager->RegisterPlot<TH2F>("EXP_1300", "E23064::TKE vs dE; TKE (keV); Pin 1 dE (keV)", this->h2dsettings.at(1300));
    hismanager->RegisterPlot<TH2F>("EXP_1301", "E23064::TKE vs dE; TKE (keV); Pin 2 dE (keV)", this->h2dsettings.at(1301));
    hismanager->RegisterPlot<TH2F>("EXP_1302", "E23064::TKE vs dE; TKE (keV); Pin 3 dE (keV)", this->h2dsettings.at(1302));
    hismanager->RegisterPlot<TH2F>("EXP_1303", "E23064::TKE vs dE; TKE (keV); Pin 4 dE (keV)", this->h2dsettings.at(1303));

	for (size_t ii = 0; ii < this->IsotopeTags.size(); ++ii) {
		const auto label = "EXP_" + std::to_string(2000 + static_cast<int>(ii));
		const auto title = "E23064 Clover Singles gated on " + this->IsotopeTags[ii] + "; Energy (keV)";
		hismanager->RegisterPlot<TH1F>(label, title, this->h1dsettings.at(2000));
	}

	this->console->info("Finished Declaring Plots");
}

void e23064Processor::RegisterTree(std::unordered_map<std::string, TTree*>& outputtrees) {
	this->OutputTree = new TTree("e23064", "e23064 Processor output");
	this->OutputTree->Branch("exp", &(this->EXPData));
	outputtrees[this->ProcessorName] = this->OutputTree;

	this->DSSDProc->RegisterTree(outputtrees);
	this->PidProc->RegisterTree(outputtrees);
	this->CloverProc->RegisterTree(outputtrees);
	this->VetoProc->RegisterTree(outputtrees);
}

void e23064Processor::CleanupTree() {
	this->DSSDProc->CleanupTree();
	this->PidProc->CleanupTree();
	this->CloverProc->CleanupTree();
	this->VetoProc->CleanupTree();

	this->EXPData = ProcessorStruct::DEFAULT_EXP_STRUCT;
}

void e23064Processor::RegisterCuts(CUTS::CutRegistry* cutmanager) {
	this->DSSDProc->RegisterCuts(cutmanager);
	this->PidProc->RegisterCuts(cutmanager);
	this->CloverProc->RegisterCuts(cutmanager);
	this->VetoProc->RegisterCuts(cutmanager);
}

void e23064Processor::Reset() {
	this->EventHasDSSD = false;
	this->EventHasPid = false;
	this->EventHasClover = false;
	this->EventHasVeto = false;
}

bool e23064Processor::IsWithinPidGate(const EventSummary* summary, const std::string& cutid, CUTS::CutRegistry* cutmanager) const {
	if (summary == nullptr || cutmanager == nullptr) {
		return false;
	}

	const auto tke = summary->GetEventObservable("ION_TKE");
	if (!tke.has_value()) {
		return false;
	}

	return cutmanager->IsWithin(cutid, this->PidProc->GetFP1Tof(6), tke.value());
}



double e23064Processor::ComputePinEnergySum(bool& valid) const {
	valid = false;

	double total = 0.0;
	for (size_t ii = 0; ii < this->PidProc->GetNumFP1Pins(); ++ii) {
		double pin_energy = this->PidProc->GetFP1PinEnergy(ii);
		if (this->RequireAllPins && pin_energy < this->PinThreshold) {
			valid = false;
		}
		if (pin_energy >= this->PinThreshold) {
			total += pin_energy;
		}
	}

	if (this->RequireAllPins) {
		valid = true;
	} else {
        valid = false;
    }
	return total;
}
