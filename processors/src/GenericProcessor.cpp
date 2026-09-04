#include "GenericProcessor.hpp"

GenericProcessor::GenericProcessor(const std::string& log)
	: Processor(log, "GenericProcessor", {"generic"}) {
}

[[maybe_unused]] bool GenericProcessor::PreProcess([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	Processor::PreProcess();

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool GenericProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	Processor::Process();

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool GenericProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	Processor::PostProcess();

	Processor::EndProcess();
	return true;
}

void GenericProcessor::Init(const pugi::xml_node& config) {
	this->console->info("Init called with pugi::xml_node");
	this->LoadHistogramSettings(config);
}

void GenericProcessor::Finalize() {
	this->console->info("{} has been finalized", this->ProcessorName);
}

void GenericProcessor::DeclarePlots([[maybe_unused]] PLOTS::PlotRegistry* hismanager) {
	this->console->info("Finished Declaring Plots");
}
