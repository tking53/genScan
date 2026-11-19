#include "TimingAnalyzer.hpp"
#include <cmath>
#include <utility>
#include <stdexcept>
#include <iterator>

TimingAnalyzer::TimingAnalyzer(const std::string& log)
	: Analyzer(log, "TimingAnalyzer", {}) {

	// this->h2dsettings = {
	// 	{1000, {1024, 0, 1024.0, 512, 0, 512.0}},
	// 	{1001, {16384, 0, 16384.0, 512, 0, 512.0}},
	// 	{1002, {1024, 0, 1024.0, 512, 0, 512.0}},
	// 	{1010, {1024, 0, 1024.0, 512, 0, 512.0}},
	// 	{1011, {16384, 0, 16384.0, 512, 0, 512.0}},
	// 	{1012, {1024, 0, 1024.0, 512, 0, 512.0}},
	// 	{1020, {1024, 0, 1024.0, 512, 0, 512.0}},
	// 	{1021, {16384, 0, 16384.0, 512, 0, 512.0}},
	// 	{1022, {16384, 0, 16384.0, 512, 0, 512.0}},
	// 	{1030, {16384, 0.0, 16384.0, 512, 0, 512.0}},
	// 	{1031, {16384, 0.0, 16384.0, 512, 0, 512.0}},
	// 	{1032, {16384, 0.0, 16384.0, 512, 0, 512.0}},
	// 	{1033, {16384, -16.0, 16.0, 512, 0, 512.0}},
	// 	{1034, {16384, -16.0, 16.0, 512, 0, 512.0}},
	// 	{1035, {16384, -16.0, 16.0, 512, 0, 512.0}},
	// };
	
	this->BadFilter = 0;
}

TimingAnalyzer::~TimingAnalyzer() {
	this->console->info("Events that failed simple check : {}",this->BadFilter);
}

[[maybe_unused]] bool TimingAnalyzer::PreProcess([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	Analyzer::PreProcess();

	for (const auto& key : this->Types) {
		eventhistory->GetCurrentEventSummary()->GetDetectorSummary(this->AllDefaultRegex[key], this->SummaryData);
		// this->console->info("ROOTDEV Size for type {} : {}",key,this->SummaryData.size());
		for (auto& evt : this->SummaryData) {
			for (auto& s : this->Settings) {
				boost::smatch cmapmatch;
				if (boost::regex_match(evt->GetCMapID(), cmapmatch, std::get<0>(s), boost::regex_constants::match_continuous)) {
					auto trace = evt->GetRawTrace();
					auto ftrace = std::vector<float>(trace.begin(), trace.end());
					auto filter = std::get<1>(s);
					auto erg = filter.RunFilter(ftrace);
					auto trap = filter.trap;
					const auto [min_it, max_it] = std::minmax_element(trap.begin(),trap.end());
					// this->console->info("{} {}",std::distance(trap.begin(),min_it),std::distance(trap.begin(),max_it));
					auto left = std::get<2>(s);
					auto right = std::get<3>(s);
					auto min_idx = std::distance(trap.begin(),min_it);
					auto max_idx = std::distance(trap.begin(),max_it);
					if (max_idx > min_idx and (min_idx > left and min_idx < right) and (max_idx > left and max_idx < right)) {
						auto mean_idx = 0.0;
						auto weight = 0.0;
						// calculate the average distance via weighting
						// I think this is going to be symmetric across zer
						for (auto it = min_it; it != max_it; ++it) {
							mean_idx += std::abs(*it) * std::distance(trap.begin(), it);
							weight += std::abs(*it);
						}
						mean_idx /= weight;
						// let's find the difference between the mean_idx and rounding to the nearest idx 
						// This should give us a variation between positive and negative timestamp shifts 
						evt->AdjustTimeStamp(mean_idx - std::round(mean_idx));
					} else {
						++(this->BadFilter);
					}
				}
			}
			// this->console->info("{} {}",evt->GetType(),this->SummaryData.size());
		}
	}
	Analyzer::EndProcess();
	return true;
}

[[maybe_unused]] bool TimingAnalyzer::Process([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	return true;
}

[[maybe_unused]] bool TimingAnalyzer::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	return true;
}

void TimingAnalyzer::Init(const pugi::xml_node& config) {
	console->info("Init called with pugi::xml_node");

	std::string additional_types = config.attribute("included_types").as_string("");
	if (additional_types.empty()) {
		this->console->error("Missing included_types tag in xml tag for TimingAnalyzer");
		throw "Missing included_types tag in xml tag for TimingAnalyzer";
	} else {
		this->InsertAdditionalTypes(additional_types);
	}

	auto check_node_attribute = [](const auto& settings, const auto& name) {
		if (settings.attribute(name).empty()) {
			throw std::string("Missing ") + name + " Attribute in TimingAnalyzer Settings";
		}
	};

	for (pugi::xml_node settings = config.child("settings"); settings; settings = settings.next_sibling("settings")) {
		auto re = this->GenerateRegex(settings.attribute("Crate").as_string("[\\d]"), settings.attribute("Module").as_string("[\\d]"), settings.attribute("Channel").as_string("[\\d]"));
		this->console->info("Found settings node regex:{}", re.str());

		check_node_attribute(settings, "length");
		check_node_attribute(settings, "gap");
		check_node_attribute(settings, "baseline");
		check_node_attribute(settings, "tau");

		check_node_attribute(settings, "min");
		check_node_attribute(settings, "max");

		auto ll = settings.attribute("length").as_uint();
		auto gg = settings.attribute("gap").as_uint();
		auto bb = settings.attribute("baseline").as_uint();
		auto tt = settings.attribute("tau").as_float();
		auto left = settings.attribute("min").as_uint();
		auto right = settings.attribute("max").as_uint();
		try {
			this->Settings.push_back(std::make_tuple(re, TrapezoidFilter<float, float>(ll, gg, bb, tt), left, right));
		} catch (std::runtime_error& e) {
			this->console->error("error parsing waveform settings for r:{} {}", re.str(), e.what());
			throw e;
		}
	}

	if (this->Settings.empty()) {
		this->console->error("Missing settings node");
		throw "Missing settings node";
	}

	this->LoadHistogramSettings(config);
}

void TimingAnalyzer::Finalize() {
	this->console->info("{} has been finalized", this->AnalyzerName);
}

void TimingAnalyzer::DeclarePlots(PLOTS::PlotRegistry* hismanager) const {
	console->info("Finished Declaring Plots");
}

void TimingAnalyzer::InsertAdditionalTypes(const std::string& typestring) {
	std::set<std::string> typelist = {};
	std::regex word_regex("(\\w+)");
	auto words_begin = std::sregex_iterator(typestring.begin(), typestring.end(), word_regex);
	auto words_end = std::sregex_iterator();
	for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
		std::smatch match = *i;
		typelist.insert(match.str());
	}
	for (const auto& t : typelist) {
		this->AssociateType(t);
	}
}

boost::regex TimingAnalyzer::GenerateRegex(const std::string& cratestr, const std::string& modstr, const std::string& channelstr) {
	auto re = Analyzer::GenerateRegex(cratestr, modstr, channelstr);
	auto result = this->KnownSettings.insert(re);
	if (result.first == this->KnownSettings.end()) {
		this->console->error("duplicate settings tag found for Crate=\"{}\" Module=\"{}\" Channel=\"{}\"", cratestr, modstr, channelstr);
		throw "duplicate settings found in TimingAnalyzer settings tag";
	}
	return re;
}
