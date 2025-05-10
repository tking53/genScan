#include "PidProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <limits>
#include <stdexcept>

PidProcessor::PidProcessor(const std::string& log) : Processor(log,"PidProcessor",{"pid"}){
	
	this->h1dsettings = {
		{ 1, { 10000,-1000,1000} },
		{ 2, { 10000,-1000,1000} },
		{ 3, { 10000,-1000,1000} },
		{ 4, { 10000,-1000,1000} },
		{ 5, { 10000,-1000,1000} },
		{ 6, { 10000,-1000,1000} },
		{ 7, { 10000,-1000,1000} },
		{ 8, { 10000,-1000,1000} },

		// Start FP2 based plots
		{ 101, { 10000,-1000,1000} },
		{ 102, { 10000,-1000,1000} },
		{ 103, { 10000,-1000,1000} },
		{ 104, { 10000,-1000,1000} },
		{ 105, { 10000,-1000,1000} },
		{ 106, { 10000,-1000,1000} },
		{ 107, { 10000,-1000,1000} },
		{ 108, { 10000,-1000,1000} },

		//DB3 PPAC0
		{300, {2000,-1000,1000}},
		{301, {2000,-1000,1000}},
		
		//DB3 PPAC1
		{310, {2000,-1000,1000}},
		{311, {2000,-1000,1000}},
		
		//DB4 PPAC0
		{400, {2000,-1000,1000}},
		{401, {2000,-1000,1000}},
		
		//DB4 PPAC1
		{410, {2000,-1000,1000}},
		{411, {2000,-1000,1000}},
		
		//DB5 PPAC0
		{500, {2000,-1000,1000}},
		{501, {2000,-1000,1000}},
		
		//DB5 PPAC1
		{510, {2000,-1000,1000}},
		{511, {2000,-1000,1000}}
	};

	this->h2dsettings = {
		{  9, { 2000,-1000,1000,2000,0,16000} },
		{ 10, { 2000,-1000,1000,2000,0,16000} },
		{ 11, { 2000,-1000,1000,2000,0,16000} },
		{ 12, { 2000,-1000,1000,2000,0,16000} },
		{ 13, { 2000,-1000,1000,2000,0,16000} },
		{ 14, { 2000,-1000,1000,2000,0,16000} },
		{ 15, { 2000,-1000,1000,2000,0,16000} },
		{ 16, { 2000,-1000,1000,2000,0,16000} },
		{ 17, { 2000,-1000,1000,2000,0,16000} },
		{ 18, { 2000,-1000,1000,2000,0,16000} },
		{ 19, { 2000,-1000,1000,2000,0,16000} },
		{ 20, { 2000,-1000,1000,2000,0,16000} },
		{ 21, { 2000,-1000,1000,2000,0,16000} },
		{ 22, { 2000,-1000,1000,2000,0,16000} },
		{ 23, { 2000,-1000,1000,2000,0,16000} },
		{ 24, { 2000,-1000,1000,2000,0,16000} },

		{ 25, { 2000,0,16000,2000,0,16000} },
		{ 26, { 2000,0,16000,2000,0,16000} },
		{ 27, { 2000,0,16000,2000,0,16000} },

		// Start FP2 based plots
		{ 109, { 2000,-1000,1000,2000,0,16000} },
		{ 110, { 2000,-1000,1000,2000,0,16000} },
		{ 111, { 2000,-1000,1000,2000,0,16000} },
		{ 112, { 2000,-1000,1000,2000,0,16000} },
		{ 113, { 2000,-1000,1000,2000,0,16000} },
		{ 114, { 2000,-1000,1000,2000,0,16000} },
		{ 115, { 2000,-1000,1000,2000,0,16000} },
		{ 116, { 2000,-1000,1000,2000,0,16000} },
		{ 117, { 2000,-1000,1000,2000,0,16000} },
		{ 118, { 2000,-1000,1000,2000,0,16000} },
		{ 119, { 2000,-1000,1000,2000,0,16000} },
		{ 120, { 2000,-1000,1000,2000,0,16000} },
		{ 121, { 2000,-1000,1000,2000,0,16000} },
		{ 122, { 2000,-1000,1000,2000,0,16000} },
		{ 123, { 2000,-1000,1000,2000,0,16000} },
		{ 124, { 2000,-1000,1000,2000,0,16000} },

		{ 125, { 2000,0,16000,2000,0,16000} },
		{ 126, { 2000,0,16000,2000,0,16000} },
		{ 127, { 2000,0,16000,2000,0,16000} },

		//DB3 PPAC0 Image
		{3000, {2000,-1000,1000,2000,-1000,1000}},
		
		//DB3 PPAC1 Image
		{3010, {2000,-1000,1000,2000,-1000,1000}},

		//DB4 PPAC0 Image
		{4000, {2000,-1000,1000,2000,-1000,1000}},

		//DB3 PPAC1 Image
		{4010, {2000,-1000,1000,2000,-1000,1000}},

		//DB5 PPAC0 Image
		{5000, {2000,-1000,1000,2000,-1000,1000}},
		
		//DB3 PPAC1 Image
		{5010, {2000,-1000,1000,2000,-1000,1000}}

	};

	this->fp1Tofs = std::vector<double>(10,std::numeric_limits<double>::max());
	this->fp2Tofs = std::vector<double>(10,std::numeric_limits<double>::max());

}

