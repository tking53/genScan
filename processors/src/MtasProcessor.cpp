#include "MtasProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

MtasProcessor::MtasProcessor(const std::string& log) : Processor(log,"MtasProcessor",{"mtas"}){
	this->fronttag = "front";
	this->backtag = "back";
	this->currsubtype = SUBTYPE::UNKNOWN;
	this->foundfirstevt = false;
	this->globalfirsttime = 0.0;

	this->h1dsettings = {
		{ 3100, {16384,0,16384} },
		{ 3110, {16384,0,16384} },
		{ 3115, {16384,0,16384} },
		{ 3120, {16384,0,16384} },
		{ 3125, {16384,0,16384} },
		{ 3130, {16384,0,16384} },
		{ 3135, {16384,0,16384} },
		{ 3140, {16384,0,16384} },
		{ 3145, {16384,0,16384} },

		{ 3200, {16384,0,16384} },
		{ 3210, {16384,0,16384} },
		{ 3215, {16384,0,16384} },
		{ 3220, {16384,0,16384} },
		{ 3225, {16384,0,16384} },
		{ 3230, {16384,0,16384} },
		{ 3235, {16384,0,16384} },
		{ 3240, {16384,0,16384} },
		{ 3245, {16384,0,16384} },

		{ 3300, {16384,0,16384} },
		{ 3310, {16384,0,16384} },
		{ 3315, {16384,0,16384} },
		{ 3320, {16384,0,16384} },
		{ 3325, {16384,0,16384} },
		{ 3330, {16384,0,16384} },
		{ 3335, {16384,0,16384} },
		{ 3340, {16384,0,16384} },
		{ 3345, {16384,0,16384} }
	};

	this->h2dsettings = {
		{3101, {16384,0.0,16384,24,0,24}},
		{3150, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3151, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3152, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3153, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3154, {4096,0.0,4096.0,4096,0.0,4096.0}},

		{3160, {1024,-1.0,1.0,8192,0.0,8192.0}},

		{31508, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{31518, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{31528, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{31538, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{31548, {2048,0.0,16384.0,2048,0.0,16384.0}},

		{3201, {16384,0.0,16384,24,0,24}},
		{3250, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3251, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3252, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3253, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3254, {4096,0.0,4096.0,4096,0.0,4096.0}},

		{3260, {1024,-1.0,1.0,8192,0.0,8192.0}},

		{32508, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{32518, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{32528, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{32538, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{32548, {2048,0.0,16384.0,2048,0.0,16384.0}},

		{3301, {16384,0.0,16384,24,0,24}},
		{3350, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3351, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3352, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3353, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3354, {4096,0.0,4096.0,4096,0.0,4096.0}},

		{3360, {1024,-1.0,1.0,8192,0.0,8192.0}},

		{33508, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{33518, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{33528, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{33538, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{33548, {2048,0.0,16384.0,2048,0.0,16384.0}},

		{3411, {8192,0.0,8192.0,12,0,12}},
		{3412, {8192,0.0,8192.0,12,0,12}},
		{3413, {8192,0.0,8192.0,12,0,12}},
		{3414, {8192,0.0,8192.0,12,0,12}},

		{3431, {8192,0.0,8192.0,12,0,12}},
		{3432, {8192,0.0,8192.0,12,0,12}},
		{3433, {8192,0.0,8192.0,12,0,12}},
		{3434, {8192,0.0,8192.0,12,0,12}},

		{3511, {8192,0.0,8192.0,12,0,12}},
		{3512, {8192,0.0,8192.0,12,0,12}},
		{3513, {8192,0.0,8192.0,12,0,12}},
		{3514, {8192,0.0,8192.0,12,0,12}},

		{3531, {8192,0.0,8192.0,12,0,12}},
		{3532, {8192,0.0,8192.0,12,0,12}},
		{3533, {8192,0.0,8192.0,12,0,12}},
		{3534, {8192,0.0,8192.0,12,0,12}},

		{4100, {8192,0.0,8192.0,1024,0.0,1024}},
		{4101, {8192,0.0,8192.0,1024,0.0,1024}},
		{4102, {8192,0.0,8192.0,1024,0.0,1024}},
		{4103, {8192,0.0,8192.0,1024,0.0,1024}},
		{4104, {8192,0.0,8192.0,1024,0.0,1024}},
		{41008, {2048,0.0,16384.0,1024,0.0,1024}},
		{41018, {2048,0.0,16384.0,1024,0.0,1024}},
		{41028, {2048,0.0,16384.0,1024,0.0,1024}},
		{41038, {2048,0.0,16384.0,1024,0.0,1024}},
		{41048, {2048,0.0,16384.0,1024,0.0,1024}},

		{4200, {8192,0.0,8192.0,1024,0.0,1024}},
		{4201, {8192,0.0,8192.0,1024,0.0,1024}},
		{4202, {8192,0.0,8192.0,1024,0.0,1024}},
		{4203, {8192,0.0,8192.0,1024,0.0,1024}},
		{4204, {8192,0.0,8192.0,1024,0.0,1024}},
		{42008, {2048,0.0,16384.0,1024,0.0,1024}},
		{42018, {2048,0.0,16384.0,1024,0.0,1024}},
		{42028, {2048,0.0,16384.0,1024,0.0,1024}},
		{42038, {2048,0.0,16384.0,1024,0.0,1024}},
		{42048, {2048,0.0,16384.0,1024,0.0,1024}},

		{4300, {8192,0.0,8192.0,1024,0.0,1024}},
		{4301, {8192,0.0,8192.0,1024,0.0,1024}},
		{4302, {8192,0.0,8192.0,1024,0.0,1024}},
		{4303, {8192,0.0,8192.0,1024,0.0,1024}},
		{4304, {8192,0.0,8192.0,1024,0.0,1024}},
		{43008, {2048,0.0,16384.0,1024,0.0,1024}},
		{43018, {2048,0.0,16384.0,1024,0.0,1024}},
		{43028, {2048,0.0,16384.0,1024,0.0,1024}},
		{43038, {2048,0.0,16384.0,1024,0.0,1024}},
		{43048, {2048,0.0,16384.0,1024,0.0,1024}}
	};

	this->Position = std::vector<double>(24,0.0);
	this->Center = std::vector<double>(12,0.0);
	this->Inner = std::vector<double>(12,0.0);
	this->Middle = std::vector<double>(12,0.0);
	this->Outer = std::vector<double>(12,0.0);

	this->diagnosticplots = false;

	this->RawCenter = std::vector<double>(12,0.0);
	this->RawInner = std::vector<double>(12,0.0);
	this->RawMiddle = std::vector<double>(12,0.0);
	this->RawOuter = std::vector<double>(12,0.0);

	this->CalCenter = std::vector<double>(12,0.0);
	this->CalInner = std::vector<double>(12,0.0);
	this->CalMiddle = std::vector<double>(12,0.0);
	this->CalOuter = std::vector<double>(12,0.0);

	this->CenterHits = std::vector<int>(12,0);
	this->InnerHits = std::vector<int>(12,0);
	this->MiddleHits = std::vector<int>(12,0);
	this->OuterHits = std::vector<int>(12,0);

	this->CrystalEnergy = std::vector<double>(24,0.0);
	this->TotalEnergy = std::vector<double>(5,0.0);

	this->IndividualPMTPileup = std::vector<bool>(48,false);
	this->CenterPileup = false;
	this->InnerPileup = false;
	this->MiddlePileup = false;
	this->OuterPileup = false;
	this->AnyPileup = false;

	this->IndividualPMTSaturate = std::vector<bool>(48,false);
	this->CenterSaturate = false;
	this->InnerSaturate = false;
	this->MiddleSaturate = false;
	this->OuterSaturate = false;
	this->AnySaturate = false;

	this->NumFire = std::vector<int>(5,0);
	this->CenterFire = false;
	this->InnerFire = false;
	this->MiddleFire = false;
	this->OuterFire = false;
	this->AnyFire = false;

	this->FirstTime = -1.0;
	this->LastTime = -1.0;

	for( size_t ii = 0; ii < 12; ++ii ){
		this->PosCorrectionMap.push_back(nullptr);
	}

	this->GenerateHexagonShapes();
	this->SegmentDataVec = std::vector<ProcessorStruct::MtasSegment>(24,ProcessorStruct::DEFAULT_MTAS_SEGMENT_STRUCT);
	this->TotalDataVec = std::vector<ProcessorStruct::MtasTotal>(5,ProcessorStruct::DEFAULT_MTAS_TOTAL_STRUCT);

	this->gamma = "gamma";
	this->muon = "muon";
}

[[maybe_unused]] bool MtasProcessor::PreProcess(EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	auto summary = eventhistory->GetCurrentEventSummary();
	summary->GetDetectorSummary(this->AllDefaultRegex["mtas"],this->SummaryData);
	for( const auto& evt : this->SummaryData ){
		auto subtype = evt->GetSubType();
		auto group = evt->GetGroup();

		auto isfront = evt->HasTag(this->fronttag);
		auto isback = evt->HasTag(this->backtag);

		//this->console->info("{} {} {}",subtype,group,group.size());

		int segmentid = std::stoi(group);

		if( (not isfront and not isback) or (isfront and isback) ){
			this->console->error("evt {} has neither front of back or both front and back tags",*evt);
			throw std::runtime_error("invalid xml config");
		}

		int offset = 0;
		if( subtype.compare("center") == 0 ){
			this->currsubtype = SUBTYPE::CENTER;
		}else if( subtype.compare("inner") == 0 ){
			this->currsubtype = SUBTYPE::INNER;
			offset = 12;
		}else if( subtype.compare("middle") == 0 ){
			this->currsubtype = SUBTYPE::MIDDLE;
			offset = 24;
		}else if( subtype.compare("outer") == 0 ){
			this->currsubtype = SUBTYPE::OUTER;
			offset = 36;
		}else{
			this->currsubtype = SUBTYPE::UNKNOWN;
			this->console->error("evt {} has incorrect subtype for mtas, needs to be center/inner/middle/outer",*evt);
			throw std::runtime_error("invalid xml config");
		}

		int position = segmentid - 1; 	
		int detectorposition = 2*position + isback;
		int pmtposition = offset + detectorposition;

		if( not foundfirstevt ){
			foundfirstevt = true;
			globalfirsttime = evt->GetTimeStamp();
		}

		if( evt->GetPileup() or evt->GetSaturation() ){
			//ignore the saturated channel, but keep everything else in this current event
			if( evt->GetPileup() ){
				this->IndividualPMTPileup[pmtposition] = true;
				this->AnyPileup = true;
				switch( this->currsubtype ){
					case SUBTYPE::CENTER:
						this->CenterPileup = true;
						break;
					case SUBTYPE::INNER:
						this->InnerPileup = true;
						break;
					case SUBTYPE::MIDDLE:
						this->MiddlePileup = true;
						break;
					case SUBTYPE::OUTER:
						this->OuterPileup = true;
						break;
					default:
						break;
				}
			}
			if( evt->GetSaturation() ){
				summary->AddEventTag(this->muon);
				this->IndividualPMTSaturate[pmtposition] = true;
				this->AnySaturate = true;
				switch( this->currsubtype ){
					case SUBTYPE::CENTER:
						this->CenterSaturate = true;
						break;
					case SUBTYPE::INNER:
						this->InnerSaturate = true;
						break;
					case SUBTYPE::MIDDLE:
						this->MiddleSaturate = true;
						break;
					case SUBTYPE::OUTER:
						this->OuterSaturate = true;
						break;
					default:
						break;
				}
			}
			continue;
		}

		if( currsubtype == SUBTYPE::CENTER ){
			if( !this->CenterHits[detectorposition] ){
				this->Center[detectorposition] = evt->GetEnergy();
				this->RawCenter[detectorposition] = evt->GetRawEnergyWRandom();
				this->CalCenter[detectorposition] = evt->GetEnergy();
				this->TimeStamps.push_back(evt->GetTimeStamp());
				++this->CenterHits[detectorposition];
				if( isfront ){
					this->SegmentDataVec[detectorposition/2].frontenergy = this->Center[detectorposition];
					this->SegmentDataVec[detectorposition/2].fronttimestamp = this->TimeStamps.back();
				}else{
					this->SegmentDataVec[detectorposition/2].backenergy = this->Center[detectorposition];
					this->SegmentDataVec[detectorposition/2].backtimestamp = this->TimeStamps.back();
				}
			}else{
				++this->CenterHits[detectorposition];
			}
		}else if( currsubtype == SUBTYPE::INNER ){
			if( !this->InnerHits[detectorposition] ){
				this->Inner[detectorposition] = evt->GetEnergy();
				this->RawInner[detectorposition] = evt->GetRawEnergyWRandom();
				this->CalInner[detectorposition] = evt->GetEnergy();
				this->TimeStamps.push_back(evt->GetTimeStamp());
				++this->InnerHits[detectorposition];
				if( isfront ){
					this->SegmentDataVec[detectorposition/2 + 6].frontenergy = this->Inner[detectorposition];
					this->SegmentDataVec[detectorposition/2 + 6].fronttimestamp = this->TimeStamps.back();
				}else{
					this->SegmentDataVec[detectorposition/2 + 6].backenergy = this->Inner[detectorposition];
					this->SegmentDataVec[detectorposition/2 + 6].backtimestamp = this->TimeStamps.back();
				}
			}else{
				++this->InnerHits[detectorposition];
			}
		}else if( currsubtype == SUBTYPE::MIDDLE ){
			if( !this->MiddleHits[detectorposition] ){
				this->Middle[detectorposition] = evt->GetEnergy();
				this->RawMiddle[detectorposition] = evt->GetRawEnergyWRandom();
				this->CalMiddle[detectorposition] = evt->GetEnergy();
				this->TimeStamps.push_back(evt->GetTimeStamp());
				++this->MiddleHits[detectorposition];
				if( isfront ){
					this->SegmentDataVec[detectorposition/2 + 12].frontenergy = this->Middle[detectorposition];
					this->SegmentDataVec[detectorposition/2 + 12].fronttimestamp = this->TimeStamps.back();
				}else{
					this->SegmentDataVec[detectorposition/2 + 12].backenergy = this->Middle[detectorposition];
					this->SegmentDataVec[detectorposition/2 + 12].backtimestamp = this->TimeStamps.back();
				}
			}else{
				++this->MiddleHits[detectorposition];
			}
		}else if( currsubtype == SUBTYPE::OUTER ){
			if( !this->OuterHits[detectorposition] ){
				this->Outer[detectorposition] = evt->GetEnergy();
				this->RawOuter[detectorposition] = evt->GetRawEnergyWRandom();
				this->CalOuter[detectorposition] = evt->GetEnergy();
				this->TimeStamps.push_back(evt->GetTimeStamp());
				++this->OuterHits[detectorposition];
				if( isfront ){
					this->SegmentDataVec[detectorposition/2 + 18].frontenergy = this->Outer[detectorposition];
					this->SegmentDataVec[detectorposition/2 + 18].fronttimestamp = this->TimeStamps.back();
				}else{
					this->SegmentDataVec[detectorposition/2 + 18].backenergy = this->Outer[detectorposition];
					this->SegmentDataVec[detectorposition/2 + 18].backtimestamp = this->TimeStamps.back();
				}
			}else{
				++this->OuterHits[detectorposition];
			}
		}else{
			this->console->error("we shouldn't have gotten here");
			throw std::runtime_error("invalid xml config");
		}

	}

	if( this->TimeStamps.size() > 0 ){
		this->FirstTime = *(std::min_element(this->TimeStamps.begin(),this->TimeStamps.end()));
		this->LastTime = *(std::max_element(this->TimeStamps.begin(),this->TimeStamps.end()));
	}

	this->currevttime = (this->FirstTime - globalfirsttime)*1.0e-9;

	for( int ii = 0; ii < 6; ++ii ){
		//the center calculation is done later since we swapped modules at some point in history
		if( this->InnerHits[2*ii] and this->InnerHits[2*ii + 1] ){
			this->CrystalEnergy[ii+6] = (this->Inner[2*ii] + this->Inner[2*ii + 1])/2.0;
			this->SegmentDataVec[ii+6].sumenergy = this->CrystalEnergy[ii+6];
			this->SegmentDataVec[ii+6].avgtimestamp = (this->SegmentDataVec[ii+6].fronttimestamp+this->SegmentDataVec[ii+6].backtimestamp)/2.0;
			this->Position[ii + 6] = this->CalcPosition(this->RawInner[2*ii],this->RawInner[2*ii + 1]);
			this->InnerFire = true;
			this->AnyFire = true;
			this->NumFire[0] += 1;
			this->NumFire[2] += 1;
		}
		if( this->MiddleHits[2*ii] and this->MiddleHits[2*ii + 1] ){
			this->CrystalEnergy[ii+12] = (this->Middle[2*ii] + this->Middle[2*ii + 1])/2.0;
			this->SegmentDataVec[ii+12].sumenergy = this->CrystalEnergy[ii+12];
			this->SegmentDataVec[ii+12].avgtimestamp = (this->SegmentDataVec[ii+12].fronttimestamp+this->SegmentDataVec[ii+12].backtimestamp)/2.0;
			this->Position[ii + 12] = this->CalcPosition(this->RawMiddle[2*ii],this->RawMiddle[2*ii + 1]);
			this->MiddleFire = true;
			this->AnyFire = true;
			this->NumFire[0] += 1;
			this->NumFire[3] += 1;
		}
		if( this->OuterHits[2*ii] and this->OuterHits[2*ii + 1] ){
			this->CrystalEnergy[ii+18] = (this->Outer[2*ii] + this->Outer[2*ii + 1])/2.0;
			this->SegmentDataVec[ii+18].sumenergy = this->CrystalEnergy[ii+18];
			this->SegmentDataVec[ii+18].avgtimestamp = (this->SegmentDataVec[ii+18].fronttimestamp+this->SegmentDataVec[ii+18].backtimestamp)/2.0;
			this->Position[ii + 18] = this->CalcPosition(this->RawOuter[2*ii],this->RawOuter[2*ii + 1]);
			this->OuterFire = true;
			this->AnyFire = true;
			this->NumFire[0] += 1;
			this->NumFire[4] += 1;
		}
	}

	if( this->UseOldCenter ){
		this->OldCenterCalculation();
	}else{
		this->NewCenterCalculation();
	}

	this->TotalDataVec[0].sumenergy = this->TotalEnergy[0];
	this->TotalDataVec[0].numfire = this->NumFire[0];
	this->TotalDataVec[0].saturate = this->AnySaturate;
	this->TotalDataVec[0].pileup = this->AnyPileup;
	this->TotalDataVec[0].timestamp = this->FirstTime;

	this->TotalDataVec[1].sumenergy = this->TotalEnergy[1];
	this->TotalDataVec[1].numfire = this->NumFire[1];
	this->TotalDataVec[1].saturate = this->CenterSaturate;
	this->TotalDataVec[1].pileup = this->CenterPileup;
	this->TotalDataVec[1].timestamp = this->FirstTime;

	this->TotalDataVec[2].sumenergy = this->TotalEnergy[2];
	this->TotalDataVec[2].numfire = this->NumFire[2];
	this->TotalDataVec[2].saturate = this->InnerSaturate;
	this->TotalDataVec[2].pileup = this->InnerPileup;
	this->TotalDataVec[2].timestamp = this->FirstTime;

	this->TotalDataVec[3].sumenergy = this->TotalEnergy[3];
	this->TotalDataVec[3].numfire = this->NumFire[3];
	this->TotalDataVec[3].saturate = this->MiddleSaturate;
	this->TotalDataVec[3].pileup = this->MiddlePileup;
	this->TotalDataVec[3].timestamp = this->FirstTime;

	this->TotalDataVec[4].sumenergy = this->TotalEnergy[4];
	this->TotalDataVec[4].numfire = this->NumFire[4];
	this->TotalDataVec[4].saturate = this->OuterSaturate;
	this->TotalDataVec[4].pileup = this->OuterPileup;
	this->TotalDataVec[4].timestamp = this->FirstTime;

	if( (not this->AnySaturate) and (not this->AnyPileup) ){
		for( int ii = 0; ii < 6; ++ii ){
			auto currhx = this->HexagonShapes[ii].center;
			if( this->CenterHits[2*ii] ){
				for( size_t jj = 0; jj < this->CenterHits[2*ii]; ++jj ){
					this->MTAS_2501->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2503->Fill(currhx.first,currhx.second,1.0);
				}
			}
			if( this->CenterHits[2*ii+1] ){
				for( size_t jj = 0; jj < this->CenterHits[2*ii]; ++jj ){
					this->MTAS_2502->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2503->Fill(currhx.first,currhx.second,-1.0);
				}
			}
			if( this->CenterHits[2*ii] and this->CenterHits[2*ii+1] ){
				this->MTAS_2500->Fill(currhx.first,currhx.second,1.0);
			}

			currhx = this->HexagonShapes[ii+6].center;
			if( this->InnerHits[2*ii] ){
				for( size_t jj = 0; jj < this->InnerHits[2*ii]; ++jj ){
					this->MTAS_2501->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2503->Fill(currhx.first,currhx.second,1.0);
				}
			}
			if( this->InnerHits[2*ii+1] ){
				for( size_t jj = 0; jj < this->InnerHits[2*ii]; ++jj ){
					this->MTAS_2502->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2503->Fill(currhx.first,currhx.second,-1.0);
				}
			}
			if( this->InnerHits[2*ii] and this->InnerHits[2*ii+1] ){
				this->MTAS_2500->Fill(currhx.first,currhx.second,1.0);
			}

			currhx = this->HexagonShapes[ii+12].center;
			if( this->MiddleHits[2*ii] ){
				for( size_t jj = 0; jj < this->MiddleHits[2*ii]; ++jj ){
					this->MTAS_2501->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2503->Fill(currhx.first,currhx.second,1.0);
				}
			}
			if( this->MiddleHits[2*ii+1] ){
				for( size_t jj = 0; jj < this->MiddleHits[2*ii]; ++jj ){
					this->MTAS_2502->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2503->Fill(currhx.first,currhx.second,-1.0);
				}
			}
			if( this->MiddleHits[2*ii] and this->MiddleHits[2*ii+1] ){
				this->MTAS_2500->Fill(currhx.first,currhx.second,1.0);
			}

			currhx = this->HexagonShapes[ii+18].center;
			if( this->OuterHits[2*ii] ){
				for( size_t jj = 0; jj < this->OuterHits[2*ii]; ++jj ){
					this->MTAS_2501->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2503->Fill(currhx.first,currhx.second,1.0);
				}
			}
			if( this->OuterHits[2*ii+1] ){
				for( size_t jj = 0; jj < this->OuterHits[2*ii]; ++jj ){
					this->MTAS_2502->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2503->Fill(currhx.first,currhx.second,-1.0);
				}
			}
			if( this->OuterHits[2*ii] and this->OuterHits[2*ii+1] ){
				this->MTAS_2500->Fill(currhx.first,currhx.second,1.0);
			}

		}

		hismanager->Fill("MTAS_4200",this->TotalEnergy[0],this->currevttime*1000.0);
		hismanager->Fill("MTAS_4201",this->TotalEnergy[0],this->currevttime);
		hismanager->Fill("MTAS_4202",this->TotalEnergy[0],this->currevttime/60.0);
		hismanager->Fill("MTAS_4203",this->TotalEnergy[0],this->currevttime/(60.0*60.0));
		hismanager->Fill("MTAS_4204",this->TotalEnergy[0],this->currevttime/(60.0*60.0*24.0));

		hismanager->Fill("MTAS_42008",this->TotalEnergy[0],this->currevttime*1000.0);
		hismanager->Fill("MTAS_42018",this->TotalEnergy[0],this->currevttime);
		hismanager->Fill("MTAS_42028",this->TotalEnergy[0],this->currevttime/60.0);
		hismanager->Fill("MTAS_42038",this->TotalEnergy[0],this->currevttime/(60.0*60.0));
		hismanager->Fill("MTAS_42048",this->TotalEnergy[0],this->currevttime/(60.0*60.0*24.0));

		hismanager->Fill("MTAS_3200",this->TotalEnergy[0]);

		hismanager->Fill("MTAS_3210",this->TotalEnergy[1]);
		hismanager->Fill("MTAS_3220",this->TotalEnergy[2]);
		hismanager->Fill("MTAS_3230",this->TotalEnergy[3]);
		hismanager->Fill("MTAS_3240",this->TotalEnergy[4]);

		hismanager->Fill("MTAS_3251",this->TotalEnergy[0],this->TotalEnergy[1]);
		hismanager->Fill("MTAS_32518",this->TotalEnergy[0],this->TotalEnergy[1]);

		for( int ii = 0; ii < 24; ++ii ){
			hismanager->Fill("MTAS_3201",this->CrystalEnergy[ii],ii);
		}

		for( int ii = 0; ii < 6; ++ii ){
			std::string id = std::to_string(ii);

			std::string name = "MTAS_326"+id+"_F";
			hismanager->Fill(name,this->Position[ii],this->RawCenter[2*ii]);

			name = "MTAS_326"+id+"_B";
			hismanager->Fill(name,this->Position[ii],this->RawCenter[2*ii + 1]);

			name = "MTAS_326"+id;
			hismanager->Fill(name,this->Position[ii],(this->RawCenter[2*ii] + this->RawCenter[2*ii + 1])/2.0);

			hismanager->Fill("MTAS_3215",this->CrystalEnergy[ii]);
			hismanager->Fill("MTAS_3225",this->CrystalEnergy[ii+6]);
			hismanager->Fill("MTAS_3235",this->CrystalEnergy[ii+12]);
			hismanager->Fill("MTAS_3245",this->CrystalEnergy[ii+18]);

			hismanager->Fill("MTAS_3250",this->TotalEnergy[0],this->CrystalEnergy[ii+6]);
			hismanager->Fill("MTAS_3250",this->TotalEnergy[0],this->CrystalEnergy[ii+12]);
			hismanager->Fill("MTAS_3250",this->TotalEnergy[0],this->CrystalEnergy[ii+18]);

			hismanager->Fill("MTAS_3252",this->TotalEnergy[0],this->CrystalEnergy[ii]);

			hismanager->Fill("MTAS_3253",this->TotalEnergy[1],this->CrystalEnergy[ii]);

			hismanager->Fill("MTAS_32508",this->TotalEnergy[0],this->CrystalEnergy[ii+6]);
			hismanager->Fill("MTAS_32508",this->TotalEnergy[0],this->CrystalEnergy[ii+12]);
			hismanager->Fill("MTAS_32508",this->TotalEnergy[0],this->CrystalEnergy[ii+18]);

			hismanager->Fill("MTAS_32528",this->TotalEnergy[0],this->CrystalEnergy[ii]);

			hismanager->Fill("MTAS_32538",this->TotalEnergy[1],this->CrystalEnergy[ii]);

			if( (not this->InnerFire) and (not this->MiddleFire) and (not this->OuterFire) ){
				hismanager->Fill("MTAS_3254",this->TotalEnergy[0],this->CrystalEnergy[ii]);
				hismanager->Fill("MTAS_32548",this->TotalEnergy[0],this->CrystalEnergy[ii]);
			}
		}
	}

	if( this->TotalEnergy[0] > 0.0 ){
		summary->AddEventTag(this->gamma);
	}
	summary->AddEventObservable("MTAS_Total",this->TotalEnergy[0]);

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool MtasProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool MtasProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->Reset();

	return true;
}

void MtasProcessor::NewCenterCalculation(){
	for( int ii = 0; ii < 6; ++ii ){
		if( this->CenterHits[2*ii] and this->CenterHits[2*ii + 1] ){
			this->CrystalEnergy[ii] = std::sqrt(this->Center[2*ii] * this->Center[2*ii + 1]);//(this->Center[2*ii] + this->Center[2*ii + 1])/2.0;
			this->SegmentDataVec[ii].sumenergy = this->CrystalEnergy[ii];
			this->SegmentDataVec[ii].avgtimestamp = (this->SegmentDataVec[ii].fronttimestamp+this->SegmentDataVec[ii].backtimestamp)/2.0;
			this->Position[ii] = this->CalcPosition(this->RawCenter[2*ii],this->RawCenter[2*ii + 1]);
			this->CenterFire = true;
			this->AnyFire = true;
			this->NumFire[0] += 1;
			this->NumFire[1] += 1;
		}
	}
	for( size_t ii = 0; ii < 6; ++ii ){
		if( ( this->PosCorrectionMap[2*ii] != nullptr ) and ( this->PosCorrectionMap[2*ii + 1] != nullptr ) ){
			if( this->CenterHits[2*ii] and this->CenterHits[2*ii + 1] ){
				auto front = this->Center[2*ii];
				auto back = this->Center[2*ii + 1];
				this->Center[2*ii] = this->PosCorrectionMap[2*ii]->Correct(this->Center[2*ii],this->Position[ii]);
				this->Center[2*ii + 1] = this->PosCorrectionMap[2*ii + 1]->Correct(this->Center[2*ii + 1],this->Position[ii]);
				auto front2 = this->Center[2*ii];
				auto back2 = this->Center[2*ii + 1];
				//this->console->info("Front {}->{} , Back {}->{}",front,front2,back,back2);
				this->CrystalEnergy[ii] = (this->Center[2*ii] + this->Center[2*ii + 1])/2.0;
				//don't update the position
				//this->Position[ii] = this->CalcPosition(this->Center[2*ii],this->Center[2*ii + 1]);
			}
		}	
	}

	for( int ii = 0; ii < 6; ++ii ){
		this->TotalEnergy[0] += this->CrystalEnergy[ii];
		this->TotalEnergy[0] += this->CrystalEnergy[ii+6];
		this->TotalEnergy[0] += this->CrystalEnergy[ii+12];
		this->TotalEnergy[0] += this->CrystalEnergy[ii+18];

		this->TotalEnergy[1] += this->CrystalEnergy[ii];
		this->TotalEnergy[2] += this->CrystalEnergy[ii+6];
		this->TotalEnergy[3] += this->CrystalEnergy[ii+12];
		this->TotalEnergy[4] += this->CrystalEnergy[ii+18];
	}
}

void MtasProcessor::OldCenterCalculation(){
	for( int ii = 0; ii < 6; ++ii ){
		if( this->CenterHits[2*ii] and this->CenterHits[2*ii + 1] ){
			this->CrystalEnergy[ii] = (this->Center[2*ii] + this->Center[2*ii + 1])/2.0;
			this->SegmentDataVec[ii].sumenergy = this->CrystalEnergy[ii];
			this->SegmentDataVec[ii].avgtimestamp = (this->SegmentDataVec[ii].fronttimestamp+this->SegmentDataVec[ii].backtimestamp)/2.0;
			this->Position[ii] = this->CalcPosition(this->RawCenter[2*ii],this->RawCenter[2*ii + 1]);
			this->CenterFire = true;
			this->AnyFire = true;
			this->NumFire[0] += 1;
			this->NumFire[1] += 1;
		}
	}
	for( int ii = 0; ii < 6; ++ii ){
		//already requiring pairs, so if we have 1 pair we divide by 1, 2 we divide by 2 which was supposed to be each chunk by 4
		//therefore 6 pairs is 12
		if( this->NumFire[0] == 6 ){
			this->TotalEnergy[0] += this->CrystalEnergy[ii]/this->NumFire[0];
		}
		this->TotalEnergy[0] += this->CrystalEnergy[ii+6];
		this->TotalEnergy[0] += this->CrystalEnergy[ii+12];
		this->TotalEnergy[0] += this->CrystalEnergy[ii+18];

		if( this->NumFire[0] == 6 ){
			this->TotalEnergy[1] += this->CrystalEnergy[ii]/this->NumFire[0];
		}
		this->TotalEnergy[2] += this->CrystalEnergy[ii+6];
		this->TotalEnergy[3] += this->CrystalEnergy[ii+12];
		this->TotalEnergy[4] += this->CrystalEnergy[ii+18];
	}
}

void MtasProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");

	for( pugi::xml_node pos = config.child("PositionCorrection"); pos; pos = pos.next_sibling("PositionCorrection") ){
		int id = pos.attribute("id").as_int(-1);
		double p0 = pos.attribute("p0").as_double(0.0);
		double p1 = pos.attribute("p1").as_double(0.0);
		double cross = pos.attribute("cross").as_double(1.0);
		std::string tag = pos.attribute("tag").as_string("");
		this->PosCorrectionMap[id].reset(new Correction::ExpoPosCorrection());
		this->PosCorrectionMap[id]->constant = p0;
		this->PosCorrectionMap[id]->slope = p1;
		this->PosCorrectionMap[id]->mean = cross;
		this->console->info("Found PositionCorrection Node for {} : p0:{} p1:{} cross:{}, E'= E*(cross/exp(p0+p1*P)); P = (Efront-Eback)/(Efront+Eback)",tag,p0,p1,cross);
	}

	this->diagnosticplots = config.attribute("diagnostic").as_bool(false);
	this->UseOldCenter = config.attribute("oldcenter").as_bool(false);

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void MtasProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void MtasProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager){
	this->MTAS_2500 = hismanager->RegisterPlot("MTAS_2500","MTAS Segment Hit Map Beam is out of the page");
	for( const auto& hx : this->HexagonShapes ){
		this->MTAS_2500->AddBin(6,hx.xcoords,hx.ycoords);
	}
	this->MTAS_2501 = hismanager->RegisterPlot("MTAS_2501","MTAS Front Hit Map Beam is out of the page");
	for( const auto& hx : this->HexagonShapes ){
		this->MTAS_2501->AddBin(6,hx.xcoords,hx.ycoords);
	}
	this->MTAS_2502 = hismanager->RegisterPlot("MTAS_2502","MTAS Back Hit Map Beam is out of the page");
	for( const auto& hx : this->HexagonShapes ){
		this->MTAS_2502->AddBin(6,hx.xcoords,hx.ycoords);
	}
	this->MTAS_2503 = hismanager->RegisterPlot("MTAS_2503","MTAS Asymmetry Hit Map Beam is out of the page, Front is positive, Back is negative");
	for( const auto& hx : this->HexagonShapes ){
		this->MTAS_2503->AddBin(6,hx.xcoords,hx.ycoords);
	}

	//MTAS diagnostic plots, always want these no matter what
	hismanager->RegisterPlot<TH1F>("MTAS_3200","Mtas Total; Energy (keV)",this->h1dsettings.at(3200));
	hismanager->RegisterPlot<TH2F>("MTAS_3201","Sum F+B; Energy (keV); F+B Pair (arb.)",this->h2dsettings.at(3201));

	hismanager->RegisterPlot<TH1F>("MTAS_3210","Mtas Center Sum; Energy (keV)",this->h1dsettings.at(3210));
	hismanager->RegisterPlot<TH1F>("MTAS_3215","Mtas Center Stack; Energy (keV)",this->h1dsettings.at(3215));

	hismanager->RegisterPlot<TH1F>("MTAS_3220","Mtas Inner Sum; Energy (keV)",this->h1dsettings.at(3220));
	hismanager->RegisterPlot<TH1F>("MTAS_3225","Mtas Inner Stack; Energy (keV)",this->h1dsettings.at(3225));

	hismanager->RegisterPlot<TH1F>("MTAS_3230","Mtas Middle Sum; Energy (keV)",this->h1dsettings.at(3230));
	hismanager->RegisterPlot<TH1F>("MTAS_3235","Mtas Middle Stack; Energy (keV)",this->h1dsettings.at(3235));

	hismanager->RegisterPlot<TH1F>("MTAS_3240","Mtas Outer Sum; Energy (keV)",this->h1dsettings.at(3240));
	hismanager->RegisterPlot<TH1F>("MTAS_3245","Mtas Outer Stack; Energy (keV)",this->h1dsettings.at(3245));

	hismanager->RegisterPlot<TH2F>("MTAS_3250","I,M,O vs Mtas Total; Energy (keV); Energy (keV)",this->h2dsettings.at(3250));
	hismanager->RegisterPlot<TH2F>("MTAS_3251","C vs Mtas Total; Energy (keV); Energy (keV)",this->h2dsettings.at(3251));
	hismanager->RegisterPlot<TH2F>("MTAS_3252","C Segment vs Mtas Total; Energy (keV); Energy (keV)",this->h2dsettings.at(3252));
	hismanager->RegisterPlot<TH2F>("MTAS_3253","C Segment vs C; Energy (keV); Energy (keV)",this->h2dsettings.at(3253));
	hismanager->RegisterPlot<TH2F>("MTAS_3254","C Segment vs Mtas Total (veto any I,M,O) ; Energy (keV); Energy (keV)",this->h2dsettings.at(3254));
	hismanager->RegisterPlot<TH2F>("MTAS_32508","I,M,O vs Mtas Total; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(32508));
	hismanager->RegisterPlot<TH2F>("MTAS_32518","C vs Mtas Total; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(32518));
	hismanager->RegisterPlot<TH2F>("MTAS_32528","C Segment vs Mtas Total; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(32528));
	hismanager->RegisterPlot<TH2F>("MTAS_32538","C Segment vs C; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(32538));
	hismanager->RegisterPlot<TH2F>("MTAS_32548","C Segment vs Mtas Total (veto any I,M,O) ; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(32548));

	hismanager->RegisterPlot<TH2F>("MTAS_4200","Run Time vs Mtas Total; Energy (keV); Run Time (ms)",this->h2dsettings.at(4200));
	hismanager->RegisterPlot<TH2F>("MTAS_4201","Run Time vs Mtas Total; Energy (keV); Run Time (s)",this->h2dsettings.at(4201));
	hismanager->RegisterPlot<TH2F>("MTAS_4202","Run Time vs Mtas Total; Energy (keV); Run Time (min)",this->h2dsettings.at(4202));
	hismanager->RegisterPlot<TH2F>("MTAS_4203","Run Time vs Mtas Total; Energy (keV); Run Time (hr)",this->h2dsettings.at(4203));
	hismanager->RegisterPlot<TH2F>("MTAS_4204","Run Time vs Mtas Total; Energy (keV); Run Time (day)",this->h2dsettings.at(4204));

	hismanager->RegisterPlot<TH2F>("MTAS_42008","Run Time vs Mtas Total; Energy (8 keV/bin); Run Time (ms)",this->h2dsettings.at(42008));
	hismanager->RegisterPlot<TH2F>("MTAS_42018","Run Time vs Mtas Total; Energy (8 keV/bin); Run Time (s)",this->h2dsettings.at(42018));
	hismanager->RegisterPlot<TH2F>("MTAS_42028","Run Time vs Mtas Total; Energy (8 keV/bin); Run Time (min)",this->h2dsettings.at(42028));
	hismanager->RegisterPlot<TH2F>("MTAS_42038","Run Time vs Mtas Total; Energy (8 keV/bin); Run Time (hr)",this->h2dsettings.at(42038));
	hismanager->RegisterPlot<TH2F>("MTAS_42048","Run Time vs Mtas Total; Energy (8 keV/bin); Run Time (day)",this->h2dsettings.at(42048));

	//center position correction plots
	for( size_t ii = 0; ii < 6; ++ii ){
		std::string name = "MTAS_326"+std::to_string(ii)+"_F";
		std::string title = "C"+std::to_string(ii+1)+"F vs Center Position; Position (arb.); Energy (channel)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3260));

		name = "MTAS_326"+std::to_string(ii)+"_B";
		title = "C"+std::to_string(ii+1)+"B vs Center Position; Position (arb.); Energy (channel)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3260));

		name = "MTAS_326"+std::to_string(ii);
		title = "C"+std::to_string(ii+1)+" vs Center Position Sum; Position (arb.); Energy (channel)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3260));
	}

	//declare the beta gated and not-beta histograms, but we don't fill them until parent processor has told which we are
	this->DeclareBetaPlots(hismanager);
	this->DeclareAntiBetaPlots(hismanager);

	this->console->info("Finished Declaring Plots");
}

void MtasProcessor::DeclareBetaPlots(PLOTS::PlotRegistry* hismanager){
	//beta event
	hismanager->RegisterPlot<TH1F>("MTAS_3300","Mtas Total #beta-gated; Energy (keV)",this->h1dsettings.at(3300));
	hismanager->RegisterPlot<TH2F>("MTAS_3301","Sum F+B; Energy (keV); F+B Pair (arb.)",this->h2dsettings.at(3301));
	
	hismanager->RegisterPlot<TH1F>("MTAS_3310","Mtas Center Sum #beta-gated; Energy (keV)",this->h1dsettings.at(3310));
	hismanager->RegisterPlot<TH1F>("MTAS_3315","Mtas Center Stack #beta-gated; Energy (keV)",this->h1dsettings.at(3315));
	
	hismanager->RegisterPlot<TH1F>("MTAS_3320","Mtas Inner Sum #beta-gated; Energy (keV)",this->h1dsettings.at(3320));
	hismanager->RegisterPlot<TH1F>("MTAS_3325","Mtas Inner Stack #beta-gated; Energy (keV)",this->h1dsettings.at(3325));
	
	hismanager->RegisterPlot<TH1F>("MTAS_3330","Mtas Middle Sum #beta-gated; Energy (keV)",this->h1dsettings.at(3330));
	hismanager->RegisterPlot<TH1F>("MTAS_3335","Mtas Middle Stack #beta-gated; Energy (keV)",this->h1dsettings.at(3335));
	
	hismanager->RegisterPlot<TH1F>("MTAS_3340","Mtas Outer Sum #beta-gated; Energy (keV)",this->h1dsettings.at(3340));
	hismanager->RegisterPlot<TH1F>("MTAS_3345","Mtas Outer Stack #beta-gated; Energy (keV)",this->h1dsettings.at(3345));
	
	hismanager->RegisterPlot<TH2F>("MTAS_3350","I,M,O vs Mtas Total #beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(3350));
	hismanager->RegisterPlot<TH2F>("MTAS_3351","C vs Mtas Total #beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(3351));
	hismanager->RegisterPlot<TH2F>("MTAS_3352","C Segment vs Mtas Total #beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(3352));
	hismanager->RegisterPlot<TH2F>("MTAS_3353","C Segment vs C #beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(3353));
	hismanager->RegisterPlot<TH2F>("MTAS_3354","C Segment vs Mtas Total (veto any I,M,O) #beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(3354));
	
	hismanager->RegisterPlot<TH2F>("MTAS_33508","I,M,O vs Mtas Total #beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33508));
	hismanager->RegisterPlot<TH2F>("MTAS_33518","C vs Mtas Total #beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33518));
	hismanager->RegisterPlot<TH2F>("MTAS_33528","C Segment vs Mtas Total #beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33528));
	hismanager->RegisterPlot<TH2F>("MTAS_33538","C Segment vs C #beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33538));
	hismanager->RegisterPlot<TH2F>("MTAS_33548","C Segment vs Mtas Total (veto any I,M,O)  #beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33548));
	
	hismanager->RegisterPlot<TH2F>("MTAS_4300","Run Time vs Mtas Total #beta-gated; Energy (keV); Run Time (ms)",this->h2dsettings.at(4300));
	hismanager->RegisterPlot<TH2F>("MTAS_4301","Run Time vs Mtas Total #beta-gated; Energy (keV); Run Time (s)",this->h2dsettings.at(4301));
	hismanager->RegisterPlot<TH2F>("MTAS_4302","Run Time vs Mtas Total #beta-gated; Energy (keV); Run Time (min)",this->h2dsettings.at(4302));
	hismanager->RegisterPlot<TH2F>("MTAS_4303","Run Time vs Mtas Total #beta-gated; Energy (keV); Run Time (hr)",this->h2dsettings.at(4303));
	hismanager->RegisterPlot<TH2F>("MTAS_4304","Run Time vs Mtas Total #beta-gated; Energy (keV); Run Time (day)",this->h2dsettings.at(4304));
	
	hismanager->RegisterPlot<TH2F>("MTAS_43008","Run Time vs Mtas Total #beta-gated; Energy (8 keV/bin); Run Time (ms)",this->h2dsettings.at(43008));
	hismanager->RegisterPlot<TH2F>("MTAS_43018","Run Time vs Mtas Total #beta-gated; Energy (8 keV/bin); Run Time (s)",this->h2dsettings.at(43018));
	hismanager->RegisterPlot<TH2F>("MTAS_43028","Run Time vs Mtas Total #beta-gated; Energy (8 keV/bin); Run Time (min)",this->h2dsettings.at(43028));
	hismanager->RegisterPlot<TH2F>("MTAS_43038","Run Time vs Mtas Total #beta-gated; Energy (8 keV/bin); Run Time (hr)",this->h2dsettings.at(43038));
	hismanager->RegisterPlot<TH2F>("MTAS_43048","Run Time vs Mtas Total #beta-gated; Energy (8 keV/bin); Run Time (day)",this->h2dsettings.at(43048));

	if( this->diagnosticplots ){
		hismanager->RegisterPlot<TH2F>("MTAS_3431","Raw IndividualPMT C PMTs #beta-gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3431));
		hismanager->RegisterPlot<TH2F>("MTAS_3432","Raw IndividualPMT I PMTs #beta-gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3432));
		hismanager->RegisterPlot<TH2F>("MTAS_3433","Raw IndividualPMT M PMTs #beta-gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3433));
		hismanager->RegisterPlot<TH2F>("MTAS_3434","Raw IndividualPMT O PMTs #beta-gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3434));

		hismanager->RegisterPlot<TH2F>("MTAS_3531","Calibrated IndividualPMT C PMTs #beta-gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3531));
		hismanager->RegisterPlot<TH2F>("MTAS_3532","Calibrated IndividualPMT I PMTs #beta-gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3532));
		hismanager->RegisterPlot<TH2F>("MTAS_3533","Calibrated IndividualPMT M PMTs #beta-gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3533));
		hismanager->RegisterPlot<TH2F>("MTAS_3534","Calibrated IndividualPMT O PMTs #beta-gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3534));
	}

	//center position correction plots
	for( size_t ii = 0; ii < 6; ++ii ){
		std::string name  = "MTAS_336"+std::to_string(ii)+"_F";
		std::string title = "C"+std::to_string(ii+1)+"F vs Center Position #beta-gated; Position (arb.); Energy (channel)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3360));

		name = "MTAS_336"+std::to_string(ii)+"_B";
		title = "C"+std::to_string(ii+1)+"B vs Center Position #beta-gated; Position (arb.); Energy (channel)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3360));

		name = "MTAS_336"+std::to_string(ii);
		title = "C"+std::to_string(ii+1)+" vs Center Position Sum #beta-gated; Position (arb.); Energy (channel)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3360));
	}

}

void MtasProcessor::DeclareAntiBetaPlots(PLOTS::PlotRegistry* hismanager){
	//not beta event
	hismanager->RegisterPlot<TH1F>("MTAS_3100","Mtas Total anti-#beta-gated; Energy (keV)",this->h1dsettings.at(3100));
	hismanager->RegisterPlot<TH2F>("MTAS_3101","Sum F+B; Energy (keV); F+B Pair (arb.)",this->h2dsettings.at(3101));
	
	hismanager->RegisterPlot<TH1F>("MTAS_3110","Mtas Center Sum anti-#beta-gated; Energy (keV)",this->h1dsettings.at(3110));
	hismanager->RegisterPlot<TH1F>("MTAS_3115","Mtas Center Stack anti-#beta-gated; Energy (keV)",this->h1dsettings.at(3115));
	
	hismanager->RegisterPlot<TH1F>("MTAS_3120","Mtas Inner Sum anti-#beta-gated; Energy (keV)",this->h1dsettings.at(3120));
	hismanager->RegisterPlot<TH1F>("MTAS_3125","Mtas Inner Stack anti-#beta-gated; Energy (keV)",this->h1dsettings.at(3125));
	
	hismanager->RegisterPlot<TH1F>("MTAS_3130","Mtas Middle Sum anti-#beta-gated; Energy (keV)",this->h1dsettings.at(3130));
	hismanager->RegisterPlot<TH1F>("MTAS_3135","Mtas Middle Stack anti-#beta-gated; Energy (keV)",this->h1dsettings.at(3135));
	
	hismanager->RegisterPlot<TH1F>("MTAS_3140","Mtas Outer Sum anti-#beta-gated; Energy (keV)",this->h1dsettings.at(3140));
	hismanager->RegisterPlot<TH1F>("MTAS_3145","Mtas Outer Stack anti-#beta-gated; Energy (keV)",this->h1dsettings.at(3145));
	
	hismanager->RegisterPlot<TH2F>("MTAS_3150","I,M,O vs Mtas Total anti-#beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(3150));
	hismanager->RegisterPlot<TH2F>("MTAS_3151","C vs Mtas Total anti-#beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(3151));
	hismanager->RegisterPlot<TH2F>("MTAS_3152","C Segment vs Mtas Total anti-#beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(3152));
	hismanager->RegisterPlot<TH2F>("MTAS_3153","C Segment vs C anti-#beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(3153));
	hismanager->RegisterPlot<TH2F>("MTAS_3154","C Segment vs Mtas Total (veto any I,M,O) anti-#beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(3154));
	
	hismanager->RegisterPlot<TH2F>("MTAS_31508","I,M,O vs Mtas Total anti-#beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(31508));
	hismanager->RegisterPlot<TH2F>("MTAS_31518","C vs Mtas Total anti-#beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(31518));
	hismanager->RegisterPlot<TH2F>("MTAS_31528","C Segment vs Mtas Total anti-#beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(31528));
	hismanager->RegisterPlot<TH2F>("MTAS_31538","C Segment vs Mtas Total anti-#beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(31538));
	hismanager->RegisterPlot<TH2F>("MTAS_31548","C Segment vs Mtas Total (veto any I,M,O) anti-#beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(31548));
	
	hismanager->RegisterPlot<TH2F>("MTAS_4100","Run Time vs Mtas Total anti-#beta-gated; Energy (keV); Run Time (ms)",this->h2dsettings.at(4100));
	hismanager->RegisterPlot<TH2F>("MTAS_4101","Run Time vs Mtas Total anti-#beta-gated; Energy (keV); Run Time (s)",this->h2dsettings.at(4101));
	hismanager->RegisterPlot<TH2F>("MTAS_4102","Run Time vs Mtas Total anti-#beta-gated; Energy (keV); Run Time (min)",this->h2dsettings.at(4102));
	hismanager->RegisterPlot<TH2F>("MTAS_4103","Run Time vs Mtas Total anti-#beta-gated; Energy (keV); Run Time (hr)",this->h2dsettings.at(4103));
	hismanager->RegisterPlot<TH2F>("MTAS_4104","Run Time vs Mtas Total anti-#beta-gated; Energy (keV); Run Time (day)",this->h2dsettings.at(4104));
	
	hismanager->RegisterPlot<TH2F>("MTAS_41008","Run Time vs Mtas Total anti-#beta-gated; Energy (8 keV/bin); Run Time (ms)",this->h2dsettings.at(41008));
	hismanager->RegisterPlot<TH2F>("MTAS_41018","Run Time vs Mtas Total anti-#beta-gated; Energy (8 keV/bin); Run Time (s)",this->h2dsettings.at(41018));
	hismanager->RegisterPlot<TH2F>("MTAS_41028","Run Time vs Mtas Total anti-#beta-gated; Energy (8 keV/bin); Run Time (min)",this->h2dsettings.at(41028));
	hismanager->RegisterPlot<TH2F>("MTAS_41038","Run Time vs Mtas Total anti-#beta-gated; Energy (8 keV/bin); Run Time (hr)",this->h2dsettings.at(41038));
	hismanager->RegisterPlot<TH2F>("MTAS_41048","Run Time vs Mtas Total anti-#beta-gated; Energy (8 keV/bin); Run Time (day)",this->h2dsettings.at(41048));

	if( this->diagnosticplots ){
		hismanager->RegisterPlot<TH2F>("MTAS_3411","Raw IndividualPMT C PMTs anti-#beta-gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3411));
		hismanager->RegisterPlot<TH2F>("MTAS_3412","Raw IndividualPMT I PMTs anti-#beta-gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3412));
		hismanager->RegisterPlot<TH2F>("MTAS_3413","Raw IndividualPMT M PMTs anti-#beta-gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3413));
		hismanager->RegisterPlot<TH2F>("MTAS_3414","Raw IndividualPMT O PMTs anti-#beta-gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3414));

		hismanager->RegisterPlot<TH2F>("MTAS_3511","Calibrated IndividualPMT C PMTs anti-#beta-gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3511));
		hismanager->RegisterPlot<TH2F>("MTAS_3512","Calibrated IndividualPMT I PMTs anti-#beta-gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3512));
		hismanager->RegisterPlot<TH2F>("MTAS_3513","Calibrated IndividualPMT M PMTs anti-#beta-gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3513));
		hismanager->RegisterPlot<TH2F>("MTAS_3514","Calibrated IndividualPMT O PMTs anti-#beta-gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3514));
	}

	//center position correction plots
	for( size_t ii = 0; ii < 6; ++ii ){
		std::string name = "MTAS_316"+std::to_string(ii)+"_F";
		std::string title = "C"+std::to_string(ii+1)+"F vs Center Position anti #beta-gated; Position (arb.); Energy (channel)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3160));

		name = "MTAS_316"+std::to_string(ii)+"_B";
		title = "C"+std::to_string(ii+1)+"B vs Center Position anti #beta-gated; Position (arb.); Energy (channel)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3160));

		name = "MTAS_316"+std::to_string(ii);
		title = "C"+std::to_string(ii+1)+" vs Center Position Sum anti #beta-gated; Position (arb.); Energy (channel)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3160));
	}
}

void MtasProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
	this->OutputTree = new TTree("Mtas","Mtas Processor output");
	this->OutputTree->Branch("Total",&(this->TotalDataVec.at(0)));

	this->OutputTree->Branch("C1",&(this->SegmentDataVec.at(0)));
	this->OutputTree->Branch("C2",&(this->SegmentDataVec.at(1)));
	this->OutputTree->Branch("C3",&(this->SegmentDataVec.at(2)));
	this->OutputTree->Branch("C4",&(this->SegmentDataVec.at(3)));
	this->OutputTree->Branch("C5",&(this->SegmentDataVec.at(4)));
	this->OutputTree->Branch("C6",&(this->SegmentDataVec.at(5)));
	this->OutputTree->Branch("CenterRing",&(this->TotalDataVec.at(1)));
	
	this->OutputTree->Branch("I1",&(this->SegmentDataVec.at(6)));
	this->OutputTree->Branch("I2",&(this->SegmentDataVec.at(7)));
	this->OutputTree->Branch("I3",&(this->SegmentDataVec.at(8)));
	this->OutputTree->Branch("I4",&(this->SegmentDataVec.at(9)));
	this->OutputTree->Branch("I5",&(this->SegmentDataVec.at(10)));
	this->OutputTree->Branch("I6",&(this->SegmentDataVec.at(11)));
	this->OutputTree->Branch("InnerRing",&(this->TotalDataVec.at(2)));
	
	this->OutputTree->Branch("M1",&(this->SegmentDataVec.at(12)));
	this->OutputTree->Branch("M2",&(this->SegmentDataVec.at(13)));
	this->OutputTree->Branch("M3",&(this->SegmentDataVec.at(14)));
	this->OutputTree->Branch("M4",&(this->SegmentDataVec.at(15)));
	this->OutputTree->Branch("M5",&(this->SegmentDataVec.at(16)));
	this->OutputTree->Branch("M6",&(this->SegmentDataVec.at(17)));
	this->OutputTree->Branch("MiddleRing",&(this->TotalDataVec.at(3)));
	
	this->OutputTree->Branch("O1",&(this->SegmentDataVec.at(18)));
	this->OutputTree->Branch("O2",&(this->SegmentDataVec.at(19)));
	this->OutputTree->Branch("O3",&(this->SegmentDataVec.at(20)));
	this->OutputTree->Branch("O4",&(this->SegmentDataVec.at(21)));
	this->OutputTree->Branch("O5",&(this->SegmentDataVec.at(22)));
	this->OutputTree->Branch("O6",&(this->SegmentDataVec.at(23)));
	this->OutputTree->Branch("OuterRing",&(this->TotalDataVec.at(4)));
	
	outputtrees[this->ProcessorName] = this->OutputTree;
}

void MtasProcessor::CleanupTree(){
	for( auto& e : this->SegmentDataVec ){
		e = ProcessorStruct::DEFAULT_MTAS_SEGMENT_STRUCT;
	}
	for( auto& e : this->TotalDataVec ){
		e = ProcessorStruct::DEFAULT_MTAS_TOTAL_STRUCT;
	}
}

void MtasProcessor::Reset(){
	for( size_t ii = 0; ii < 24; ++ii ){
		this->CrystalEnergy[ii] = 0.0;
		this->IndividualPMTPileup[ii] = false;
		this->IndividualPMTPileup[24+ii] = false;
		this->IndividualPMTSaturate[ii] = false;
		this->IndividualPMTSaturate[24+ii] = false;
		this->Position[ii] = 0.0;
	}
	for( size_t ii = 0; ii < 5; ++ii ){
		this->TotalEnergy[ii] = 0.0;
		this->NumFire[ii] = 0;
	}

	this->CenterPileup = false;
	this->InnerPileup = false;
	this->MiddlePileup = false;
	this->OuterPileup = false;
	this->AnyPileup = false;

	this->CenterSaturate = false;
	this->InnerSaturate = false;
	this->MiddleSaturate = false;
	this->OuterSaturate = false;
	this->AnySaturate = false;

	this->CenterFire = false;
	this->InnerFire = false;
	this->MiddleFire = false;
	this->OuterFire = false;
	this->AnyFire = false;

	this->FirstTime = -1.0;
	this->LastTime = -1.0;

	this->TimeStamps.clear();

	for( size_t ii = 0; ii < 12; ++ii ){
		this->Center[ii] = 0.0;
		this->Inner[ii] = 0.0;
		this->Middle[ii] = 0.0;
		this->Outer[ii] = 0.0;

		this->RawCenter[ii] = 0.0;
		this->RawInner[ii] = 0.0;
		this->RawMiddle[ii] = 0.0;
		this->RawOuter[ii] = 0.0;

		this->CalCenter[ii] = 0.0;
		this->CalInner[ii] = 0.0;
		this->CalMiddle[ii] = 0.0;
		this->CalOuter[ii] = 0.0;

		this->CenterHits[ii] = 0.0;
		this->InnerHits[ii] = 0.0;
		this->MiddleHits[ii] = 0.0;
		this->OuterHits[ii] = 0.0;
	}

	this->CenterFire = false;
	this->InnerFire = false;
	this->MiddleFire = false;
	this->OuterFire = false;
}

void MtasProcessor::GenerateHexagonShapes(){
	this->hexagonsize = 1.0;	
	this->hexagonpad = 0.95;

	double hexwidth = (3.0/2.0)*this->hexagonsize;
	double hexheight = std::sqrt(3.0)*this->hexagonsize;

	Geometry::hexagon centercoords(0.0,0.0,this->hexagonsize,this->hexagonpad);
	Geometry::hexagon centerholecoords(0.0,0.0,0.5*this->hexagonsize,1.0);
	this->HexagonShapes.push_back(Geometry::hexagon(0.0,0.0,this->hexagonsize,this->hexagonpad));
	//this is C1
	this->HexagonShapes[0].xcoords[0] = (0.55*centercoords.xcoords[2]+0.45*centercoords.xcoords[1]);
	this->HexagonShapes[0].ycoords[0] = (0.55*centercoords.ycoords[2]+0.45*centercoords.ycoords[1]);

	this->HexagonShapes[0].xcoords[1] = centercoords.xcoords[2];
	this->HexagonShapes[0].ycoords[1] = centercoords.ycoords[2];

	this->HexagonShapes[0].xcoords[2] = (0.55*centercoords.xcoords[2]+0.45*centercoords.xcoords[3]);
	this->HexagonShapes[0].ycoords[2] = (0.55*centercoords.ycoords[2]+0.45*centercoords.ycoords[3]);

	this->HexagonShapes[0].xcoords[3] = (0.55*centerholecoords.xcoords[2]+0.45*centerholecoords.xcoords[3]);
	this->HexagonShapes[0].ycoords[3] = (0.55*centerholecoords.ycoords[2]+0.45*centerholecoords.ycoords[3]);

	this->HexagonShapes[0].xcoords[4] = centerholecoords.xcoords[2];
	this->HexagonShapes[0].ycoords[4] = centerholecoords.ycoords[2];

	this->HexagonShapes[0].xcoords[5] = (0.55*centerholecoords.xcoords[2]+0.45*centerholecoords.xcoords[1]);
	this->HexagonShapes[0].ycoords[5] = (0.55*centerholecoords.ycoords[2]+0.45*centerholecoords.ycoords[1]);

	this->HexagonShapes[0].center.first = (centercoords.xcoords[2]+centerholecoords.xcoords[2])/2.0;
	this->HexagonShapes[0].center.second = (centercoords.ycoords[2]+centerholecoords.ycoords[2])/2.0;


	this->HexagonShapes.push_back(Geometry::hexagon(0.0,0.0,this->hexagonsize,this->hexagonpad));
	//this is C2
	this->HexagonShapes[1].xcoords[0] = (0.55*centercoords.xcoords[3]+0.45*centercoords.xcoords[2]);
	this->HexagonShapes[1].ycoords[0] = (0.55*centercoords.ycoords[3]+0.45*centercoords.ycoords[2]);

	this->HexagonShapes[1].xcoords[1] = centercoords.xcoords[3];
	this->HexagonShapes[1].ycoords[1] = centercoords.ycoords[3];

	this->HexagonShapes[1].xcoords[2] = (0.45*centercoords.xcoords[4]+0.55*centercoords.xcoords[3]);
	this->HexagonShapes[1].ycoords[2] = (0.45*centercoords.ycoords[4]+0.55*centercoords.ycoords[3]);

	this->HexagonShapes[1].xcoords[3] = (0.45*centerholecoords.xcoords[4]+0.55*centerholecoords.xcoords[3]);
	this->HexagonShapes[1].ycoords[3] = (0.45*centerholecoords.ycoords[4]+0.55*centerholecoords.ycoords[3]);

	this->HexagonShapes[1].xcoords[4] = centerholecoords.xcoords[3];
	this->HexagonShapes[1].ycoords[4] = centerholecoords.ycoords[3];

	this->HexagonShapes[1].xcoords[5] = (0.45*centerholecoords.xcoords[2]+0.55*centerholecoords.xcoords[3]);
	this->HexagonShapes[1].ycoords[5] = (0.45*centerholecoords.ycoords[2]+0.55*centerholecoords.ycoords[3]);

	this->HexagonShapes[1].center.first = (centercoords.xcoords[3]+centerholecoords.xcoords[3])/2.0;
	this->HexagonShapes[1].center.second = (centercoords.ycoords[3]+centerholecoords.ycoords[3])/2.0;

	this->HexagonShapes.push_back(Geometry::hexagon(0.0,0.0,this->hexagonsize,this->hexagonpad));
	//this is C3
	this->HexagonShapes[2].xcoords[0] = (0.45*centercoords.xcoords[3]+0.55*centercoords.xcoords[4]);
	this->HexagonShapes[2].ycoords[0] = (0.45*centercoords.ycoords[3]+0.55*centercoords.ycoords[4]);

	this->HexagonShapes[2].xcoords[1] = centercoords.xcoords[4];
	this->HexagonShapes[2].ycoords[1] = centercoords.ycoords[4];

	this->HexagonShapes[2].xcoords[2] = (0.55*centercoords.xcoords[4]+0.45*centercoords.xcoords[5]);
	this->HexagonShapes[2].ycoords[2] = (0.55*centercoords.ycoords[4]+0.45*centercoords.ycoords[5]);

	this->HexagonShapes[2].xcoords[3] = (0.55*centerholecoords.xcoords[4]+0.45*centerholecoords.xcoords[5]);
	this->HexagonShapes[2].ycoords[3] = (0.55*centerholecoords.ycoords[4]+0.45*centerholecoords.ycoords[5]);

	this->HexagonShapes[2].xcoords[4] = centerholecoords.xcoords[4];
	this->HexagonShapes[2].ycoords[4] = centerholecoords.ycoords[4];

	this->HexagonShapes[2].xcoords[5] = (0.55*centerholecoords.xcoords[4]+0.45*centerholecoords.xcoords[3]);
	this->HexagonShapes[2].ycoords[5] = (0.55*centerholecoords.ycoords[4]+0.45*centerholecoords.ycoords[3]);

	this->HexagonShapes[2].center.first = (centercoords.xcoords[4]+centerholecoords.xcoords[4])/2.0;
	this->HexagonShapes[2].center.second = (centercoords.ycoords[4]+centerholecoords.ycoords[4])/2.0;

	this->HexagonShapes.push_back(Geometry::hexagon(0.0,0.0,this->hexagonsize,this->hexagonpad));
	//this is C4
	this->HexagonShapes[3].xcoords[0] = (0.55*centercoords.xcoords[5]+0.45*centercoords.xcoords[4]);
	this->HexagonShapes[3].ycoords[0] = (0.55*centercoords.ycoords[5]+0.45*centercoords.ycoords[4]);

	this->HexagonShapes[3].xcoords[1] = centercoords.xcoords[5];
	this->HexagonShapes[3].ycoords[1] = centercoords.ycoords[5];

	this->HexagonShapes[3].xcoords[2] = (0.45*centercoords.xcoords[0]+0.55*centercoords.xcoords[5]);
	this->HexagonShapes[3].ycoords[2] = (0.45*centercoords.ycoords[0]+0.55*centercoords.ycoords[5]);

	this->HexagonShapes[3].xcoords[3] = (0.45*centerholecoords.xcoords[0]+0.55*centerholecoords.xcoords[5]);
	this->HexagonShapes[3].ycoords[3] = (0.45*centerholecoords.ycoords[0]+0.55*centerholecoords.ycoords[5]);

	this->HexagonShapes[3].xcoords[4] = centerholecoords.xcoords[5];
	this->HexagonShapes[3].ycoords[4] = centerholecoords.ycoords[5];

	this->HexagonShapes[3].xcoords[5] = (0.45*centerholecoords.xcoords[4]+0.55*centerholecoords.xcoords[5]);
	this->HexagonShapes[3].ycoords[5] = (0.45*centerholecoords.ycoords[4]+0.55*centerholecoords.ycoords[5]);

	this->HexagonShapes[3].center.first = (centercoords.xcoords[5]+centerholecoords.xcoords[5])/2.0;
	this->HexagonShapes[3].center.second = (centercoords.ycoords[5]+centerholecoords.ycoords[5])/2.0;

	this->HexagonShapes.push_back(Geometry::hexagon(0.0,0.0,this->hexagonsize,this->hexagonpad));
	//this is C5
	this->HexagonShapes[4].xcoords[0] = (0.45*centercoords.xcoords[5]+0.55*centercoords.xcoords[0]);
	this->HexagonShapes[4].ycoords[0] = (0.45*centercoords.ycoords[5]+0.55*centercoords.ycoords[0]);

	this->HexagonShapes[4].xcoords[1] = centercoords.xcoords[0];
	this->HexagonShapes[4].ycoords[1] = centercoords.ycoords[0];

	this->HexagonShapes[4].xcoords[2] = (0.55*centercoords.xcoords[0]+0.45*centercoords.xcoords[1]);
	this->HexagonShapes[4].ycoords[2] = (0.55*centercoords.ycoords[0]+0.45*centercoords.ycoords[1]);

	this->HexagonShapes[4].xcoords[3] = (0.55*centerholecoords.xcoords[0]+0.45*centerholecoords.xcoords[1]);
	this->HexagonShapes[4].ycoords[3] = (0.55*centerholecoords.ycoords[0]+0.45*centerholecoords.ycoords[1]);

	this->HexagonShapes[4].xcoords[4] = centerholecoords.xcoords[0];
	this->HexagonShapes[4].ycoords[4] = centerholecoords.ycoords[0];

	this->HexagonShapes[4].xcoords[5] = (0.55*centerholecoords.xcoords[0]+0.45*centerholecoords.xcoords[5]);
	this->HexagonShapes[4].ycoords[5] = (0.55*centerholecoords.ycoords[0]+0.45*centerholecoords.ycoords[5]);

	this->HexagonShapes[4].center.first = (centercoords.xcoords[0]+centerholecoords.xcoords[0])/2.0;
	this->HexagonShapes[4].center.second = (centercoords.ycoords[0]+centerholecoords.ycoords[0])/2.0;

	this->HexagonShapes.push_back(Geometry::hexagon(0.0,0.0,this->hexagonsize,this->hexagonpad));
	//this is C6
	this->HexagonShapes[5].xcoords[0] = (0.45*centercoords.xcoords[0]+0.55*centercoords.xcoords[1]); 
	this->HexagonShapes[5].ycoords[0] = (0.45*centercoords.ycoords[0]+0.55*centercoords.ycoords[1]); 

	this->HexagonShapes[5].xcoords[1] = centercoords.xcoords[1]; 
	this->HexagonShapes[5].ycoords[1] = centercoords.ycoords[1]; 

	this->HexagonShapes[5].xcoords[2] = (0.45*centercoords.xcoords[2]+0.55*centercoords.xcoords[1]); 
	this->HexagonShapes[5].ycoords[2] = (0.45*centercoords.ycoords[2]+0.55*centercoords.ycoords[1]); 

	this->HexagonShapes[5].xcoords[3] = (0.45*centerholecoords.xcoords[2]+0.55*centerholecoords.xcoords[1]);
	this->HexagonShapes[5].ycoords[3] = (0.45*centerholecoords.ycoords[2]+0.55*centerholecoords.ycoords[1]);

	this->HexagonShapes[5].xcoords[4] = centerholecoords.xcoords[1];
	this->HexagonShapes[5].ycoords[4] = centerholecoords.ycoords[1];

	this->HexagonShapes[5].xcoords[5] = (0.45*centerholecoords.xcoords[0]+0.55*centerholecoords.xcoords[1]);
	this->HexagonShapes[5].ycoords[5] = (0.45*centerholecoords.ycoords[0]+0.55*centerholecoords.ycoords[1]);

	this->HexagonShapes[5].center.first = (centercoords.xcoords[1]+centerholecoords.xcoords[1])/2.0;
	this->HexagonShapes[5].center.second = (centercoords.ycoords[1]+centerholecoords.ycoords[1])/2.0;


	//inner
	this->HexagonShapes.push_back(Geometry::hexagon(0.0,hexheight,this->hexagonsize,this->hexagonpad));
	this->HexagonShapes.push_back(Geometry::hexagon(hexwidth,0.5*hexheight,this->hexagonsize,this->hexagonpad));
	this->HexagonShapes.push_back(Geometry::hexagon(hexwidth,-0.5*hexheight,this->hexagonsize,this->hexagonpad));
	this->HexagonShapes.push_back(Geometry::hexagon(0.0,-hexheight,this->hexagonsize,this->hexagonpad));
	this->HexagonShapes.push_back(Geometry::hexagon(-hexwidth,-0.5*hexheight,this->hexagonsize,this->hexagonpad));
	this->HexagonShapes.push_back(Geometry::hexagon(-hexwidth,0.5*hexheight,this->hexagonsize,this->hexagonpad));

	//middle
	this->HexagonShapes.push_back(Geometry::hexagon(hexwidth,1.5*hexheight,this->hexagonsize,this->hexagonpad));
	this->HexagonShapes.push_back(Geometry::hexagon(2.0*hexwidth,0.0,this->hexagonsize,this->hexagonpad));
	this->HexagonShapes.push_back(Geometry::hexagon(hexwidth,-1.5*hexheight,this->hexagonsize,this->hexagonpad));
	this->HexagonShapes.push_back(Geometry::hexagon(-hexwidth,-1.5*hexheight,this->hexagonsize,this->hexagonpad));
	this->HexagonShapes.push_back(Geometry::hexagon(-2.0*hexwidth,0.0,this->hexagonsize,this->hexagonpad));
	this->HexagonShapes.push_back(Geometry::hexagon(-hexwidth,1.5*hexheight,this->hexagonsize,this->hexagonpad));
	
	//outer
	this->HexagonShapes.push_back(Geometry::hexagon(0.0,2.0*hexheight,this->hexagonsize,this->hexagonpad));
	this->HexagonShapes.push_back(Geometry::hexagon(2.0*hexwidth,1.0*hexheight,this->hexagonsize,this->hexagonpad));
	this->HexagonShapes.push_back(Geometry::hexagon(2.0*hexwidth,-1.0*hexheight,this->hexagonsize,this->hexagonpad));
	this->HexagonShapes.push_back(Geometry::hexagon(0.0,-2.0*hexheight,this->hexagonsize,this->hexagonpad));
	this->HexagonShapes.push_back(Geometry::hexagon(-2.0*hexwidth,-1.0*hexheight,this->hexagonsize,this->hexagonpad));
	this->HexagonShapes.push_back(Geometry::hexagon(-2.0*hexwidth,1.0*hexheight,this->hexagonsize,this->hexagonpad));
}

void MtasProcessor::FillBetaPlots(PLOTS::PlotRegistry* hismanager){
	if( (not this->AnySaturate) and (not this->AnyPileup) ){
		hismanager->Fill("MTAS_3300",this->TotalEnergy[0]);

		hismanager->Fill("MTAS_3310",this->TotalEnergy[1]);
		hismanager->Fill("MTAS_3320",this->TotalEnergy[2]);
		hismanager->Fill("MTAS_3330",this->TotalEnergy[3]);
		hismanager->Fill("MTAS_3340",this->TotalEnergy[4]);

		hismanager->Fill("MTAS_3351",this->TotalEnergy[0],this->TotalEnergy[1]);
		hismanager->Fill("MTAS_33518",this->TotalEnergy[0],this->TotalEnergy[1]);

		for( int ii = 0; ii < 24; ++ii ){
			hismanager->Fill("MTAS_3301",this->CrystalEnergy[ii],ii);
		}

		for( int ii = 0; ii < 6; ++ii ){
			std::string id = std::to_string(ii);

			std::string name = "MTAS_336"+id+"_F";
			hismanager->Fill(name,this->Position[ii],this->RawCenter[2*ii]);

			name = "MTAS_336"+id+"_B";
			hismanager->Fill(name,this->Position[ii],this->RawCenter[2*ii + 1]);

			name = "MTAS_336"+id;
			hismanager->Fill(name,this->Position[ii],(this->RawCenter[2*ii] + this->RawCenter[2*ii + 1])/2.0);

			hismanager->Fill("MTAS_3315",this->CrystalEnergy[ii]);
			hismanager->Fill("MTAS_3325",this->CrystalEnergy[ii+6]);
			hismanager->Fill("MTAS_3335",this->CrystalEnergy[ii+12]);
			hismanager->Fill("MTAS_3345",this->CrystalEnergy[ii+18]);

			hismanager->Fill("MTAS_3350",this->TotalEnergy[0],this->CrystalEnergy[ii+6]);
			hismanager->Fill("MTAS_3350",this->TotalEnergy[0],this->CrystalEnergy[ii+12]);
			hismanager->Fill("MTAS_3350",this->TotalEnergy[0],this->CrystalEnergy[ii+18]);

			hismanager->Fill("MTAS_3352",this->TotalEnergy[0],this->CrystalEnergy[ii]);

			hismanager->Fill("MTAS_3353",this->TotalEnergy[1],this->CrystalEnergy[ii]);

			hismanager->Fill("MTAS_33508",this->TotalEnergy[0],this->CrystalEnergy[ii+6]);
			hismanager->Fill("MTAS_33508",this->TotalEnergy[0],this->CrystalEnergy[ii+12]);
			hismanager->Fill("MTAS_33508",this->TotalEnergy[0],this->CrystalEnergy[ii+18]);

			hismanager->Fill("MTAS_33528",this->TotalEnergy[0],this->CrystalEnergy[ii]);

			hismanager->Fill("MTAS_33538",this->TotalEnergy[1],this->CrystalEnergy[ii]);

			if( (not this->InnerFire) and (not this->MiddleFire) and (not this->OuterFire) ){
				hismanager->Fill("MTAS_3354",this->TotalEnergy[0],this->CrystalEnergy[ii]);
				hismanager->Fill("MTAS_33548",this->TotalEnergy[0],this->CrystalEnergy[ii]);
			}


		}
	}
}

void MtasProcessor::FillNoLogicBetaPlots(PLOTS::PlotRegistry* hismanager){
	if( (not this->AnySaturate) and (not this->AnyPileup) ){
		hismanager->Fill("MTAS_4300",this->TotalEnergy[0],this->currevttime*1000.0);
		hismanager->Fill("MTAS_4301",this->TotalEnergy[0],this->currevttime);
		hismanager->Fill("MTAS_4302",this->TotalEnergy[0],this->currevttime/60.0);
		hismanager->Fill("MTAS_4303",this->TotalEnergy[0],this->currevttime/(60.0*60.0));
		hismanager->Fill("MTAS_4304",this->TotalEnergy[0],this->currevttime/(60.0*60.0*24.0));
		hismanager->Fill("MTAS_43008",this->TotalEnergy[0],this->currevttime*1000.0);
		hismanager->Fill("MTAS_43018",this->TotalEnergy[0],this->currevttime);
		hismanager->Fill("MTAS_43028",this->TotalEnergy[0],this->currevttime/60.0);
		hismanager->Fill("MTAS_43038",this->TotalEnergy[0],this->currevttime/(60.0*60.0));
		hismanager->Fill("MTAS_43048",this->TotalEnergy[0],this->currevttime/(60.0*60.0*24.0));
		if( this->diagnosticplots ){
			for( int ii = 0; ii < 6; ++ii ){
				hismanager->Fill("MTAS_3431",this->RawCenter[2*ii],2*ii);
				hismanager->Fill("MTAS_3431",this->RawCenter[2*ii + 1],2*ii + 1);

				hismanager->Fill("MTAS_3432",this->RawInner[2*ii],2*ii);
				hismanager->Fill("MTAS_3432",this->RawInner[2*ii + 1],2*ii + 1);

				hismanager->Fill("MTAS_3433",this->RawMiddle[2*ii],2*ii);
				hismanager->Fill("MTAS_3433",this->RawMiddle[2*ii + 1],2*ii + 1);

				hismanager->Fill("MTAS_3434",this->RawOuter[2*ii],2*ii);
				hismanager->Fill("MTAS_3434",this->RawOuter[2*ii + 1],2*ii + 1);

				hismanager->Fill("MTAS_3531",this->CalCenter[2*ii],2*ii);
				hismanager->Fill("MTAS_3531",this->CalCenter[2*ii + 1],2*ii + 1);

				hismanager->Fill("MTAS_3532",this->CalInner[2*ii],2*ii);
				hismanager->Fill("MTAS_3532",this->CalInner[2*ii + 1],2*ii + 1);

				hismanager->Fill("MTAS_3533",this->CalMiddle[2*ii],2*ii);
				hismanager->Fill("MTAS_3533",this->CalMiddle[2*ii + 1],2*ii + 1);

				hismanager->Fill("MTAS_3534",this->CalOuter[2*ii],2*ii);
				hismanager->Fill("MTAS_3534",this->CalOuter[2*ii + 1],2*ii + 1);
			}
		}
	}
}

void MtasProcessor::FillNonBetaPlots(PLOTS::PlotRegistry* hismanager){
	if( (not this->AnySaturate) and (not this->AnyPileup) ){
		hismanager->Fill("MTAS_3100",this->TotalEnergy[0]);

		hismanager->Fill("MTAS_3110",this->TotalEnergy[1]);
		hismanager->Fill("MTAS_3120",this->TotalEnergy[2]);
		hismanager->Fill("MTAS_3130",this->TotalEnergy[3]);
		hismanager->Fill("MTAS_3140",this->TotalEnergy[4]);

		hismanager->Fill("MTAS_3151",this->TotalEnergy[0],this->TotalEnergy[1]);
		hismanager->Fill("MTAS_31518",this->TotalEnergy[0],this->TotalEnergy[1]);

		for( int ii = 0; ii < 24; ++ii ){
			hismanager->Fill("MTAS_3101",this->CrystalEnergy[ii],ii);
		}

		for( int ii = 0; ii < 6; ++ii ){
			std::string id = std::to_string(ii);

			std::string name = "MTAS_316"+id+"_F";
			hismanager->Fill(name,this->Position[ii],this->RawCenter[2*ii]);

			name = "MTAS_316"+id+"_B";
			hismanager->Fill(name,this->Position[ii],this->RawCenter[2*ii + 1]);

			name = "MTAS_316"+id;
			hismanager->Fill(name,this->Position[ii],(this->RawCenter[2*ii] + this->RawCenter[2*ii + 1])/2.0);

			hismanager->Fill("MTAS_3115",this->CrystalEnergy[ii]);
			hismanager->Fill("MTAS_3125",this->CrystalEnergy[ii+6]);
			hismanager->Fill("MTAS_3135",this->CrystalEnergy[ii+12]);
			hismanager->Fill("MTAS_3145",this->CrystalEnergy[ii+18]);

			hismanager->Fill("MTAS_3150",this->TotalEnergy[0],this->CrystalEnergy[ii+6]);
			hismanager->Fill("MTAS_3150",this->TotalEnergy[0],this->CrystalEnergy[ii+12]);
			hismanager->Fill("MTAS_3150",this->TotalEnergy[0],this->CrystalEnergy[ii+18]);

			hismanager->Fill("MTAS_3152",this->TotalEnergy[0],this->CrystalEnergy[ii]);

			hismanager->Fill("MTAS_3153",this->TotalEnergy[1],this->CrystalEnergy[ii]);

			hismanager->Fill("MTAS_31508",this->TotalEnergy[0],this->CrystalEnergy[ii+6]);
			hismanager->Fill("MTAS_31508",this->TotalEnergy[0],this->CrystalEnergy[ii+12]);
			hismanager->Fill("MTAS_31508",this->TotalEnergy[0],this->CrystalEnergy[ii+18]);

			hismanager->Fill("MTAS_31528",this->TotalEnergy[0],this->CrystalEnergy[ii]);

			hismanager->Fill("MTAS_31538",this->TotalEnergy[1],this->CrystalEnergy[ii]);

			if( (not this->InnerFire) and (not this->MiddleFire) and (not this->OuterFire) ){
				hismanager->Fill("MTAS_3154",this->TotalEnergy[0],this->CrystalEnergy[ii]);
				hismanager->Fill("MTAS_31548",this->TotalEnergy[0],this->CrystalEnergy[ii]);
			}
		}
	}
}

void MtasProcessor::FillNoLogicNonBetaPlots(PLOTS::PlotRegistry* hismanager){
	if( (not this->AnySaturate) and (not this->AnyPileup) ){
		hismanager->Fill("MTAS_4100",this->TotalEnergy[0],this->currevttime*1000.0);
		hismanager->Fill("MTAS_4101",this->TotalEnergy[0],this->currevttime);
		hismanager->Fill("MTAS_4102",this->TotalEnergy[0],this->currevttime/60.0);
		hismanager->Fill("MTAS_4103",this->TotalEnergy[0],this->currevttime/(60.0*60.0));
		hismanager->Fill("MTAS_4104",this->TotalEnergy[0],this->currevttime/(60.0*60.0*24.0));
		hismanager->Fill("MTAS_41008",this->TotalEnergy[0],this->currevttime*1000.0);
		hismanager->Fill("MTAS_41018",this->TotalEnergy[0],this->currevttime);
		hismanager->Fill("MTAS_41028",this->TotalEnergy[0],this->currevttime/60.0);
		hismanager->Fill("MTAS_41038",this->TotalEnergy[0],this->currevttime/(60.0*60.0));
		hismanager->Fill("MTAS_41048",this->TotalEnergy[0],this->currevttime/(60.0*60.0*24.0));


		if( this->diagnosticplots ){
			for( int ii = 0; ii < 6; ++ii ){
				hismanager->Fill("MTAS_3411",this->RawCenter[2*ii],2*ii);
				hismanager->Fill("MTAS_3411",this->RawCenter[2*ii + 1],2*ii + 1);

				hismanager->Fill("MTAS_3412",this->RawInner[2*ii],2*ii);
				hismanager->Fill("MTAS_3412",this->RawInner[2*ii + 1],2*ii + 1);

				hismanager->Fill("MTAS_3413",this->RawMiddle[2*ii],2*ii);
				hismanager->Fill("MTAS_3413",this->RawMiddle[2*ii + 1],2*ii + 1);

				hismanager->Fill("MTAS_3414",this->RawOuter[2*ii],2*ii);
				hismanager->Fill("MTAS_3414",this->RawOuter[2*ii + 1],2*ii + 1);

				hismanager->Fill("MTAS_3511",this->CalCenter[2*ii],2*ii);
				hismanager->Fill("MTAS_3511",this->CalCenter[2*ii + 1],2*ii + 1);

				hismanager->Fill("MTAS_3512",this->CalInner[2*ii],2*ii);
				hismanager->Fill("MTAS_3512",this->CalInner[2*ii + 1],2*ii + 1);

				hismanager->Fill("MTAS_3513",this->CalMiddle[2*ii],2*ii);
				hismanager->Fill("MTAS_3513",this->CalMiddle[2*ii + 1],2*ii + 1);

				hismanager->Fill("MTAS_3514",this->CalOuter[2*ii],2*ii);
				hismanager->Fill("MTAS_3514",this->CalOuter[2*ii + 1],2*ii + 1);
			}
		}
	}
}

double MtasProcessor::CalcPosition(double front,double back){
	return (front - back)/(front + back);
}

const double& MtasProcessor::GetTotalEnergy(const int& idx) const{
	return this->TotalEnergy[idx];
}

const int& MtasProcessor::GetNumPairsFire() const{
	return this->NumFire[0];
}

const int& MtasProcessor::GetNumCenterPairsFire() const{
	return this->NumFire[1];
}

const int& MtasProcessor::GetNumInnerPairsFire() const{
	return this->NumFire[2];
}

const int& MtasProcessor::GetNumMiddlePairsFire() const{
	return this->NumFire[3];
}

const int& MtasProcessor::GetNumOuterPairsFire() const{
	return this->NumFire[4];
}

const bool& MtasProcessor::DidAnyFire() const{
	return this->AnyFire;
}

const bool& MtasProcessor::DidAnyCenterFire() const{
	return this->CenterFire;
}

const bool& MtasProcessor::DidAnyInnerFire() const{
	return this->InnerFire;
}

const bool& MtasProcessor::DidAnyMiddleFire() const{
	return this->MiddleFire;
}

const bool& MtasProcessor::DidAnyOuterFire() const{
	return this->OuterFire;
}

const bool& MtasProcessor::DidAnySaturate() const{
	return this->AnySaturate;
}

const bool& MtasProcessor::DidAnyCenterSaturate() const{
	return this->CenterSaturate;
}

const bool& MtasProcessor::DidAnyInnerSaturate() const{
	return this->InnerSaturate;
}

const bool& MtasProcessor::DidAnyMiddleSaturate() const{
	return this->MiddleSaturate;
}

const bool& MtasProcessor::DidAnyOuterSaturate() const{
	return this->OuterSaturate;
}

const bool& MtasProcessor::DidAnyPileup() const{
	return this->AnyPileup;
}

const bool& MtasProcessor::DidAnyCenterPileup() const{
	return this->CenterPileup;
}

const bool& MtasProcessor::DidAnyInnerPileup() const{
	return this->InnerPileup;
}

const bool& MtasProcessor::DidAnyMiddlePileup() const{
	return this->MiddlePileup;
}

const bool& MtasProcessor::DidAnyOuterPileup() const{
	return this->OuterPileup;
}

const double& MtasProcessor::GetCrystalEnergy(const int& idx) const{
	return this->CrystalEnergy[idx];
}

bool MtasProcessor::DidIndividualPMTSaturate(const int& idx) const{
	return this->IndividualPMTSaturate[idx];
}

bool MtasProcessor::DidIndividualPMTPileup(const int& idx) const{
	return this->IndividualPMTPileup[idx];
}

const double& MtasProcessor::GetFirstFireTime() const{
	return this->FirstTime;
}

const double& MtasProcessor::GetLastFireTime() const{
	return this->LastTime;
}
