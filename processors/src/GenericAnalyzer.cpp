#include "GenericAnalyzer.hpp"
#include "Analyzer.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"

GenericAnalyzer::GenericAnalyzer(const std::string& log) : Analyzer(log,"GenericAnalyzer",{"generic"}){
}

[[maybe_unused]] bool GenericAnalyzer::PreProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Analyzer::PreProcess();
	Analyzer::EndProcess();
	return true;
}

[[maybe_unused]] bool GenericAnalyzer::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Analyzer::Process();
	Analyzer::EndProcess();
	return true;
}

[[maybe_unused]] bool GenericAnalyzer::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
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

void GenericAnalyzer::Finalize(){
	this->console->info("{} has been finalized",this->AnalyzerName);
}


void GenericAnalyzer::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	(void) hismanager;
	console->info("Finished Declaring Plots");
}
