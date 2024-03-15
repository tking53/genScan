#include "MtasProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "PhysicsData.hpp"
#include "MtasStruct.hpp"
#include <TTree.h>

MtasProcessor::MtasProcessor(const std::string& log) : Processor(log,"MtasProcessor",{"mtas"}){
}

[[maybe_unused]] bool MtasProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();
	summary.GetDetectorSummary(this->AllDefaultRegex["mtas"],this->SummaryData);
	//this->console->info("Size of event for MTAS : {}",this->SummaryData.size());
	//for( const auto& evt : this->SummaryData ){
	//}
	
//	summary.GetDetectorSummary("mtas:center.*",this->SummaryData);
//	if( this->SummaryData.size() > 0 )
//		this->console->info("Num Center : {}",this->SummaryData.size());
//
//	summary.GetDetectorSummary("mtas:inner.*",this->SummaryData);
//	if( this->SummaryData.size() > 0 )
//		this->console->info("Num Inner : {}",this->SummaryData.size());
//
//	summary.GetDetectorSummary("mtas:middle.*",this->SummaryData);
//	if( this->SummaryData.size() > 0 )
//		this->console->info("Num Middle : {}",this->SummaryData.size());
//
//	summary.GetDetectorSummary("mtas:outer.*",this->SummaryData);
//	if( this->SummaryData.size() > 0 )
//		this->console->info("Num Outer : {}",this->SummaryData.size());

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool MtasProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool MtasProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

void MtasProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
}

void MtasProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
}

void MtasProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
}
		
void MtasProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void MtasProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	(void) hismanager;
	this->console->info("Finished Declaring Plots");
}

TTree* MtasProcessor::RegisterTree(){
	this->OutputTree = new TTree("Mtas","Mtas Processor output");
	this->OutputTree->Branch("segment_vec",&(this->SegmentDataVec));
	this->OutputTree->Branch("total_vec",&(this->TotalDataVec));
	return this->OutputTree;
}

void MtasProcessor::CleanupTree(){
	this->SegmentDataVec.clear();
	this->TotalDataVec.clear();
}
