#include "GenericProcessor.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"

GenericProcessor::GenericProcessor(const std::string& log) : Processor(log,"GenericProcessor",{"generic"}){
}

[[maybe_unused]] bool GenericProcessor::PreProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager){
	Processor::PreProcess();

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool GenericProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager){
	Processor::Process();

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool GenericProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager){
	Processor::PostProcess();

	Processor::EndProcess();
	return true;
}

void GenericProcessor::Init(const YAML::Node&){
	this->console->info("Init called with YAML::Node");
}

void GenericProcessor::Init(const Json::Value&){
	this->console->info("Init called with Json::Value");
}

void GenericProcessor::Init(const pugi::xml_node&){
	this->console->info("Init called with pugi::xml_node");
}

void GenericProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void GenericProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	(void) hismanager;
	this->console->info("Finished Declaring Plots");
}
