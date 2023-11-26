#include "GenericAnalyzer.hpp"
#include "Analyzer.hpp"

GenericAnalyzer::GenericAnalyzer(const std::string& log) : Analyzer(log,"GenericAnalyzer"){
}

bool GenericAnalyzer::PreProcess(){
	Analyzer::PreProcess();
	Analyzer::EndProcess();
	return true;
}

bool GenericAnalyzer::Process(){
	Analyzer::Process();
	Analyzer::EndProcess();
	return true;
}

bool GenericAnalyzer::PostProcess(){
	Analyzer::PostProcess();
	Analyzer::EndProcess();
	return true;
}

void GenericAnalyzer::Init(const YAML::Node&){
	console->info("Init called with YAML::Node");
}

void GenericAnalyzer::Init(const Json::Value&){
	console->info("Init called with Json::Value");
}

void GenericAnalyzer::Init(const pugi::xml_node&){
	console->info("Init called with pugi::xml_node");
}

void GenericAnalyzer::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	(void) hismanager;
	console->info("Finished Declaring Plots");
}
