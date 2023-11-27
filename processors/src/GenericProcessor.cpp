#include "GenericProcessor.hpp"

GenericProcessor::GenericProcessor(const std::string& log) : Processor(log,"GenericProcessor",{"Generic"}){
}

bool GenericProcessor::PreProcess(){
	Processor::PreProcess();

	Processor::EndProcess();
	return true;
}

bool GenericProcessor::Process(){
	Processor::Process();

	Processor::EndProcess();
	return true;
}

bool GenericProcessor::PostProcess(){
	Processor::PostProcess();

	Processor::EndProcess();
	return true;
}

void GenericProcessor::Init(const YAML::Node&){
	console->info("Init called with YAML::Node");
}

void GenericProcessor::Init(const Json::Value&){
	console->info("Init called with Json::Value");
}

void GenericProcessor::Init(const pugi::xml_node&){
	console->info("Init called with pugi::xml_node");
}

void GenericProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	(void) hismanager;
	console->info("Finished Declaring Plots");
}
