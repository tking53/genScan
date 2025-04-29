#include "PidProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>

PidProcessor::PidProcessor(const std::string& log) : Processor(log,"PidProcessor",{"pid"}){
}

[[maybe_unused]] bool PidProcessor::PreProcess(EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();
	eventhistory->GetCurrentEventSummary()->GetDetectorSummary(this->AllDefaultRegex["pid"],this->SummaryData);

	for (const auto& evt: this->SummaryData){
		auto group = evt->GetGroup();
		auto subtype = evt->GetSubType();
		
		// Skip all events from VANDLE crate. 
		// This are all duplicated channels. Will think of a clever way to handle if they become needed.
		if (evt->GetCrate() == 0 ) {
			continue;
		}

		/*pid:ppac:0:db3:left*/

		if (subtype.compare("scint") == 0 ){
			this->currDETSUBTYPE = DETSUBTYPE::SCINT;
		} else if (subtype.compare("ppac") == 0 && group.compare("0") == 0 ){
			this->currDETSUBTYPE = DETSUBTYPE::PPAC0;
		} else if (subtype.compare("ppac") == 0 && group.compare("1") == 0 ){
			this->currDETSUBTYPE = DETSUBTYPE::PPAC1;
		} else if (subtype.compare("pin") == 0 ){
			this->currDETSUBTYPE = DETSUBTYPE::PIN;
		}
		
		if (evt->HasTag("db3")){
			this->currBOXID = BOXID::DB3;
		} else if (evt->HasTag("db4")){
			this->currBOXID = BOXID::DB4;
		} else if (evt->HasTag("db5")){
			this->currBOXID = BOXID::DB5;
		} else if (evt->HasTag("fp1")){
			this->currBOXID = BOXID::FP1;
		} else if (evt->HasTag("fp2")){
			this->currBOXID = BOXID::FP2;
		}
		
		if (evt->HasTag("up") ){
			this->currDETPOSITION = DETPOSITION::UP;
		} else if (evt->HasTag("down") ){
			this->currDETPOSITION = DETPOSITION::DOWN;
		} else if (evt->HasTag("left") ){
			this->currDETPOSITION = DETPOSITION::LEFT;
		} else if (evt->HasTag("right") ){
			this->currDETPOSITION = DETPOSITION::RIGHT;
		} else if (evt->HasTag("anode") ){
			this->currDETPOSITION = DETPOSITION::ANODE;
		}

		ProcessorStruct::PidDet* activDet = nullptr;

		if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::LEFT ) {
			activDet = &db3.scint.left;
		}else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::RIGHT ) {
			activDet = &db3.scint.right;
		}
		// Start DB3:PPAC0
		else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::ANODE ) {
			activDet = &db3.ppac0.anode;
		} else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::UP ) {
			activDet = &db3.ppac0.up;
		} else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::DOWN ) {
			activDet = &db3.ppac0.down;
		} else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::LEFT ) {
			activDet = &db3.ppac0.left;
		} else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::RIGHT ) {
			activDet = &db3.ppac0.right;
		}
		// Start DB3:PPAC1
		else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::ANODE ) {
			activDet = &db3.ppac1.anode;
		}else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::UP ) {
			activDet = &db3.ppac1.up;
		}else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::DOWN ) {
			activDet = &db3.ppac1.down;
		}else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::LEFT ) {
			activDet = &db3.ppac1.left;
		}else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::RIGHT ) {
			activDet = &db3.ppac1.right;
		}
			// Start DB4:PPAC0
		else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::ANODE ) {
			activDet = &db4.ppac0.anode;
		} else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::UP ) {
			activDet = &db4.ppac0.up;
		} else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::DOWN ) {
			activDet = &db4.ppac0.down;
		} else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::LEFT ) {
			activDet = &db4.ppac0.left;
		} else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::RIGHT ) {
			activDet = &db4.ppac0.right;
		}
		// Start DB4:PPAC1 (Generally unused in the FDSi but here for completeness if it ends up being needed at some point)
		else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::ANODE ) [[unlikely]] { 
			activDet = &db4.ppac1.anode;
		}else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::UP ) [[unlikely]] { 
			activDet = &db4.ppac1.up;
		}else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::DOWN ) [[unlikely]] { 
			activDet = &db4.ppac1.down;
		}else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::LEFT ) [[unlikely]] { 
			activDet = &db4.ppac1.left;
		}else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::RIGHT ) [[unlikely]] {
			activDet = &db4.ppac1.right;
		}
		// Start DB5:PPAC0 
		else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::ANODE ) { 
			activDet = &db5.ppac0.anode;
		}else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::UP ) { 
			activDet = &db5.ppac0.up;
		}else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::DOWN ) { 
			activDet = &db5.ppac0.down;
		}else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::LEFT ) { 
			activDet = &db5.ppac0.left;
		}else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::RIGHT ) {
			activDet = &db5.ppac0.right;
		}
		// Start DB5:PPAC1 
		else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::ANODE ) { 
			activDet = &db5.ppac1.anode;
		}else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::UP ) { 
			activDet = &db5.ppac1.up;
		}else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::DOWN ) { 
			activDet = &db5.ppac1.down;
		}else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::LEFT ) { 
			activDet = &db5.ppac1.left;
		}else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::RIGHT ) {
			activDet = &db5.ppac1.right;
		}
		// Start FP1 
		else if (this->currBOXID == BOXID::FP1 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::UP ) { 
			activDet = &fp1.xplas.at(0);
		}else if (this->currBOXID == BOXID::FP1 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::DOWN ) { 
			activDet = &fp1.xplas.at(1);
		}
		else if (this->currBOXID == BOXID::FP1 && this->currDETSUBTYPE == DETSUBTYPE::PIN && group.compare("1") == 0 ) { 
			activDet = &fp1.pin.at(0);
		}else if (this->currBOXID == BOXID::FP1 && this->currDETSUBTYPE == DETSUBTYPE::PIN && group.compare("2") == 0 ) { 
			activDet = &fp1.pin.at(1);
		}
		else if (this->currBOXID == BOXID::FP1 && this->currDETSUBTYPE == DETSUBTYPE::PIN && group.compare("3") == 0 ) { 
			activDet = &fp1.pin.at(2);
		}else if (this->currBOXID == BOXID::FP1 && this->currDETSUBTYPE == DETSUBTYPE::PIN && group.compare("4") == 0 ) { 
			activDet = &fp1.pin.at(3);
		}
		// Start FP2
		else if (this->currBOXID == BOXID::FP2 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::LEFT ) { 
			activDet = &fp2.xplas.at(0);
		}else if (this->currBOXID == BOXID::FP2 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::RIGHT ) { 
			activDet = &fp2.xplas.at(1);
		}
		else if (this->currBOXID == BOXID::FP2 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::UP ) { 
			activDet = &fp2.xplas.at(2);
		}else if (this->currBOXID == BOXID::FP2 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::DOWN ) { 
			activDet = &fp2.xplas.at(3);
		}
		else if (this->currBOXID == BOXID::FP2 && this->currDETSUBTYPE == DETSUBTYPE::PIN && group.compare("1") == 0 ) { 
			activDet = &fp2.pin.at(0);
		}else if (this->currBOXID == BOXID::FP2 && this->currDETSUBTYPE == DETSUBTYPE::PIN && group.compare("2") == 0 ) { 
			activDet = &fp2.pin.at(1);
		}
		else if (this->currBOXID == BOXID::FP2 && this->currDETSUBTYPE == DETSUBTYPE::PIN && group.compare("3") == 0 ) { 
			activDet = &fp2.pin.at(2);
		}else if (this->currBOXID == BOXID::FP2 && this->currDETSUBTYPE == DETSUBTYPE::PIN && group.compare("4") == 0 ) { 
			activDet = &fp2.pin.at(3);
		}
		else {
		continue;
		}

		activDet->energy = evt->GetEnergy();
		activDet->time = evt->GetCFDTimeStamp();
		activDet->saturation = evt->GetSaturation();
		activDet->pileup = evt->GetPileup();

	};

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool PidProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool PidProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

void PidProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
}
		
void PidProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void PidProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager){
	(void) hismanager;
	this->console->info("Finished Declaring Plots");
}

void PidProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
	this->OutputTree = new TTree("Pid","Pid Processor output");
	this->OutputTree->Branch("rf",&currRF);
	this->OutputTree->Branch("db3",&(db3));
	this->OutputTree->Branch("db4",&(db4));
	this->OutputTree->Branch("db5",&(db5));
	this->OutputTree->Branch("fp1",&(fp1));
	this->OutputTree->Branch("fp2",&(fp2));
	outputtrees[this->ProcessorName] = this->OutputTree;

}

void PidProcessor::CleanupTree(){
	this->currRF = -999;
	this->db3 = ProcessorStruct::DEFAULT_DBOX_STRUCT;
	this->db4 = ProcessorStruct::DEFAULT_DBOX_STRUCT;
	this->db5 = ProcessorStruct::DEFAULT_DBOX_STRUCT;
	this->fp1 = ProcessorStruct::DEFAULT_FP_STRUCT;
	this->fp2 = ProcessorStruct::DEFAULT_FP_STRUCT;
}

