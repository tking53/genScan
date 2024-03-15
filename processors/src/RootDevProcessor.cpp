#include "RootDevProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "PhysicsData.hpp"
#include "RootDevStruct.hpp"
#include <TTree.h>

RootDevProcessor::RootDevProcessor(const std::string& log) : Processor(log,"RootDevProcessor",{"RD"}){
}

[[maybe_unused]] bool RootDevProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();
	for( const auto& key : this->Types ){
		summary.GetDetectorSummary(this->AllDefaultRegex[key],this->SummaryData);
		//this->console->info("ROOTDEV Size for type {} : {}",key,this->SummaryData.size());
		for( const auto& evt : this->SummaryData ){
			this->CurrData.crateNum = evt->GetCrate();
			this->CurrData.modNum = evt->GetModule();
			this->CurrData.chanNum = evt->GetChannel();
			this->CurrData.rawEnergy = evt->GetRawEnergy();
			this->CurrData.energy = evt->GetEnergy();
			this->CurrData.timeSansCfd = evt->GetRawTimeStamp();
			this->CurrData.time = evt->GetTimeStamp();
			this->CurrData.cfdForcedBit = evt->GetCFDForcedBit();
			this->CurrData.cfdFraction = evt->GetCFDFraction();
			this->CurrData.detNum = evt->GetLocation();
			this->CurrData.type = evt->GetType();
			this->CurrData.subtype = evt->GetSubType();
			this->CurrData.group = evt->GetGroup();
			this->CurrData.tag = evt->GetTags();
			this->CurrData.pileup = evt->GetPileup();
			this->CurrData.saturation = evt->GetSaturation();
			this->CurrData.trace = std::vector<unsigned int>(evt->GetRawTraceData().begin(),evt->GetRawTraceData().end());
			this->CurrData.qdcSums = evt->GetQDCSums();

			this->DataVec.push_back(this->CurrData);
		}
	}
	this->CurrData = ProcessorStruct::DEFAULT_RD_STRUCT;
	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool RootDevProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool RootDevProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

void RootDevProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
	std::string additional_types = config["included_types"].as<std::string>("");
	this->InsertAdditionalTypes(additional_types);
}

void RootDevProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
	std::string additional_types = config.get("included_types","").asString();
	this->InsertAdditionalTypes(additional_types);
}

void RootDevProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
	std::string additional_types = config.attribute("included_types").as_string(""); 
	this->InsertAdditionalTypes(additional_types);
}
		
void RootDevProcessor::InsertAdditionalTypes(const std::string& typestring){
	std::set<std::string> typelist = {};
	std::regex word_regex("(\\w+)");
	auto words_begin =std::sregex_iterator(typestring.begin(),typestring.end(), word_regex);
	auto words_end = std::sregex_iterator();
	for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
		std::smatch match = *i;
		typelist.insert(match.str());
	}
	for( const auto& t : typelist ){
		this->AssociateType(t);
	}
}

void RootDevProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void RootDevProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	(void) hismanager;
	this->console->info("Finished Declaring Plots");
}

TTree* RootDevProcessor::RegisterTree(){
	this->OutputTree = new TTree("RootDev","RootDev Processor output");
	this->OutputTree->Branch("data_vec",&(this->DataVec));
	return this->OutputTree;
}

void RootDevProcessor::CleanupTree(){
	this->DataVec.clear();
}