[[maybe_unused]] bool PidProcessor::PreProcess(EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();
	auto summary = eventhistory->GetCurrentEventSummary();
	summary->GetDetectorSummary(this->AllDefaultRegex["pid"],this->SummaryData);

	for (const auto& evt: this->SummaryData){
		auto group = evt->GetGroup();
		auto subtype = evt->GetSubType();
		
		// Skip all events from VANDLE crate. 
		// This are all duplicated channels. Will think of a clever way to handle if they become needed.
		if (evt->GetCrate() == 0 and not evt->HasTag("tas") ) {
			continue;
		}

		/*pid:ppac:0:db3:left*/

		if (subtype.compare("scint") == 0 ){
			this->currDETSUBTYPE = DETSUBTYPE::SCINT;
		} else if (subtype.compare("ppac") == 0 && group.compare("0") == 0 ){
			this->currDETSUBTYPE = DETSUBTYPE::PPAC0;
		} else if (subtype.compare("ppac") == 0 && group.compare("1") == 0 ){
			this->currDETSUBTYPE = DETSUBTYPE::PPAC1;
		} else if (subtype.compare("pin") == 0  && group.compare("1") == 0 ){
			this->currDETSUBTYPE = DETSUBTYPE::PIN1;
		} else if (subtype.compare("pin") == 0  && group.compare("2") == 0 ){
			this->currDETSUBTYPE = DETSUBTYPE::PIN2;
		} else if (subtype.compare("pin") == 0  && group.compare("3") == 0 ){
			this->currDETSUBTYPE = DETSUBTYPE::PIN3;
		} else if (subtype.compare("pin") == 0  && group.compare("4") == 0 ){
			this->currDETSUBTYPE = DETSUBTYPE::PIN4;
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

		if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::LEFT ) {
			FillStruct(evt, db3.scint.left);
		}else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::RIGHT ) {
			FillStruct(evt,db3.scint.right);
		}
		// Start DB3:PPAC0
		else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::ANODE ) {
			FillStruct(evt,db3.ppac0.anode);
		} else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::UP ) {
			FillStruct(evt,db3.ppac0.up);
		} else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::DOWN ) {
			FillStruct(evt,db3.ppac0.down);
		} else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::LEFT ) {
			FillStruct(evt,db3.ppac0.left);
		} else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::RIGHT ) {
			FillStruct(evt,db3.ppac0.right);
		}
		// Start DB3:PPAC1
		else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::ANODE ) {
			FillStruct(evt,db3.ppac1.anode);
		}else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::UP ) {
			FillStruct(evt,db3.ppac1.up);
		}else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::DOWN ) {
			FillStruct(evt,db3.ppac1.down);
		}else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::LEFT ) {
			FillStruct(evt,db3.ppac1.left);
		}else if (this->currBOXID == BOXID::DB3 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::RIGHT ) {
			FillStruct(evt,db3.ppac1.right);
		}
			// Start DB4:PPAC0
		else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::ANODE ) {
			FillStruct(evt,db4.ppac0.anode);
		} else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::UP ) {
			FillStruct(evt,db4.ppac0.up);
		} else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::DOWN ) {
			FillStruct(evt,db4.ppac0.down);
		} else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::LEFT ) {
			FillStruct(evt,db4.ppac0.left);
		} else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::RIGHT ) {
			FillStruct(evt,db4.ppac0.right);
		}
		// Start DB4:PPAC1 (Generally unused in the FDSi but here for completeness if it ends up being needed at some point)
		else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::ANODE ) [[unlikely]] { 
			FillStruct(evt,db4.ppac1.anode);
		}else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::UP ) [[unlikely]] { 
			FillStruct(evt,db4.ppac1.up);
		}else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::DOWN ) [[unlikely]] { 
			FillStruct(evt,db4.ppac1.down);
		}else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::LEFT ) [[unlikely]] { 
			FillStruct(evt,db4.ppac1.left);
		}else if (this->currBOXID == BOXID::DB4 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::RIGHT ) [[unlikely]] {
			FillStruct(evt,db4.ppac1.right);
		}
		// Start DB5:PPAC0 
		else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::ANODE ) { 
			FillStruct(evt,db5.ppac0.anode);
		}else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::UP ) { 
			FillStruct(evt,db5.ppac0.up);
		}else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::DOWN ) { 
			FillStruct(evt,db5.ppac0.down);
		}else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::LEFT ) { 
			FillStruct(evt,db5.ppac0.left);
		}else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC0 && this->currDETPOSITION == DETPOSITION::RIGHT ) {
			FillStruct(evt,db5.ppac0.right);
		}
		// Start DB5:PPAC1 
		else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::ANODE ) { 
			FillStruct(evt,db5.ppac1.anode);
		}else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::UP ) { 
			FillStruct(evt,db5.ppac1.up);
		}else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::DOWN ) { 
			FillStruct(evt,db5.ppac1.down);
		}else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::LEFT ) { 
			FillStruct(evt,db5.ppac1.left);
		}else if (this->currBOXID == BOXID::DB5 && this->currDETSUBTYPE == DETSUBTYPE::PPAC1 && this->currDETPOSITION == DETPOSITION::RIGHT ) {
			FillStruct(evt,db5.ppac1.right);
		}
		// Start FP1 
		else if (this->currBOXID == BOXID::FP1 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::LEFT ) { 
			FillStruct(evt,fp1.xplas[0]);
		}else if (this->currBOXID == BOXID::FP1 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::RIGHT ) { 
			FillStruct(evt,fp1.xplas[1]);
		}
		else if (this->currBOXID == BOXID::FP1 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::UP ) { 
			FillStruct(evt,fp1.xplas[2]);
		}else if (this->currBOXID == BOXID::FP1 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::DOWN ) { 
			FillStruct(evt,fp1.xplas[3]);
		}
		else if (this->currBOXID == BOXID::FP1 && this->currDETSUBTYPE == DETSUBTYPE::PIN1 ) { 
			FillStruct(evt,fp1.pin[0]);
		}else if (this->currBOXID == BOXID::FP1 && this->currDETSUBTYPE == DETSUBTYPE::PIN2 ) { 
			FillStruct(evt,fp1.pin[1]);
		}
		else if (this->currBOXID == BOXID::FP1 && this->currDETSUBTYPE == DETSUBTYPE::PIN3 ) { 
			FillStruct(evt,fp1.pin[2]);
		}else if (this->currBOXID == BOXID::FP1 && this->currDETSUBTYPE == DETSUBTYPE::PIN4 ) { 
			FillStruct(evt,fp1.pin[3]);
		}
		// Start FP2
		else if (this->currBOXID == BOXID::FP2 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::LEFT ) { 
			FillStruct(evt,fp2.xplas[0]);
		}else if (this->currBOXID == BOXID::FP2 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::RIGHT ) { 
			FillStruct(evt,fp2.xplas[1]);
		}
		else if (this->currBOXID == BOXID::FP2 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::UP ) { 
			FillStruct(evt,fp2.xplas[2]);
		}else if (this->currBOXID == BOXID::FP2 && this->currDETSUBTYPE == DETSUBTYPE::SCINT && this->currDETPOSITION == DETPOSITION::DOWN ) { 
			FillStruct(evt,fp2.xplas[3]);
		}
		else if (this->currBOXID == BOXID::FP2 && this->currDETSUBTYPE == DETSUBTYPE::PIN1 ) { 
			FillStruct(evt,fp2.pin[0]);
		}else if (this->currBOXID == BOXID::FP2 && this->currDETSUBTYPE == DETSUBTYPE::PIN2 ) { 
			FillStruct(evt,fp2.pin[1]);
		}
		else if (this->currBOXID == BOXID::FP2 && this->currDETSUBTYPE == DETSUBTYPE::PIN3 ) { 
			FillStruct(evt,fp2.pin[2]);
		}else if (this->currBOXID == BOXID::FP2 && this->currDETSUBTYPE == DETSUBTYPE::PIN4 ) { 
			FillStruct(evt,fp2.pin[3]);
		}
		else {
		continue;
		}
	};

	fp1Tofs[0] = db3.ppac0.anode.time - fp1.xplas.at(0).time ;
	fp1Tofs[1] = db3.ppac0.anode.time - fp1.xplas.at(1).time ;
	fp1Tofs[2] = db3.ppac1.anode.time - fp1.xplas.at(0).time ;
	fp1Tofs[3] = db3.ppac1.anode.time - fp1.xplas.at(1).time ;
	fp1Tofs[4] = db3.scint.left.time  - fp1.xplas.at(0).time ;
	fp1Tofs[5] = db3.scint.left.time  - fp1.xplas.at(1).time ;
	fp1Tofs[6] = db3.scint.right.time - fp1.xplas.at(0).time ;
	fp1Tofs[7] = db3.scint.right.time - fp1.xplas.at(1).time ;

	db3.ppac0.xpos = this->CalcPPACPosition(db3.ppac0.left.time,db3.ppac0.right.time);
	db3.ppac0.ypos = this->CalcPPACPosition(db3.ppac0.up.time  ,db3.ppac0.down.time);

	db3.ppac1.xpos = this->CalcPPACPosition(db3.ppac1.left.time,db3.ppac1.right.time);
	db3.ppac1.ypos = this->CalcPPACPosition(db3.ppac1.up.time  ,db3.ppac1.down.time);

	db4.ppac0.xpos = this->CalcPPACPosition(db4.ppac0.left.time,db4.ppac0.right.time);
	db4.ppac0.ypos = this->CalcPPACPosition(db4.ppac0.up.time  ,db4.ppac0.down.time);

	db4.ppac1.xpos = this->CalcPPACPosition(db4.ppac1.left.time,db4.ppac1.right.time);
	db4.ppac1.ypos = this->CalcPPACPosition(db4.ppac1.up.time  ,db4.ppac1.down.time);

	db5.ppac0.xpos = this->CalcPPACPosition(db5.ppac0.left.time,db5.ppac0.right.time);
	db5.ppac0.ypos = this->CalcPPACPosition(db5.ppac0.up.time  ,db5.ppac0.down.time);

	db5.ppac1.xpos = this->CalcPPACPosition(db5.ppac1.left.time,db5.ppac1.right.time);
	db5.ppac1.ypos = this->CalcPPACPosition(db5.ppac1.up.time  ,db5.ppac1.down.time);

	fp2Tofs[0] = db3.ppac0.anode.time - fp2.xplas.at(0).time ;
	fp2Tofs[1] = db3.ppac0.anode.time - fp2.xplas.at(1).time ;
	fp2Tofs[2] = db3.ppac1.anode.time - fp2.xplas.at(0).time ;
	fp2Tofs[3] = db3.ppac1.anode.time - fp2.xplas.at(1).time ;
	fp2Tofs[4] = db3.scint.left.time  - fp2.xplas.at(0).time ;
	fp2Tofs[5] = db3.scint.left.time  - fp2.xplas.at(1).time ;
	fp2Tofs[6] = db3.scint.right.time - fp2.xplas.at(0).time ;
	fp2Tofs[7] = db3.scint.right.time - fp2.xplas.at(1).time ;

	hismanager->Fill("PID_1", fp1Tofs.at(0));
	hismanager->Fill("PID_2", fp1Tofs.at(1));
	hismanager->Fill("PID_3", fp1Tofs.at(2));
	hismanager->Fill("PID_4", fp1Tofs.at(3));
	hismanager->Fill("PID_5", fp1Tofs.at(4));
	hismanager->Fill("PID_6", fp1Tofs.at(5));
	hismanager->Fill("PID_7", fp1Tofs.at(6));
	hismanager->Fill("PID_8", fp1Tofs.at(7));

	hismanager->Fill("PID_9",  fp1Tofs[0], fp1.pin.at(0).energy);
	hismanager->Fill("PID_10", fp1Tofs[0], fp1.pin.at(1).energy);
	hismanager->Fill("PID_11", fp1Tofs[0], fp1.pin.at(2).energy);
	hismanager->Fill("PID_12", fp1Tofs[0], fp1.pin.at(3).energy);

	hismanager->Fill("PID_13", fp1Tofs[2], fp1.pin.at(0).energy);
	hismanager->Fill("PID_14", fp1Tofs[2], fp1.pin.at(1).energy);
	hismanager->Fill("PID_15", fp1Tofs[2], fp1.pin.at(2).energy);
	hismanager->Fill("PID_16", fp1Tofs[2], fp1.pin.at(3).energy);

	hismanager->Fill("PID_17", fp1Tofs[4], fp1.pin.at(0).energy);
	hismanager->Fill("PID_18", fp1Tofs[4], fp1.pin.at(1).energy);
	hismanager->Fill("PID_19", fp1Tofs[4], fp1.pin.at(2).energy);
	hismanager->Fill("PID_20", fp1Tofs[4], fp1.pin.at(3).energy);

	hismanager->Fill("PID_21", fp1Tofs[6], fp1.pin.at(0).energy);
	hismanager->Fill("PID_22", fp1Tofs[6], fp1.pin.at(1).energy);
	hismanager->Fill("PID_23", fp1Tofs[6], fp1.pin.at(2).energy);
	hismanager->Fill("PID_24", fp1Tofs[6], fp1.pin.at(3).energy);

	hismanager->Fill("PID_25",fp1.pin.at(0).energy,fp1.pin.at(1).energy);
	hismanager->Fill("PID_26",fp1.pin.at(0).energy,fp1.pin.at(2).energy);
	hismanager->Fill("PID_27",fp1.pin.at(0).energy,fp1.pin.at(3).energy);


	hismanager->Fill("PID_101", fp2Tofs.at(0));
	hismanager->Fill("PID_102", fp2Tofs.at(1));
	hismanager->Fill("PID_103", fp2Tofs.at(2));
	hismanager->Fill("PID_104", fp2Tofs.at(3));
	hismanager->Fill("PID_105", fp2Tofs.at(4));
	hismanager->Fill("PID_106", fp2Tofs.at(5));
	hismanager->Fill("PID_107", fp2Tofs.at(6));
	hismanager->Fill("PID_108", fp2Tofs.at(7));

	hismanager->Fill("PID_109", fp2Tofs[0], fp2.pin.at(0).energy);
	hismanager->Fill("PID_110", fp2Tofs[0], fp2.pin.at(1).energy);
	hismanager->Fill("PID_111", fp2Tofs[0], fp2.pin.at(2).energy);
	hismanager->Fill("PID_112", fp2Tofs[0], fp2.pin.at(3).energy);

	hismanager->Fill("PID_113", fp2Tofs[2], fp2.pin.at(0).energy);
	hismanager->Fill("PID_114", fp2Tofs[2], fp2.pin.at(1).energy);
	hismanager->Fill("PID_115", fp2Tofs[2], fp2.pin.at(2).energy);
	hismanager->Fill("PID_116", fp2Tofs[2], fp2.pin.at(3).energy);

	hismanager->Fill("PID_117", fp2Tofs[4], fp2.pin.at(0).energy);
	hismanager->Fill("PID_118", fp2Tofs[4], fp2.pin.at(1).energy);
	hismanager->Fill("PID_119", fp2Tofs[4], fp2.pin.at(2).energy);
	hismanager->Fill("PID_120", fp2Tofs[4], fp2.pin.at(3).energy);

	hismanager->Fill("PID_121", fp2Tofs[6], fp2.pin.at(0).energy);
	hismanager->Fill("PID_122", fp2Tofs[6], fp2.pin.at(1).energy);
	hismanager->Fill("PID_123", fp2Tofs[6], fp2.pin.at(2).energy);
	hismanager->Fill("PID_124", fp2Tofs[6], fp2.pin.at(3).energy);

	hismanager->Fill("PID_125",fp2.pin.at(0).energy,fp2.pin.at(1).energy);
	hismanager->Fill("PID_126",fp2.pin.at(0).energy,fp2.pin.at(2).energy);
	hismanager->Fill("PID_127",fp2.pin.at(0).energy,fp2.pin.at(3).energy);

	hismanager->Fill("PID_300",db3.ppac0.xpos);
	hismanager->Fill("PID_301",db3.ppac0.ypos);
	hismanager->Fill("PID_3000",db3.ppac0.xpos,db3.ppac0.ypos);
	
	hismanager->Fill("PID_310",db3.ppac1.xpos);
	hismanager->Fill("PID_311",db3.ppac1.ypos);
	hismanager->Fill("PID_3010",db3.ppac1.xpos,db3.ppac1.ypos);

	hismanager->Fill("PID_400",db4.ppac0.xpos);
	hismanager->Fill("PID_401",db4.ppac0.ypos);
	hismanager->Fill("PID_4000",db4.ppac0.xpos,db4.ppac0.ypos);
	
	hismanager->Fill("PID_410",db4.ppac1.xpos);
	hismanager->Fill("PID_411",db4.ppac1.ypos);
	hismanager->Fill("PID_4010",db4.ppac1.xpos,db4.ppac1.ypos);
	
	hismanager->Fill("PID_500",db5.ppac0.xpos);
	hismanager->Fill("PID_501",db5.ppac0.ypos);
	hismanager->Fill("PID_5000",db5.ppac0.xpos,db5.ppac0.ypos);

	hismanager->Fill("PID_510",db5.ppac1.xpos);
	hismanager->Fill("PID_511",db5.ppac1.ypos);
	hismanager->Fill("PID_5010",db5.ppac1.xpos,db5.ppac1.ypos);

	//in here tell it if what cut we made it in
	for( const auto& kv : this->isotopes ){
		//hismanager->Fill("PID_21", fp1Tofs[6], fp1.pin.at(0).energy);
		if( this->PIDPLOT == 21 ){
			if( cutmanager->IsWithin(kv.second,fp1Tofs[6],fp1.pin[0].energy) ){
				summary->AddEventTag(kv.first);
			}
		}else{
			this->console->error("No PID used");
		}
	}


	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool PidProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool PidProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Reset();
	return true;
}

void PidProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
	this->LoadCustomCuts(config);
	this->LoadHistogramSettings(config);

	this->PIDPLOT = config.attribute("PIDPlot").as_int(-1);

	for( pugi::xml_node isotope = config.child("Isotope"); isotope; isotope = isotope.next_sibling("Isotope") ){
		std::string tagname = isotope.attribute("name").as_string("");
		if( tagname.empty() ){
			throw std::runtime_error("isotope qualified, but no name given");
		}
		std::string cutname = isotope.attribute("cutid").as_string("");
		if( cutname.empty() ){
			throw std::runtime_error("isotope qualified, but no cutname provided");
		}
		std::string filename = isotope.attribute("filename").as_string("");
		if( filename.empty() ){
			throw std::runtime_error("isotope qualified, but cut file not given");
		}

		this->isotopes[tagname] = cutname;

		this->customcuts[cutname] = filename;
		this->console->info("Found Isotope : {} associated with Cut {} using file {}",tagname,cutname,filename);
	}

	for( const auto& kv : this->isotopes ){
		this->isotopetags.push_back(kv.first);
	}
}
		
void PidProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void PidProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager){
	hismanager->RegisterPlot<TH1F>("PID_1" ,"DB3P0 Anode - FP1XP1 TDiff; TDiff (ns)",this->h1dsettings.at(1));
	hismanager->RegisterPlot<TH1F>("PID_2" ,"DB3P0 Anode - FP1XP2 TDiff; TDiff (ns)",this->h1dsettings.at(2));
	hismanager->RegisterPlot<TH1F>("PID_3" ,"DB3P1 Anode - FP1XP1 TDiff; TDiff (ns)",this->h1dsettings.at(3));
	hismanager->RegisterPlot<TH1F>("PID_4" ,"DB3P1 Anode - FP1XP2 TDiff; TDiff (ns)",this->h1dsettings.at(4));
	hismanager->RegisterPlot<TH1F>("PID_5" ,"DB3 Scint L - FP1XP1 TDiff; TDiff (ns)",this->h1dsettings.at(5));
	hismanager->RegisterPlot<TH1F>("PID_6" ,"DB3 Scint L - FP1XP2 TDiff; TDiff (ns)",this->h1dsettings.at(6));
	hismanager->RegisterPlot<TH1F>("PID_7" ,"DB3 Scint R - FP1XP1 TDiff; TDiff (ns)",this->h1dsettings.at(7));
	hismanager->RegisterPlot<TH1F>("PID_8" ,"DB3 Scint R - FP1XP2 TDiff; TDiff (ns)",this->h1dsettings.at(8));

	hismanager->RegisterPlot<TH2F>("PID_9" ,"Pin 1 Energy vs DB3P0A-FP1XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(9));
	hismanager->RegisterPlot<TH2F>("PID_10","Pin 2 Energy vs DB3P0A-FP1XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(10));
	hismanager->RegisterPlot<TH2F>("PID_11","Pin 3 Energy vs DB3P0A-FP1XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(11));
	hismanager->RegisterPlot<TH2F>("PID_12","Pin 4 Energy vs DB3P0A-FP1XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(12));
                                                             
	hismanager->RegisterPlot<TH2F>("PID_13","Pin 1 Energy vs DB3P1A-FP1XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(13));
	hismanager->RegisterPlot<TH2F>("PID_14","Pin 2 Energy vs DB3P1A-FP1XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(14));
	hismanager->RegisterPlot<TH2F>("PID_15","Pin 3 Energy vs DB3P1A-FP1XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(15));
	hismanager->RegisterPlot<TH2F>("PID_16","Pin 4 Energy vs DB3P1A-FP1XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(16));

	hismanager->RegisterPlot<TH2F>("PID_17","Pin 1 Energy vs DB3SL-FP1XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(17));
	hismanager->RegisterPlot<TH2F>("PID_18","Pin 2 Energy vs DB3SL-FP1XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(18));
	hismanager->RegisterPlot<TH2F>("PID_19","Pin 3 Energy vs DB3SL-FP1XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(19));
	hismanager->RegisterPlot<TH2F>("PID_20","Pin 4 Energy vs DB3SL-FP1XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(20));
                                                                 
	hismanager->RegisterPlot<TH2F>("PID_21","Pin 1 Energy vs DB3SR-FP1XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(21));
	hismanager->RegisterPlot<TH2F>("PID_22","Pin 2 Energy vs DB3SR-FP1XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(22));
	hismanager->RegisterPlot<TH2F>("PID_23","Pin 3 Energy vs DB3SR-FP1XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(23));
	hismanager->RegisterPlot<TH2F>("PID_24","Pin 4 Energy vs DB3SR-FP1XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(24));

	hismanager->RegisterPlot<TH2F>("PID_25","Pin 2 vs Pin 1 Energy; Energy (keV); Energy (keV)",this->h2dsettings.at(25));
	hismanager->RegisterPlot<TH2F>("PID_26","Pin 3 vs Pin 1 Energy; Energy (keV); Energy (keV)",this->h2dsettings.at(26));
	hismanager->RegisterPlot<TH2F>("PID_27","Pin 4 vs Pin 1 Energy; Energy (keV); Energy (keV)",this->h2dsettings.at(27));

	//FP2
	hismanager->RegisterPlot<TH1F>("PID_101" ,"DB3P0 Anode - FP2XP1 TDiff; TDiff (ns)",this->h1dsettings.at(101));
	hismanager->RegisterPlot<TH1F>("PID_102" ,"DB3P0 Anode - FP2XP2 TDiff; TDiff (ns)",this->h1dsettings.at(102));
	hismanager->RegisterPlot<TH1F>("PID_103" ,"DB3P1 Anode - FP2XP1 TDiff; TDiff (ns)",this->h1dsettings.at(103));
	hismanager->RegisterPlot<TH1F>("PID_104" ,"DB3P1 Anode - FP2XP2 TDiff; TDiff (ns)",this->h1dsettings.at(104));
	hismanager->RegisterPlot<TH1F>("PID_105" ,"DB3 Scint L - FP2XP1 TDiff; TDiff (ns)",this->h1dsettings.at(105));
	hismanager->RegisterPlot<TH1F>("PID_106" ,"DB3 Scint L - FP2XP2 TDiff; TDiff (ns)",this->h1dsettings.at(106));
	hismanager->RegisterPlot<TH1F>("PID_107" ,"DB3 Scint R - FP2XP1 TDiff; TDiff (ns)",this->h1dsettings.at(107));
	hismanager->RegisterPlot<TH1F>("PID_108" ,"DB3 Scint R - FP2XP2 TDiff; TDiff (ns)",this->h1dsettings.at(108));

	hismanager->RegisterPlot<TH2F>("PID_109","Pin 1 Energy vs DB3P0A-FP2XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(109));
	hismanager->RegisterPlot<TH2F>("PID_110","Pin 2 Energy vs DB3P0A-FP2XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(110));
	hismanager->RegisterPlot<TH2F>("PID_111","Pin 3 Energy vs DB3P0A-FP2XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(111));
	hismanager->RegisterPlot<TH2F>("PID_112","Pin 4 Energy vs DB3P0A-FP2XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(112));
                                                                  
	hismanager->RegisterPlot<TH2F>("PID_113","Pin 1 Energy vs DB3P1A-FP2XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(113));
	hismanager->RegisterPlot<TH2F>("PID_114","Pin 2 Energy vs DB3P1A-FP2XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(114));
	hismanager->RegisterPlot<TH2F>("PID_115","Pin 3 Energy vs DB3P1A-FP2XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(115));
	hismanager->RegisterPlot<TH2F>("PID_116","Pin 4 Energy vs DB3P1A-FP2XP1; TDiff (ns); Energy (keV)",this->h2dsettings.at(116));

	hismanager->RegisterPlot<TH2F>("PID_117","Pin 1 Energy vs DB3SL-FP2XP; TDiff (ns); Energy (keV)1",this->h2dsettings.at(117));
	hismanager->RegisterPlot<TH2F>("PID_118","Pin 2 Energy vs DB3SL-FP2XP; TDiff (ns); Energy (keV)1",this->h2dsettings.at(118));
	hismanager->RegisterPlot<TH2F>("PID_119","Pin 3 Energy vs DB3SL-FP2XP; TDiff (ns); Energy (keV)1",this->h2dsettings.at(119));
	hismanager->RegisterPlot<TH2F>("PID_120","Pin 4 Energy vs DB3SL-FP2XP; TDiff (ns); Energy (keV)1",this->h2dsettings.at(120));
                                                                  
	hismanager->RegisterPlot<TH2F>("PID_121","Pin 1 Energy vs DB3SR-FP2XP; TDiff (ns); Energy (keV)1",this->h2dsettings.at(121));
	hismanager->RegisterPlot<TH2F>("PID_122","Pin 2 Energy vs DB3SR-FP2XP; TDiff (ns); Energy (keV)1",this->h2dsettings.at(122));
	hismanager->RegisterPlot<TH2F>("PID_123","Pin 3 Energy vs DB3SR-FP2XP; TDiff (ns); Energy (keV)1",this->h2dsettings.at(123));
	hismanager->RegisterPlot<TH2F>("PID_124","Pin 4 Energy vs DB3SR-FP2XP; TDiff (ns); Energy (keV)1",this->h2dsettings.at(124));

	hismanager->RegisterPlot<TH2F>("PID_125","Pin 2 vs Pin 1 Energy; Energy (keV); Energy (keV)",this->h2dsettings.at(125));
	hismanager->RegisterPlot<TH2F>("PID_126","Pin 3 vs Pin 1 Energy; Energy (keV); Energy (keV)",this->h2dsettings.at(126));
	hismanager->RegisterPlot<TH2F>("PID_127","Pin 4 vs Pin 1 Energy; Energy (keV); Energy (keV)",this->h2dsettings.at(127));

	hismanager->RegisterPlot<TH1F>("PID_300","DB3 PPAC0 Left - Right; TDiff (ns)",this->h1dsettings.at(300));
	hismanager->RegisterPlot<TH1F>("PID_301","DB3 PPAC0 Up - Down; TDiff (ns)",this->h1dsettings.at(301));
	
	hismanager->RegisterPlot<TH1F>("PID_310","DB3 PPAC1 Left - Right; TDiff (ns)",this->h1dsettings.at(310));
	hismanager->RegisterPlot<TH1F>("PID_311","DB3 PPAC1 Up - Down; TDiff (ns)",this->h1dsettings.at(311));

	hismanager->RegisterPlot<TH1F>("PID_400","DB4 PPAC0 Left - Right; TDiff (ns)",this->h1dsettings.at(400));
	hismanager->RegisterPlot<TH1F>("PID_401","DB4 PPAC0 Up - Down; TDiff (ns)",this->h1dsettings.at(401));
	
	hismanager->RegisterPlot<TH1F>("PID_410","DB4 PPAC1 Left - Right; TDiff (ns)",this->h1dsettings.at(410));
	hismanager->RegisterPlot<TH1F>("PID_411","DB4 PPAC1 Up - Down; TDiff (ns)",this->h1dsettings.at(411));

	hismanager->RegisterPlot<TH1F>("PID_500","DB5 PPAC0 Left - Right; TDiff (ns)",this->h1dsettings.at(500));
	hismanager->RegisterPlot<TH1F>("PID_501","DB5 PPAC0 Up - Down; TDiff (ns)",this->h1dsettings.at(501));
	
	hismanager->RegisterPlot<TH1F>("PID_510","DB5 PPAC1 Left - Right; TDiff (ns)",this->h1dsettings.at(510));
	hismanager->RegisterPlot<TH1F>("PID_511","DB5 PPAC1 Up - Down; TDiff (ns)",this->h1dsettings.at(511));

	hismanager->RegisterPlot<TH2F>("PID_3000","DB3 PPAC0 Image; TDiff (ns); TDiff (ns)",this->h2dsettings.at(3000));
	hismanager->RegisterPlot<TH2F>("PID_3010","DB3 PPAC1 Image; TDiff (ns); TDiff (ns)",this->h2dsettings.at(3010));

	hismanager->RegisterPlot<TH2F>("PID_4000","DB4 PPAC0 Image; TDiff (ns); TDiff (ns)",this->h2dsettings.at(4000));
	hismanager->RegisterPlot<TH2F>("PID_4010","DB4 PPAC1 Image; TDiff (ns); TDiff (ns)",this->h2dsettings.at(4010));

	hismanager->RegisterPlot<TH2F>("PID_5000","DB5 PPAC0 Image; TDiff (ns); TDiff (ns)",this->h2dsettings.at(5000));
	hismanager->RegisterPlot<TH2F>("PID_5010","DB5 PPAC1 Image; TDiff (ns); TDiff (ns)",this->h2dsettings.at(5010));

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

void PidProcessor::Reset(){
	for ( auto& iter: fp1Tofs){
		iter = std::numeric_limits<double>::max();
	}
	for ( auto& iter: fp2Tofs){
		iter = std::numeric_limits<double>::max();
	}
}


void PidProcessor::FillStruct(PhysicsData* data, ProcessorStruct::PidDet &det){
	det.energy = data->GetEnergy();
	det.time = data->GetCFDTimeStamp();
	det.saturation = data->GetSaturation();
	det.pileup = data->GetPileup();
};

size_t PidProcessor::GetNumFP1Pins() const{
	return this->fp1.pin.size();
}

size_t PidProcessor::GetNumFP2Pins() const{
	return this->fp2.pin.size();
}

double PidProcessor::GetFP1Tof(size_t idx) const {
	return this->fp1Tofs.at(idx);
}

double PidProcessor::GetFP2Tof(size_t idx) const {
	return this->fp2Tofs.at(idx);
}

double PidProcessor::GetFP1PinEnergy(size_t idx) const {
	return this->fp1.pin.at(idx).energy;
}

double PidProcessor::GetFP2PinEnergy(size_t idx) const {
	return this->fp2.pin.at(idx).energy;
}

const std::vector<std::string>& PidProcessor::GetIsotopeTags() const{
	return this->isotopetags;
}

inline double PidProcessor::CalcPPACPosition(const double& a,const double& b){
	if( a > 0.0 and b > 0.0 ){
		return a - b;
	}else{
		return -999.0;
	}
}
