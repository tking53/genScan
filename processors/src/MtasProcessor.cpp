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

		//rate
		{ 3170, {65536,0,65536} },
		{ 3171, {65536,0,65536} },
		{ 3172, {65536,0,65536} },

		{ 3200, {16384,0,16384} },
		{ 3210, {16384,0,16384} },
		{ 3215, {16384,0,16384} },
		{ 3220, {16384,0,16384} },
		{ 3225, {16384,0,16384} },
		{ 3230, {16384,0,16384} },
		{ 3235, {16384,0,16384} },
		{ 3240, {16384,0,16384} },
		{ 3245, {16384,0,16384} },

		//rate
		{ 3270, {65536,0,65536} },
		{ 3271, {65536,0,65536} },
		{ 3272, {65536,0,65536} },

		{ 3300, {16384,0,16384} },
		{ 3310, {16384,0,16384} },
		{ 3315, {16384,0,16384} },
		{ 3320, {16384,0,16384} },
		{ 3325, {16384,0,16384} },
		{ 3330, {16384,0,16384} },
		{ 3335, {16384,0,16384} },
		{ 3340, {16384,0,16384} },
		{ 3345, {16384,0,16384} },

		//rate
		{ 3370, {65536,0,65536} },
		{ 3371, {65536,0,65536} },
		{ 3372, {65536,0,65536} }
	};

	this->h2dsettings = {
		{3101, {16384,0.0,16384,24,0,24}},
		{3102, {16384,0.0,16384,48,0,48}},
		{3103, {16384,0.0,16384,48,0,48}},
		{3150, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3151, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3152, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3153, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3154, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3155, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3156, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3157, {4096,0.0,4096.0,4096,0.0,4096.0}},

		{3160, {1024,-1.0,1.0,8192,0.0,8192.0}},

		{31508, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{31518, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{31528, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{31538, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{31548, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{31558, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{31568, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{31578, {2048,0.0,16384.0,2048,0.0,16384.0}},

		{3201, {16384,0.0,16384,24,0,24}},
		{3202, {16384,0.0,16384,48,0,48}},
		{3203, {16384,0.0,16384,48,0,48}},
		{3250, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3251, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3252, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3253, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3254, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3255, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3256, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3257, {4096,0.0,4096.0,4096,0.0,4096.0}},

		{3260, {1024,-1.0,1.0,8192,0.0,8192.0}},

		{32508, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{32518, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{32528, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{32538, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{32548, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{32558, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{32568, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{32578, {2048,0.0,16384.0,2048,0.0,16384.0}},

		{3301, {16384,0.0,16384,24,0,24}},
		{3302, {16384,0.0,16384,48,0,48}},
		{3303, {16384,0.0,16384,48,0,48}},
		{3350, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3351, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3352, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3353, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3354, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3355, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3356, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{3357, {4096,0.0,4096.0,4096,0.0,4096.0}},

		{3360, {1024,-1.0,1.0,8192,0.0,8192.0}},

		{33508, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{33518, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{33528, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{33538, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{33548, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{33558, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{33568, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{33578, {2048,0.0,16384.0,2048,0.0,16384.0}},

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
		{4110, {8192,0.0,8192.0,1024,0.0,1024}},
		{4111, {8192,0.0,8192.0,1024,0.0,1024}},
		{4112, {8192,0.0,8192.0,1024,0.0,1024}},
		{4120, {8192,0.0,8192.0,1024,0.0,1024}},
		{4121, {8192,0.0,8192.0,1024,0.0,1024}},
		{4122, {8192,0.0,8192.0,1024,0.0,1024}},
		{4130, {8192,0.0,8192.0,1024,0.0,1024}},
		{4131, {8192,0.0,8192.0,1024,0.0,1024}},
		{4132, {8192,0.0,8192.0,1024,0.0,1024}},
		{4140, {8192,0.0,8192.0,1024,0.0,1024}},
		{4141, {8192,0.0,8192.0,1024,0.0,1024}},
		{4142, {8192,0.0,8192.0,1024,0.0,1024}},
		{41008, {2048,0.0,16384.0,1024,0.0,1024}},
		{41018, {2048,0.0,16384.0,1024,0.0,1024}},
		{41028, {2048,0.0,16384.0,1024,0.0,1024}},

		{4200, {8192,0.0,8192.0,1024,0.0,1024}},
		{4201, {8192,0.0,8192.0,1024,0.0,1024}},
		{4202, {8192,0.0,8192.0,1024,0.0,1024}},
		{4210, {8192,0.0,8192.0,1024,0.0,1024}},
		{4211, {8192,0.0,8192.0,1024,0.0,1024}},
		{4212, {8192,0.0,8192.0,1024,0.0,1024}},
		{4220, {8192,0.0,8192.0,1024,0.0,1024}},
		{4221, {8192,0.0,8192.0,1024,0.0,1024}},
		{4222, {8192,0.0,8192.0,1024,0.0,1024}},
		{4230, {8192,0.0,8192.0,1024,0.0,1024}},
		{4231, {8192,0.0,8192.0,1024,0.0,1024}},
		{4232, {8192,0.0,8192.0,1024,0.0,1024}},
		{4240, {8192,0.0,8192.0,1024,0.0,1024}},
		{4241, {8192,0.0,8192.0,1024,0.0,1024}},
		{4242, {8192,0.0,8192.0,1024,0.0,1024}},
		{42008, {2048,0.0,16384.0,1024,0.0,1024}},
		{42018, {2048,0.0,16384.0,1024,0.0,1024}},
		{42028, {2048,0.0,16384.0,1024,0.0,1024}},

		{4300, {8192,0.0,8192.0,1024,0.0,1024}},
		{4301, {8192,0.0,8192.0,1024,0.0,1024}},
		{4302, {8192,0.0,8192.0,1024,0.0,1024}},
		{4310, {8192,0.0,8192.0,1024,0.0,1024}},
		{4311, {8192,0.0,8192.0,1024,0.0,1024}},
		{4312, {8192,0.0,8192.0,1024,0.0,1024}},
		{4320, {8192,0.0,8192.0,1024,0.0,1024}},
		{4321, {8192,0.0,8192.0,1024,0.0,1024}},
		{4322, {8192,0.0,8192.0,1024,0.0,1024}},
		{4330, {8192,0.0,8192.0,1024,0.0,1024}},
		{4331, {8192,0.0,8192.0,1024,0.0,1024}},
		{4332, {8192,0.0,8192.0,1024,0.0,1024}},
		{4340, {8192,0.0,8192.0,1024,0.0,1024}},
		{4341, {8192,0.0,8192.0,1024,0.0,1024}},
		{4342, {8192,0.0,8192.0,1024,0.0,1024}},
		{43008, {2048,0.0,16384.0,1024,0.0,1024}},
		{43018, {2048,0.0,16384.0,1024,0.0,1024}},
		{43028, {2048,0.0,16384.0,1024,0.0,1024}},

		//Gamma - Gamma Matrix
		{5100, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{5101, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{5102, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{5200, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{5201, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{5202, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{5300, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{5301, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{5302, {4096,0.0,4096.0,4096,0.0,4096.0}},
		{51008, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{51018, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{51028, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{52008, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{52018, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{52028, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{53008, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{53018, {2048,0.0,16384.0,2048,0.0,16384.0}},
		{53028, {2048,0.0,16384.0,2048,0.0,16384.0}},

		//Center numfire vs energy before applying zeroing logic
		{5510, {16384,0.0,16384.0,12,1,13}},
		{5520, {16384,0.0,16384.0,12,1,13}},
		{5530, {16384,0.0,16384.0,12,1,13}}
	};

	this->Position = std::vector<double>(24,0.0);
	this->Center = std::vector<double>(12,0.0);
	this->Inner = std::vector<double>(12,0.0);
	this->Middle = std::vector<double>(12,0.0);
	this->Outer = std::vector<double>(12,0.0);

	this->diagnosticplots = false;
	this->logictimeplots = false;
	this->gammagammaplots = false;

	this->RawCenter = std::vector<double>(12,0.0);
	this->RawInner = std::vector<double>(12,0.0);
	this->RawMiddle = std::vector<double>(12,0.0);
	this->RawOuter = std::vector<double>(12,0.0);

	this->CalCenter = std::vector<double>(12,0.0);
	this->CalInner = std::vector<double>(12,0.0);
	this->CalMiddle = std::vector<double>(12,0.0);
	this->CalOuter = std::vector<double>(12,0.0);

	this->CenterHits = std::vector<int>(12,0);
	this->ValidCenterSegments = std::vector<bool>(6,false);
	this->InnerHits = std::vector<int>(12,0);
	this->MiddleHits = std::vector<int>(12,0);
	this->OuterHits = std::vector<int>(12,0);

	this->CrystalEnergy = std::vector<double>(24,0.0);
	//this seems weird, but for compliance with the old center and new center,
	//we will store the old center calculation before zeroing in [5] along with it's 
	//numfire, so we can use it later after we have zeroed it. either way
	//[1] will still store the total like normal and is the value that 
	//should be used by users
	this->TotalEnergy = std::vector<double>(6,0.0);

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

	this->NumFire = std::vector<int>(6,0);
	this->CenterFire = false;
	this->InnerFire = false;
	this->MiddleFire = false;
	this->OuterFire = false;
	this->AnyFire = false;
	
	this->Back2BackCenterFire = false;
	this->Back2BackCenter511RegionFire = false;

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

	//this->SetEventIdx(eventhistory->GetEventCount());
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
		this->OldCenterCalculation(hismanager);
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
		hismanager->Fill("MTAS_3270",this->currevttime);
		hismanager->Fill("MTAS_3271",this->currevttime/60.0);
		hismanager->Fill("MTAS_3272",this->currevttime/(60.0*60.0));
		for( int ii = 0; ii < 6; ++ii ){
			auto currhx = this->HexagonShapes[ii].center;
			if( this->CenterHits[2*ii] ){
				for( size_t jj = 0; jj < this->CenterHits[2*ii]; ++jj ){
					this->MTAS_2501->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2503->Fill(currhx.first,currhx.second,1.0);
				}
			}
			if( this->CenterHits[2*ii+1] ){
				for( size_t jj = 0; jj < this->CenterHits[2*ii+1]; ++jj ){
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
				for( size_t jj = 0; jj < this->InnerHits[2*ii+1]; ++jj ){
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
				for( size_t jj = 0; jj < this->MiddleHits[2*ii+1]; ++jj ){
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
				for( size_t jj = 0; jj < this->OuterHits[2*ii+1]; ++jj ){
					this->MTAS_2502->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2503->Fill(currhx.first,currhx.second,-1.0);
				}
			}
			if( this->OuterHits[2*ii] and this->OuterHits[2*ii+1] ){
				this->MTAS_2500->Fill(currhx.first,currhx.second,1.0);
			}
		}

		if( this->logictimeplots ){
			hismanager->Fill("MTAS_4200",this->TotalEnergy[0],this->currevttime);
			hismanager->Fill("MTAS_4201",this->TotalEnergy[0],this->currevttime/60.0);
			hismanager->Fill("MTAS_4202",this->TotalEnergy[0],this->currevttime/(60.0*60.0));

			hismanager->Fill("MTAS_4210",this->TotalEnergy[1],this->currevttime);
			hismanager->Fill("MTAS_4211",this->TotalEnergy[1],this->currevttime/60.0);
			hismanager->Fill("MTAS_4212",this->TotalEnergy[1],this->currevttime/(60.0*60.0));

			hismanager->Fill("MTAS_4220",this->TotalEnergy[2],this->currevttime);
			hismanager->Fill("MTAS_4221",this->TotalEnergy[2],this->currevttime/60.0);
			hismanager->Fill("MTAS_4222",this->TotalEnergy[2],this->currevttime/(60.0*60.0));

			hismanager->Fill("MTAS_4230",this->TotalEnergy[3],this->currevttime);
			hismanager->Fill("MTAS_4231",this->TotalEnergy[3],this->currevttime/60.0);
			hismanager->Fill("MTAS_4232",this->TotalEnergy[3],this->currevttime/(60.0*60.0));

			hismanager->Fill("MTAS_4240",this->TotalEnergy[4],this->currevttime);
			hismanager->Fill("MTAS_4241",this->TotalEnergy[4],this->currevttime/60.0);
			hismanager->Fill("MTAS_4242",this->TotalEnergy[4],this->currevttime/(60.0*60.0));

			hismanager->Fill("MTAS_42008",this->TotalEnergy[0],this->currevttime);
			hismanager->Fill("MTAS_42018",this->TotalEnergy[0],this->currevttime/60.0);
			hismanager->Fill("MTAS_42028",this->TotalEnergy[0],this->currevttime/(60.0*60.0));
		}

		hismanager->Fill("MTAS_3200",this->TotalEnergy[0]);

		hismanager->Fill("MTAS_3210",this->TotalEnergy[1]);
		hismanager->Fill("MTAS_3220",this->TotalEnergy[2]);
		hismanager->Fill("MTAS_3230",this->TotalEnergy[3]);
		hismanager->Fill("MTAS_3240",this->TotalEnergy[4]);

		hismanager->Fill("MTAS_3251",this->TotalEnergy[0],this->TotalEnergy[1]);
		hismanager->Fill("MTAS_32518",this->TotalEnergy[0],this->TotalEnergy[1]);
		
		if( (not this->MiddleFire) and (not this->OuterFire) ){
			hismanager->Fill("MTAS_3257",this->TotalEnergy[0],this->TotalEnergy[1]);
			hismanager->Fill("MTAS_32578",this->TotalEnergy[0],this->TotalEnergy[1]);
			if( not this->InnerFire ){
				hismanager->Fill("MTAS_3256",this->TotalEnergy[0],this->TotalEnergy[1]);
				hismanager->Fill("MTAS_32568",this->TotalEnergy[0],this->TotalEnergy[1]);
			}
		}

		//gamma-gamma matrices
		if( this->gammagammaplots ){
			//if this is included it only drags it up by 40 seconds
			//prefetch these histograms which are filled a combined total of 4608 times
			//every single PreProcess call, could get rid of the 5200 since it's the sum 
			//of 5201 and 5202, but for laziness having it in there is nice to draw immediately
			auto MTAS_5200  = hismanager->GetPlot<TH2*>("MTAS_5200");
			auto MTAS_5201  = hismanager->GetPlot<TH2*>("MTAS_5201");
			auto MTAS_5202  = hismanager->GetPlot<TH2*>("MTAS_5202");
			auto MTAS_52008 = hismanager->GetPlot<TH2*>("MTAS_52008");
			auto MTAS_52018 = hismanager->GetPlot<TH2*>("MTAS_52018");
			auto MTAS_52028 = hismanager->GetPlot<TH2*>("MTAS_52028");
			for( int ii = 0; ii < 6; ++ii ){
				for( int jj = ii+1; jj < 6; ++jj ){
					MTAS_5201->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_5201->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_5202->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_5202->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_52018->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_52018->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_52028->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_52028->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
				}	
			}
			for( int ii = 6; ii < 24; ++ii ){
				for( int jj = ii+1; jj < 24; ++jj ){
					MTAS_5200->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_5200->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_5202->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_5202->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_52008->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_52008->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_52028->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_52028->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
				}
			}
		}

		//prefetch these histograms because grabbing them every single loop is expensive
		auto MTAS_3201 = hismanager->GetPlot<TH2*>("MTAS_3201");
		for( int ii = 0; ii < 24; ++ii ){
			MTAS_3201->Fill(this->CrystalEnergy[ii],ii);
		}

		//prefetch these histograms because grabbing them every single loop is expensive
		auto MTAS_3202 = hismanager->GetPlot<TH2*>("MTAS_3202");
		auto MTAS_3203 = hismanager->GetPlot<TH2*>("MTAS_3203");
		for( int ii = 0; ii < 12; ++ii ){
			MTAS_3202->Fill(this->RawCenter[ii],ii);
			MTAS_3202->Fill(this->RawInner[ii],ii+12);
			MTAS_3202->Fill(this->RawMiddle[ii],ii+24);
			MTAS_3202->Fill(this->RawOuter[ii],ii+36);

			MTAS_3203->Fill(this->Center[ii],ii);
			MTAS_3203->Fill(this->Inner[ii],ii+12);
			MTAS_3203->Fill(this->Middle[ii],ii+24);
			MTAS_3203->Fill(this->Outer[ii],ii+36);
		}

		//prefetch these histograms because grabbing them every single loop is expensive
		auto MTAS_3250  = hismanager->GetPlot<TH2*>("MTAS_3250");
		auto MTAS_32508 = hismanager->GetPlot<TH2*>("MTAS_32508");
		auto MTAS_3215  = hismanager->GetPlot<TH1*>("MTAS_3215");
		auto MTAS_3225  = hismanager->GetPlot<TH1*>("MTAS_3225");
		auto MTAS_3235  = hismanager->GetPlot<TH1*>("MTAS_3235");
		auto MTAS_3245  = hismanager->GetPlot<TH1*>("MTAS_3245");
		auto MTAS_3252  = hismanager->GetPlot<TH2*>("MTAS_3252");
		auto MTAS_32528 = hismanager->GetPlot<TH2*>("MTAS_32528");
		auto MTAS_3253  = hismanager->GetPlot<TH2*>("MTAS_3253");
		auto MTAS_32538 = hismanager->GetPlot<TH2*>("MTAS_32538");
		auto MTAS_3254  = hismanager->GetPlot<TH2*>("MTAS_3254");
		auto MTAS_32548 = hismanager->GetPlot<TH2*>("MTAS_32548");
		auto MTAS_3255  = hismanager->GetPlot<TH2*>("MTAS_3255");
		auto MTAS_32558 = hismanager->GetPlot<TH2*>("MTAS_32558");
		for( int ii = 0; ii < 6; ++ii ){
			std::string id = std::to_string(ii);

			std::string name = "MTAS_326"+id+"_F";
			hismanager->Fill(name,this->Position[ii],this->RawCenter[2*ii]);

			name = "MTAS_326"+id+"_B";
			hismanager->Fill(name,this->Position[ii],this->RawCenter[2*ii + 1]);

			name = "MTAS_326"+id;
			hismanager->Fill(name,this->Position[ii],(this->RawCenter[2*ii] + this->RawCenter[2*ii + 1])/2.0);

			MTAS_3215->Fill(this->CrystalEnergy[ii]);
			MTAS_3225->Fill(this->CrystalEnergy[ii+6]);
			MTAS_3235->Fill(this->CrystalEnergy[ii+12]);
			MTAS_3245->Fill(this->CrystalEnergy[ii+18]);

			MTAS_3250->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+6]);
			MTAS_3250->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+12]);
			MTAS_3250->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+18]);

			MTAS_3252->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);
			MTAS_3253->Fill(this->TotalEnergy[1],this->CrystalEnergy[ii]);

			MTAS_32508->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+6]);
			MTAS_32508->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+12]);
			MTAS_32508->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+18]);

			MTAS_32528->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);
			MTAS_32538->Fill(this->TotalEnergy[1],this->CrystalEnergy[ii]);

			if( (not this->MiddleFire) and (not this->OuterFire) ){
				MTAS_3255->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);
				MTAS_32558->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);
				if( not this->InnerFire ){
					MTAS_3254->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);
					MTAS_32548->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);
				}
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
	Processor::Process();

	//this allows for all the beta and non-beta to be filled when no exp is defined
	//but if we're in exp mode (i.e.) one known processor then we will not fill
	//this could double fill if you define an exp that has MTAS as well as 
	//mtas outside as well. hopefull nobody will be that dumb, likely 
	//we need to recursively ask all the processors to name themselves and we double check 
	//that there are no duplicates present
	if( not this->ExpProcessorMode ){
		//we're in a cluster of procs and need to query beta or not beta ourselves
		auto summary = eventhistory->GetCurrentEventSummary();
		auto HasBeta = summary->ContainsEventTag("beta");
		if( HasBeta ){
			this->FillBetaPlots(hismanager);
			this->FillNoLogicBetaPlots(hismanager);
		}else{
			this->FillNonBetaPlots(hismanager);
			this->FillNoLogicNonBetaPlots(hismanager);
		}
	}

	Processor::EndProcess();
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
			this->ValidCenterSegments[ii] = true;
		}
	}
	//this check is not straightforward, since compton scatters will confuse this quite easily
	//however the 511 gate, should help clean things easier
	for( size_t ii = 0; ii < 3; ++ii ){
		if( this->ValidCenterSegments[ii] && this->ValidCenterSegments[ii+2] ){
			this->Back2BackCenterFire = true;
			if( this->Center511Region.IsWithin(this->CrystalEnergy[ii]) && this->Center511Region.IsWithin(this->CrystalEnergy[ii+2]) ){
				this->Back2BackCenter511RegionFire = true;
			}
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

void MtasProcessor::OldCenterCalculation(PLOTS::PlotRegistry* hismanager){
	//this is the super old method but it allows us to check if we need to lower the requirement
	//of how many centers are needed to fire to call it a valid center event
	for( int ii = 0; ii < 12; ++ii ){
		if( this->CenterHits[ii] ){
			this->NumFire[5] += 1;
			this->TotalEnergy[5] += this->Center[ii];
		}
	}
	this->TotalEnergy[5] /= this->NumFire[5];

	//likely need to change the constraint below based on this plot
	hismanager->Fill("MTAS_5520",this->TotalEnergy[5],this->NumFire[5]);
	if( this->NumFire[5] != 12 ){
		this->TotalEnergy[1] = 0.0;
	}else{
		this->TotalEnergy[1] = this->TotalEnergy[5];
		this->TotalEnergy[0] += this->TotalEnergy[1];
		this->CenterFire = true;
		this->AnyFire = true;
	}

	for( int ii = 0; ii < 6; ++ii ){
		if( this->CenterHits[2*ii] and this->CenterHits[2*ii + 1] ){
			this->CrystalEnergy[ii] = (this->Center[2*ii] + this->Center[2*ii + 1])/2.0;
			this->SegmentDataVec[ii].sumenergy = this->CrystalEnergy[ii];
			this->SegmentDataVec[ii].avgtimestamp = (this->SegmentDataVec[ii].fronttimestamp+this->SegmentDataVec[ii].backtimestamp)/2.0;
			this->Position[ii] = this->CalcPosition(this->RawCenter[2*ii],this->RawCenter[2*ii + 1]);
			this->NumFire[0] += 1;
			this->NumFire[1] += 1;
		}
	}

	for( int ii = 0; ii < 6; ++ii ){
		this->TotalEnergy[0] += this->CrystalEnergy[ii+6];
		this->TotalEnergy[0] += this->CrystalEnergy[ii+12];
		this->TotalEnergy[0] += this->CrystalEnergy[ii+18];

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

	//let's default this before we try to set it
	this->Center511Region = Gate<double>(446.0,540.0);
	for( pugi::xml_node gate = config.child("Gate"); gate; gate = gate.next_sibling("Gate") ){
		std::string label = gate.attribute("label").as_string("");
		if( label.compare("Center511") == 0 ){
			//these are the values seen from Zr90
			auto low = gate.attribute("lowerbound").as_double(446.0);
			auto high = gate.attribute("upperbound").as_double(540.0);
			this->Center511Region = Gate<double>(low,high);
		}
	}


	this->diagnosticplots = config.attribute("diagnostic").as_bool(false);
	this->logictimeplots = config.attribute("logic").as_bool(false);
	this->gammagammaplots = config.attribute("gamma-gamma").as_bool(false);
	this->UseOldCenter = config.attribute("oldcenter").as_bool(false);

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void MtasProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
	if( not this->ExpProcessorMode ){
		this->console->critical("Experiment Processor Mode is disabled, MtasProcessor itself will try and fill Beta/NonBeta");
	}
}

void MtasProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager){
	this->MTAS_2400 = hismanager->RegisterPlot("MTAS_2400","anti-#beta gated MTAS Segment Hit Map Beam is out of the page");
	for( const auto& hx : this->HexagonShapes ){
		this->MTAS_2400->AddBin(6,hx.xcoords,hx.ycoords);
	}
	this->MTAS_2401 = hismanager->RegisterPlot("MTAS_2401","anti-#beta gated MTAS Front Hit Map Beam is out of the page");
	for( const auto& hx : this->HexagonShapes ){
		this->MTAS_2401->AddBin(6,hx.xcoords,hx.ycoords);
	}
	this->MTAS_2402 = hismanager->RegisterPlot("MTAS_2402","anti-#beta gated MTAS Back Hit Map Beam is out of the page");
	for( const auto& hx : this->HexagonShapes ){
		this->MTAS_2402->AddBin(6,hx.xcoords,hx.ycoords);
	}
	this->MTAS_2403 = hismanager->RegisterPlot("MTAS_2403","anti-#beta gated MTAS Asymmetry Hit Map Beam is out of the page, Front is positive, Back is negative");
	for( const auto& hx : this->HexagonShapes ){
		this->MTAS_2403->AddBin(6,hx.xcoords,hx.ycoords);
	}

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

	this->MTAS_2600 = hismanager->RegisterPlot("MTAS_2600","#beta gated MTAS Segment Hit Map Beam is out of the page");
	for( const auto& hx : this->HexagonShapes ){
		this->MTAS_2600->AddBin(6,hx.xcoords,hx.ycoords);
	}
	this->MTAS_2601 = hismanager->RegisterPlot("MTAS_2601","#beta gated MTAS Front Hit Map Beam is out of the page");
	for( const auto& hx : this->HexagonShapes ){
		this->MTAS_2601->AddBin(6,hx.xcoords,hx.ycoords);
	}
	this->MTAS_2602 = hismanager->RegisterPlot("MTAS_2602","#beta gated MTAS Back Hit Map Beam is out of the page");
	for( const auto& hx : this->HexagonShapes ){
		this->MTAS_2602->AddBin(6,hx.xcoords,hx.ycoords);
	}
	this->MTAS_2603 = hismanager->RegisterPlot("MTAS_2603","#beta gated MTAS Asymmetry Hit Map Beam is out of the page, Front is positive, Back is negative");
	for( const auto& hx : this->HexagonShapes ){
		this->MTAS_2603->AddBin(6,hx.xcoords,hx.ycoords);
	}

	//MTAS diagnostic plots, always want these no matter what
	hismanager->RegisterPlot<TH1F>("MTAS_3270","Mtas Total Scalar Rate (s); Time (s)",this->h1dsettings.at(3270));
	hismanager->RegisterPlot<TH1F>("MTAS_3271","Mtas Total Scalar Rate (min); Time (min)",this->h1dsettings.at(3271));
	hismanager->RegisterPlot<TH1F>("MTAS_3272","Mtas Total Scalar Rate (hr); Time (hr)",this->h1dsettings.at(3272));

	hismanager->RegisterPlot<TH1F>("MTAS_3200","Mtas Total; Energy (keV)",this->h1dsettings.at(3200));
	hismanager->RegisterPlot<TH2F>("MTAS_3201","Sum F+B; Energy (keV); F+B Pair (arb.)",this->h2dsettings.at(3201));
	hismanager->RegisterPlot<TH2F>("MTAS_3202","Raw Individual PMT; Channel (arb.); PMT Number (arb.)",this->h2dsettings.at(3202));
	hismanager->RegisterPlot<TH2F>("MTAS_3203","Cal Individual PMT; Energy (keV); PMT Number (arb.)",this->h2dsettings.at(3203));

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
	hismanager->RegisterPlot<TH2F>("MTAS_3255","C Segment vs Mtas Total (veto any M,O) ; Energy (keV); Energy (keV)",this->h2dsettings.at(3255));
	hismanager->RegisterPlot<TH2F>("MTAS_3256","C vs Mtas Total (veto any I,M,O) ; Energy (keV); Energy (keV)",this->h2dsettings.at(3256));
	hismanager->RegisterPlot<TH2F>("MTAS_3257","C vs Mtas Total (veto any M,O) ; Energy (keV); Energy (keV)",this->h2dsettings.at(3257));

	hismanager->RegisterPlot<TH2F>("MTAS_32508","I,M,O vs Mtas Total; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(32508));
	hismanager->RegisterPlot<TH2F>("MTAS_32518","C vs Mtas Total; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(32518));
	hismanager->RegisterPlot<TH2F>("MTAS_32528","C Segment vs Mtas Total; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(32528));
	hismanager->RegisterPlot<TH2F>("MTAS_32538","C Segment vs C; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(32538));
	hismanager->RegisterPlot<TH2F>("MTAS_32548","C Segment vs Mtas Total (veto any I,M,O) ; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(32548));
	hismanager->RegisterPlot<TH2F>("MTAS_32558","C Segment vs Mtas Total (veto any M,O) ; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(32558));
	hismanager->RegisterPlot<TH2F>("MTAS_32568","C Segment vs Mtas Total (veto any I,M,O) ; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(32568));
	hismanager->RegisterPlot<TH2F>("MTAS_32578","C Segment vs Mtas Total (veto any M,O) ; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(32578));

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

	if( this->gammagammaplots ){
		hismanager->RegisterPlot<TH2F>("MTAS_5200","I,M,O, Gamma-Gamma Matrix; Energy (keV); Energy (keV)",this->h2dsettings.at(5200));
		hismanager->RegisterPlot<TH2F>("MTAS_5201","C Gamma-Gamma Matrix; Energy (keV); Energy (keV)",this->h2dsettings.at(5201));
		hismanager->RegisterPlot<TH2F>("MTAS_5202","C,I,M,O, Gamma-Gamma Matrix; Energy (keV); Energy (keV)",this->h2dsettings.at(5202));
		hismanager->RegisterPlot<TH2F>("MTAS_52008","I,M,O, Gamma-Gamma Matrix; Energy (keV); Energy (keV)",this->h2dsettings.at(52008));
		hismanager->RegisterPlot<TH2F>("MTAS_52018","C Gamma-Gamma Matrix; Energy (keV); Energy (keV)",this->h2dsettings.at(52018));
		hismanager->RegisterPlot<TH2F>("MTAS_52028","C,I,M,O, Gamma-Gamma Matrix; Energy (keV); Energy (keV)",this->h2dsettings.at(52028));
	}
	
	if( this->UseOldCenter ){
		hismanager->RegisterPlot<TH2F>("MTAS_5520","Num Center PMT Fire vs Center Sum Before Zero; Energy (keV); count (arb.)",this->h2dsettings.at(5520));
	}

	//declare the beta gated and not-beta histograms, but we don't fill them until parent processor has told which we are
	this->DeclareBetaPlots(hismanager);
	this->DeclareAntiBetaPlots(hismanager);
	if( this->logictimeplots ){
		this->DeclareNoLogicPlots(hismanager);
	}

	this->console->info("Finished Declaring Plots");
}

void MtasProcessor::DeclareNoLogicPlots(PLOTS::PlotRegistry* hismanager){
	hismanager->RegisterPlot<TH2F>("MTAS_4200","Run Time vs Mtas Total; Energy (keV); Run Time (s)",this->h2dsettings.at(4200));
	hismanager->RegisterPlot<TH2F>("MTAS_4201","Run Time vs Mtas Total; Energy (keV); Run Time (min)",this->h2dsettings.at(4201));
	hismanager->RegisterPlot<TH2F>("MTAS_4202","Run Time vs Mtas Total; Energy (keV); Run Time (hr)",this->h2dsettings.at(4202));

	hismanager->RegisterPlot<TH2F>("MTAS_4210","Run Time vs Mtas Center Sum; Energy (keV); Run Time (s)",this->h2dsettings.at(4210));
	hismanager->RegisterPlot<TH2F>("MTAS_4211","Run Time vs Mtas Center Sum; Energy (keV); Run Time (min)",this->h2dsettings.at(4211));
	hismanager->RegisterPlot<TH2F>("MTAS_4212","Run Time vs Mtas Center Sum; Energy (keV); Run Time (hr)",this->h2dsettings.at(4212));
	
	hismanager->RegisterPlot<TH2F>("MTAS_4220","Run Time vs Mtas Inner Sum; Energy (keV); Run Time (s)",this->h2dsettings.at(4220));
	hismanager->RegisterPlot<TH2F>("MTAS_4221","Run Time vs Mtas Inner Sum; Energy (keV); Run Time (min)",this->h2dsettings.at(4221));
	hismanager->RegisterPlot<TH2F>("MTAS_4222","Run Time vs Mtas Inner Sum; Energy (keV); Run Time (hr)",this->h2dsettings.at(4222));
	
	hismanager->RegisterPlot<TH2F>("MTAS_4230","Run Time vs Mtas Middle Sum; Energy (keV); Run Time (s)",this->h2dsettings.at(4230));
	hismanager->RegisterPlot<TH2F>("MTAS_4231","Run Time vs Mtas Middle Sum; Energy (keV); Run Time (min)",this->h2dsettings.at(4231));
	hismanager->RegisterPlot<TH2F>("MTAS_4232","Run Time vs Mtas Middle Sum; Energy (keV); Run Time (hr)",this->h2dsettings.at(4232));
	
	hismanager->RegisterPlot<TH2F>("MTAS_4240","Run Time vs Mtas Outer Sum; Energy (keV); Run Time (s)",this->h2dsettings.at(4240));
	hismanager->RegisterPlot<TH2F>("MTAS_4241","Run Time vs Mtas Outer Sum; Energy (keV); Run Time (min)",this->h2dsettings.at(4241));
	hismanager->RegisterPlot<TH2F>("MTAS_4242","Run Time vs Mtas Outer Sum; Energy (keV); Run Time (hr)",this->h2dsettings.at(4242));
	
	hismanager->RegisterPlot<TH2F>("MTAS_42008","Run Time vs Mtas Total; Energy (8 keV/bin); Run Time (s)",this->h2dsettings.at(42008));
	hismanager->RegisterPlot<TH2F>("MTAS_42018","Run Time vs Mtas Total; Energy (8 keV/bin); Run Time (min)",this->h2dsettings.at(42018));
	hismanager->RegisterPlot<TH2F>("MTAS_42028","Run Time vs Mtas Total; Energy (8 keV/bin); Run Time (hr)",this->h2dsettings.at(42028));

	this->DeclareNoLogicBetaPlots(hismanager);
	this->DeclareNoLogicAntiBetaPlots(hismanager);

}

void MtasProcessor::DeclareNoLogicBetaPlots(PLOTS::PlotRegistry* hismanager){
	hismanager->RegisterPlot<TH2F>("MTAS_4300","Run Time vs Mtas Total #beta-gated; Energy (keV); Run Time (s)",this->h2dsettings.at(4300));
	hismanager->RegisterPlot<TH2F>("MTAS_4301","Run Time vs Mtas Total #beta-gated; Energy (keV); Run Time (min)",this->h2dsettings.at(4301));
	hismanager->RegisterPlot<TH2F>("MTAS_4302","Run Time vs Mtas Total #beta-gated; Energy (keV); Run Time (hr)",this->h2dsettings.at(4302));
	
	hismanager->RegisterPlot<TH2F>("MTAS_4310","Run Time vs Mtas Center Sum #beta-gated; Energy (keV); Run Time (s)",this->h2dsettings.at(4310));
	hismanager->RegisterPlot<TH2F>("MTAS_4311","Run Time vs Mtas Center Sum #beta-gated; Energy (keV); Run Time (min)",this->h2dsettings.at(4311));
	hismanager->RegisterPlot<TH2F>("MTAS_4312","Run Time vs Mtas Center Sum #beta-gated; Energy (keV); Run Time (hr)",this->h2dsettings.at(4312));
	
	hismanager->RegisterPlot<TH2F>("MTAS_4320","Run Time vs Mtas Inner Sum #beta-gated; Energy (keV); Run Time (s)",this->h2dsettings.at(4320));
	hismanager->RegisterPlot<TH2F>("MTAS_4321","Run Time vs Mtas Inner Sum #beta-gated; Energy (keV); Run Time (min)",this->h2dsettings.at(4321));
	hismanager->RegisterPlot<TH2F>("MTAS_4322","Run Time vs Mtas Inner Sum #beta-gated; Energy (keV); Run Time (hr)",this->h2dsettings.at(4322));
	
	hismanager->RegisterPlot<TH2F>("MTAS_4330","Run Time vs Mtas Middle Sum #beta-gated; Energy (keV); Run Time (s)",this->h2dsettings.at(4330));
	hismanager->RegisterPlot<TH2F>("MTAS_4331","Run Time vs Mtas Middle Sum #beta-gated; Energy (keV); Run Time (min)",this->h2dsettings.at(4331));
	hismanager->RegisterPlot<TH2F>("MTAS_4332","Run Time vs Mtas Middle Sum #beta-gated; Energy (keV); Run Time (hr)",this->h2dsettings.at(4332));
	
	hismanager->RegisterPlot<TH2F>("MTAS_4340","Run Time vs Mtas Outer Sum #beta-gated; Energy (keV); Run Time (s)",this->h2dsettings.at(4340));
	hismanager->RegisterPlot<TH2F>("MTAS_4341","Run Time vs Mtas Outer Sum #beta-gated; Energy (keV); Run Time (min)",this->h2dsettings.at(4341));
	hismanager->RegisterPlot<TH2F>("MTAS_4342","Run Time vs Mtas Outer Sum #beta-gated; Energy (keV); Run Time (hr)",this->h2dsettings.at(4342));
	
	hismanager->RegisterPlot<TH2F>("MTAS_43008","Run Time vs Mtas Total #beta-gated; Energy (8 keV/bin); Run Time (s)",this->h2dsettings.at(43008));
	hismanager->RegisterPlot<TH2F>("MTAS_43018","Run Time vs Mtas Total #beta-gated; Energy (8 keV/bin); Run Time (min)",this->h2dsettings.at(43018));
	hismanager->RegisterPlot<TH2F>("MTAS_43028","Run Time vs Mtas Total #beta-gated; Energy (8 keV/bin); Run Time (hr)",this->h2dsettings.at(43028));
}

void MtasProcessor::DeclareNoLogicAntiBetaPlots(PLOTS::PlotRegistry* hismanager){
	hismanager->RegisterPlot<TH2F>("MTAS_4100","Run Time vs Mtas Total anti-#beta-gated; Energy (keV); Run Time (s)",this->h2dsettings.at(4100));
	hismanager->RegisterPlot<TH2F>("MTAS_4101","Run Time vs Mtas Total anti-#beta-gated; Energy (keV); Run Time (min)",this->h2dsettings.at(4101));
	hismanager->RegisterPlot<TH2F>("MTAS_4102","Run Time vs Mtas Total anti-#beta-gated; Energy (keV); Run Time (hr)",this->h2dsettings.at(4102));
	
	hismanager->RegisterPlot<TH2F>("MTAS_4110","Run Time vs Mtas Center Sum anti-#beta-gated; Energy (keV); Run Time (s)",this->h2dsettings.at(4110));
	hismanager->RegisterPlot<TH2F>("MTAS_4111","Run Time vs Mtas Center Sum anti-#beta-gated; Energy (keV); Run Time (min)",this->h2dsettings.at(4111));
	hismanager->RegisterPlot<TH2F>("MTAS_4112","Run Time vs Mtas Center Sum anti-#beta-gated; Energy (keV); Run Time (hr)",this->h2dsettings.at(4112));
	
	hismanager->RegisterPlot<TH2F>("MTAS_4120","Run Time vs Mtas Inner Sum anti-#beta-gated; Energy (keV); Run Time (s)",this->h2dsettings.at(4120));
	hismanager->RegisterPlot<TH2F>("MTAS_4121","Run Time vs Mtas Inner Sum anti-#beta-gated; Energy (keV); Run Time (min)",this->h2dsettings.at(4121));
	hismanager->RegisterPlot<TH2F>("MTAS_4122","Run Time vs Mtas Inner Sum anti-#beta-gated; Energy (keV); Run Time (hr)",this->h2dsettings.at(4122));
	
	hismanager->RegisterPlot<TH2F>("MTAS_4130","Run Time vs Mtas Middle Sum anti-#beta-gated; Energy (keV); Run Time (s)",this->h2dsettings.at(4130));
	hismanager->RegisterPlot<TH2F>("MTAS_4131","Run Time vs Mtas Middle Sum anti-#beta-gated; Energy (keV); Run Time (min)",this->h2dsettings.at(4131));
	hismanager->RegisterPlot<TH2F>("MTAS_4132","Run Time vs Mtas Middle Sum anti-#beta-gated; Energy (keV); Run Time (hr)",this->h2dsettings.at(4132));
	
	hismanager->RegisterPlot<TH2F>("MTAS_4140","Run Time vs Mtas Outer Sum anti-#beta-gated; Energy (keV); Run Time (s)",this->h2dsettings.at(4140));
	hismanager->RegisterPlot<TH2F>("MTAS_4141","Run Time vs Mtas Outer Sum anti-#beta-gated; Energy (keV); Run Time (min)",this->h2dsettings.at(4141));
	hismanager->RegisterPlot<TH2F>("MTAS_4142","Run Time vs Mtas Outer Sum anti-#beta-gated; Energy (keV); Run Time (hr)",this->h2dsettings.at(4142));
	
	hismanager->RegisterPlot<TH2F>("MTAS_41008","Run Time vs Mtas Total anti-#beta-gated; Energy (8 keV/bin); Run Time (s)",this->h2dsettings.at(41008));
	hismanager->RegisterPlot<TH2F>("MTAS_41018","Run Time vs Mtas Total anti-#beta-gated; Energy (8 keV/bin); Run Time (min)",this->h2dsettings.at(41018));
	hismanager->RegisterPlot<TH2F>("MTAS_41028","Run Time vs Mtas Total anti-#beta-gated; Energy (8 keV/bin); Run Time (hr)",this->h2dsettings.at(41028));
}

void MtasProcessor::DeclareBetaPlots(PLOTS::PlotRegistry* hismanager){
	//beta event
	hismanager->RegisterPlot<TH1F>("MTAS_3370","Mtas Total Scalar Rate #beta-gated (s); Time (s)",this->h1dsettings.at(3370));
	hismanager->RegisterPlot<TH1F>("MTAS_3371","Mtas Total Scalar Rate #beta-gated (min); Time (min)",this->h1dsettings.at(3371));
	hismanager->RegisterPlot<TH1F>("MTAS_3372","Mtas Total Scalar Rate #beta-gated (hr); Time (hr)",this->h1dsettings.at(3372));

	hismanager->RegisterPlot<TH1F>("MTAS_3300","Mtas Total #beta-gated; Energy (keV)",this->h1dsettings.at(3300));
	hismanager->RegisterPlot<TH2F>("MTAS_3301","Sum F+B #beta-gated; Energy (keV); F+B Pair (arb.)",this->h2dsettings.at(3301));
	hismanager->RegisterPlot<TH2F>("MTAS_3302","Individual PMT #beta-gated; Channel (arb.); PMT Number (arb.)",this->h2dsettings.at(3302));
	hismanager->RegisterPlot<TH2F>("MTAS_3303","Individual PMT #beta-gated; Energy (keV); PMT Number (arb.)",this->h2dsettings.at(3303));
	
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
	hismanager->RegisterPlot<TH2F>("MTAS_3355","C Segment vs Mtas Total (veto any M,O) #beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(3355));
	hismanager->RegisterPlot<TH2F>("MTAS_3356","C vs Mtas Total (veto any I,M,O) #beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(3356));
	hismanager->RegisterPlot<TH2F>("MTAS_3357","C vs Mtas Total (veto any M,O) #beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(3357));
	
	hismanager->RegisterPlot<TH2F>("MTAS_33508","I,M,O vs Mtas Total #beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33508));
	hismanager->RegisterPlot<TH2F>("MTAS_33518","C vs Mtas Total #beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33518));
	hismanager->RegisterPlot<TH2F>("MTAS_33528","C Segment vs Mtas Total #beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33528));
	hismanager->RegisterPlot<TH2F>("MTAS_33538","C Segment vs C #beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33538));
	hismanager->RegisterPlot<TH2F>("MTAS_33548","C Segment vs Mtas Total (veto any I,M,O)  #beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33548));
	hismanager->RegisterPlot<TH2F>("MTAS_33558","C Segment vs Mtas Total (veto any M,O)  #beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33558));
	hismanager->RegisterPlot<TH2F>("MTAS_33568","C vs Mtas Total (veto any I,M,O)  #beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33568));
	hismanager->RegisterPlot<TH2F>("MTAS_33578","C vs Mtas Total (veto any M,O)  #beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(33578));
	
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

	if( this->gammagammaplots ){
		hismanager->RegisterPlot<TH2F>("MTAS_5300","I,M,O, Gamma-Gamma Matrix #beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(5300));
		hismanager->RegisterPlot<TH2F>("MTAS_5301","C Gamma-Gamma Matrix #beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(5301));
		hismanager->RegisterPlot<TH2F>("MTAS_5302","C,I,M,O, Gamma-Gamma Matrix #beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(5302));
		hismanager->RegisterPlot<TH2F>("MTAS_53008","I,M,O, Gamma-Gamma Matrix #beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(53008));
		hismanager->RegisterPlot<TH2F>("MTAS_53018","C Gamma-Gamma Matrix #beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(53018));
		hismanager->RegisterPlot<TH2F>("MTAS_53028","C,I,M,O, Gamma-Gamma Matrix #beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(53028));
	}

	if( this->UseOldCenter ){
		hismanager->RegisterPlot<TH2F>("MTAS_5530","Num Center PMT Fire vs Center Sum Before Zero #beta-gated; Energy (keV); count (arb.)",this->h2dsettings.at(5530));
	}
}

void MtasProcessor::DeclareAntiBetaPlots(PLOTS::PlotRegistry* hismanager){
	//not beta event
	hismanager->RegisterPlot<TH1F>("MTAS_3170","Mtas Total Scalar Rate anti-#beta-gated (s); Time (s)",this->h1dsettings.at(3170));
	hismanager->RegisterPlot<TH1F>("MTAS_3171","Mtas Total Scalar Rate anti-#beta-gated (min); Time (min)",this->h1dsettings.at(3171));
	hismanager->RegisterPlot<TH1F>("MTAS_3172","Mtas Total Scalar Rate anti-#beta-gated (hr); Time (hr)",this->h1dsettings.at(3172));

	hismanager->RegisterPlot<TH1F>("MTAS_3100","Mtas Total anti-#beta-gated; Energy (keV)",this->h1dsettings.at(3100));
	hismanager->RegisterPlot<TH2F>("MTAS_3101","Sum F+B anti-#beta-gated; Energy (keV); F+B Pair (arb.)",this->h2dsettings.at(3101));
	hismanager->RegisterPlot<TH2F>("MTAS_3102","Individual PMT anti-#beta-gated; Channel (arb.); PMT Number (arb.)",this->h2dsettings.at(3102));
	hismanager->RegisterPlot<TH2F>("MTAS_3103","Individual PMT anti-#beta-gated; Energy (keV); PMT Number (arb.)",this->h2dsettings.at(3103));
	
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
	hismanager->RegisterPlot<TH2F>("MTAS_3155","C Segment vs Mtas Total (veto any M,O) anti-#beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(3155));
	hismanager->RegisterPlot<TH2F>("MTAS_3156","C vs Mtas Total (veto any I,M,O) anti-#beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(3156));
	hismanager->RegisterPlot<TH2F>("MTAS_3157","C vs Mtas Total (veto any M,O) anti-#beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(3157));
	
	hismanager->RegisterPlot<TH2F>("MTAS_31508","I,M,O vs Mtas Total anti-#beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(31508));
	hismanager->RegisterPlot<TH2F>("MTAS_31518","C vs Mtas Total anti-#beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(31518));
	hismanager->RegisterPlot<TH2F>("MTAS_31528","C Segment vs Mtas Total anti-#beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(31528));
	hismanager->RegisterPlot<TH2F>("MTAS_31538","C Segment vs Mtas Total anti-#beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(31538));
	hismanager->RegisterPlot<TH2F>("MTAS_31548","C Segment vs Mtas Total (veto any I,M,O) anti-#beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(31548));
	hismanager->RegisterPlot<TH2F>("MTAS_31558","C Segment vs Mtas Total (veto any M,O) anti-#beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(31558));
	hismanager->RegisterPlot<TH2F>("MTAS_31568","C vs Mtas Total (veto any I,M,O) anti-#beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(31568));
	hismanager->RegisterPlot<TH2F>("MTAS_31578","C vs Mtas Total (veto any M,O) anti-#beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",this->h2dsettings.at(31578));
	
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

	if( this->gammagammaplots ){
		hismanager->RegisterPlot<TH2F>("MTAS_5100","I,M,O, Gamma-Gamma Matrix anti-#beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(5100));
		hismanager->RegisterPlot<TH2F>("MTAS_5101","C Gamma-Gamma Matrix anti-#beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(5101));
		hismanager->RegisterPlot<TH2F>("MTAS_5102","C,I,M,O, Gamma-Gamma Matrix anti-#beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(5102));
		hismanager->RegisterPlot<TH2F>("MTAS_51008","I,M,O, Gamma-Gamma Matrix anti-#beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(51008));
		hismanager->RegisterPlot<TH2F>("MTAS_51018","C Gamma-Gamma Matrix anti-#beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(51018));
		hismanager->RegisterPlot<TH2F>("MTAS_51028","C,I,M,O, Gamma-Gamma Matrix anti-#beta-gated; Energy (keV); Energy (keV)",this->h2dsettings.at(51028));
	}
	
	if( this->UseOldCenter ){
		hismanager->RegisterPlot<TH2F>("MTAS_5510","Num Center PMT Fire vs Center Sum Before Zero anti-#beta-gated; Energy (keV); count (arb.)",this->h2dsettings.at(5510));
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

	//this->OutputTree->Branch("EventIdx",&(this->Eventidx));
	
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
	for( size_t ii = 0; ii < 6; ++ii ){
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

	this->Back2BackCenterFire = false;
	this->Back2BackCenter511RegionFire = false;

	this->FirstTime = -1.0;
	this->LastTime = -1.0;

	this->TimeStamps.clear();

	for( size_t ii = 0; ii < 6; ++ii ){
		this->ValidCenterSegments[ii] = false;
	}

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
		if( this->UseOldCenter ){
			hismanager->Fill("MTAS_5530",this->TotalEnergy[5],this->NumFire[5]);
		}
		hismanager->Fill("MTAS_3370",this->currevttime);
		hismanager->Fill("MTAS_3371",this->currevttime/60.0);
		hismanager->Fill("MTAS_3372",this->currevttime/(60.0*60.0));
		for( int ii = 0; ii < 6; ++ii ){
			auto currhx = this->HexagonShapes[ii].center;
			if( this->CenterHits[2*ii] ){
				for( size_t jj = 0; jj < this->CenterHits[2*ii]; ++jj ){
					this->MTAS_2601->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2603->Fill(currhx.first,currhx.second,1.0);
				}
			}
			if( this->CenterHits[2*ii+1] ){
				for( size_t jj = 0; jj < this->CenterHits[2*ii]; ++jj ){
					this->MTAS_2602->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2603->Fill(currhx.first,currhx.second,-1.0);
				}
			}
			if( this->CenterHits[2*ii] and this->CenterHits[2*ii+1] ){
				this->MTAS_2600->Fill(currhx.first,currhx.second,1.0);
			}

			currhx = this->HexagonShapes[ii+6].center;
			if( this->InnerHits[2*ii] ){
				for( size_t jj = 0; jj < this->InnerHits[2*ii]; ++jj ){
					this->MTAS_2601->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2603->Fill(currhx.first,currhx.second,1.0);
				}
			}
			if( this->InnerHits[2*ii+1] ){
				for( size_t jj = 0; jj < this->InnerHits[2*ii]; ++jj ){
					this->MTAS_2602->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2603->Fill(currhx.first,currhx.second,-1.0);
				}
			}
			if( this->InnerHits[2*ii] and this->InnerHits[2*ii+1] ){
				this->MTAS_2600->Fill(currhx.first,currhx.second,1.0);
			}

			currhx = this->HexagonShapes[ii+12].center;
			if( this->MiddleHits[2*ii] ){
				for( size_t jj = 0; jj < this->MiddleHits[2*ii]; ++jj ){
					this->MTAS_2601->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2603->Fill(currhx.first,currhx.second,1.0);
				}
			}
			if( this->MiddleHits[2*ii+1] ){
				for( size_t jj = 0; jj < this->MiddleHits[2*ii]; ++jj ){
					this->MTAS_2602->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2603->Fill(currhx.first,currhx.second,-1.0);
				}
			}
			if( this->MiddleHits[2*ii] and this->MiddleHits[2*ii+1] ){
				this->MTAS_2600->Fill(currhx.first,currhx.second,1.0);
			}

			currhx = this->HexagonShapes[ii+18].center;
			if( this->OuterHits[2*ii] ){
				for( size_t jj = 0; jj < this->OuterHits[2*ii]; ++jj ){
					this->MTAS_2601->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2603->Fill(currhx.first,currhx.second,1.0);
				}
			}
			if( this->OuterHits[2*ii+1] ){
				for( size_t jj = 0; jj < this->OuterHits[2*ii]; ++jj ){
					this->MTAS_2602->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2603->Fill(currhx.first,currhx.second,-1.0);
				}
			}
			if( this->OuterHits[2*ii] and this->OuterHits[2*ii+1] ){
				this->MTAS_2600->Fill(currhx.first,currhx.second,1.0);
			}
		}

		hismanager->Fill("MTAS_3300",this->TotalEnergy[0]);

		hismanager->Fill("MTAS_3310",this->TotalEnergy[1]);
		hismanager->Fill("MTAS_3320",this->TotalEnergy[2]);
		hismanager->Fill("MTAS_3330",this->TotalEnergy[3]);
		hismanager->Fill("MTAS_3340",this->TotalEnergy[4]);

		hismanager->Fill("MTAS_3351",this->TotalEnergy[0],this->TotalEnergy[1]);
		hismanager->Fill("MTAS_33518",this->TotalEnergy[0],this->TotalEnergy[1]);

		if( (not this->MiddleFire) and (not this->OuterFire) ){
			hismanager->Fill("MTAS_3357",this->TotalEnergy[0],this->TotalEnergy[1]);
			hismanager->Fill("MTAS_33578",this->TotalEnergy[0],this->TotalEnergy[1]);
			if( not this->InnerFire ){
				hismanager->Fill("MTAS_3356",this->TotalEnergy[0],this->TotalEnergy[1]);
				hismanager->Fill("MTAS_33568",this->TotalEnergy[0],this->TotalEnergy[1]);
			}
		}

		//these are the gamma-gamma matrices
		if( this->gammagammaplots ){
			auto MTAS_5300  = hismanager->GetPlot<TH2*>("MTAS_5300");
			auto MTAS_5301  = hismanager->GetPlot<TH2*>("MTAS_5301");
			auto MTAS_5302  = hismanager->GetPlot<TH2*>("MTAS_5302");
			auto MTAS_53008 = hismanager->GetPlot<TH2*>("MTAS_53008");
			auto MTAS_53018 = hismanager->GetPlot<TH2*>("MTAS_53018");
			auto MTAS_53028 = hismanager->GetPlot<TH2*>("MTAS_53028");
			for( int ii = 0; ii < 6; ++ii ){
				for( int jj = ii+1; jj < 6; ++jj ){
					MTAS_5301->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_5301->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_5302->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_5302->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_53018->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_53018->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_53028->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_53028->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
				}	
			}
			for( int ii = 6; ii < 24; ++ii ){
				for( int jj = ii+1; jj < 24; ++jj ){
					MTAS_5300->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_5300->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_5302->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_5302->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_53008->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_53008->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_53028->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_53028->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
				}
			}
		}

		auto MTAS_3301 = hismanager->GetPlot<TH2*>("MTAS_3301");
		for( int ii = 0; ii < 24; ++ii ){
			MTAS_3301->Fill(this->CrystalEnergy[ii],ii);
		}
		auto MTAS_3302 = hismanager->GetPlot<TH2*>("MTAS_3302");
		auto MTAS_3303 = hismanager->GetPlot<TH2*>("MTAS_3303");
		for( int ii = 0; ii < 12; ++ii ){
			MTAS_3302->Fill(this->RawCenter[ii],ii);
			MTAS_3302->Fill(this->RawInner[ii],ii+12);
			MTAS_3302->Fill(this->RawMiddle[ii],ii+24);
			MTAS_3302->Fill(this->RawOuter[ii],ii+36);
			MTAS_3303->Fill(this->Center[ii],ii);
			MTAS_3303->Fill(this->Inner[ii],ii+12);
			MTAS_3303->Fill(this->Middle[ii],ii+24);
			MTAS_3303->Fill(this->Outer[ii],ii+36);
		}
		auto MTAS_3350  = hismanager->GetPlot<TH2*>("MTAS_3350");
		auto MTAS_33508 = hismanager->GetPlot<TH2*>("MTAS_33508");
		auto MTAS_3315  = hismanager->GetPlot<TH1*>("MTAS_3315");
		auto MTAS_3325  = hismanager->GetPlot<TH1*>("MTAS_3325");
		auto MTAS_3335  = hismanager->GetPlot<TH1*>("MTAS_3335");
		auto MTAS_3345  = hismanager->GetPlot<TH1*>("MTAS_3345");
		auto MTAS_3352  = hismanager->GetPlot<TH2*>("MTAS_3352");
		auto MTAS_33528 = hismanager->GetPlot<TH2*>("MTAS_33528");
		auto MTAS_3353  = hismanager->GetPlot<TH2*>("MTAS_3353");
		auto MTAS_33538 = hismanager->GetPlot<TH2*>("MTAS_33538");
		auto MTAS_3354  = hismanager->GetPlot<TH2*>("MTAS_3354");
		auto MTAS_33548 = hismanager->GetPlot<TH2*>("MTAS_33548");
		auto MTAS_3355  = hismanager->GetPlot<TH2*>("MTAS_3355");
		auto MTAS_33558 = hismanager->GetPlot<TH2*>("MTAS_33558");
		for( int ii = 0; ii < 6; ++ii ){
			std::string id = std::to_string(ii);

			std::string name = "MTAS_336"+id+"_F";
			hismanager->Fill(name,this->Position[ii],this->RawCenter[2*ii]);

			name = "MTAS_336"+id+"_B";
			hismanager->Fill(name,this->Position[ii],this->RawCenter[2*ii + 1]);

			name = "MTAS_336"+id;
			hismanager->Fill(name,this->Position[ii],(this->RawCenter[2*ii] + this->RawCenter[2*ii + 1])/2.0);

			MTAS_3315->Fill(this->CrystalEnergy[ii]);
			MTAS_3325->Fill(this->CrystalEnergy[ii+6]);
			MTAS_3335->Fill(this->CrystalEnergy[ii+12]);
			MTAS_3345->Fill(this->CrystalEnergy[ii+18]);

			MTAS_3350->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+6]);
			MTAS_3350->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+12]);
			MTAS_3350->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+18]);

			MTAS_3352->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);

			MTAS_3353->Fill(this->TotalEnergy[1],this->CrystalEnergy[ii]);

			MTAS_33508->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+6]);
			MTAS_33508->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+12]);
			MTAS_33508->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+18]);

			MTAS_33528->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);

			MTAS_33538->Fill(this->TotalEnergy[1],this->CrystalEnergy[ii]);

			if( (not this->MiddleFire) and (not this->OuterFire) ){
				MTAS_3355->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);
				MTAS_33558->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);
				if( not this->InnerFire ){
					MTAS_3354->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);
					MTAS_33548->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);
				}
			}
		}
	}
}

void MtasProcessor::FillNoLogicBetaPlots(PLOTS::PlotRegistry* hismanager){
	if( (not this->AnySaturate) and (not this->AnyPileup) and this->logictimeplots ){
		hismanager->Fill("MTAS_4300",this->TotalEnergy[0],this->currevttime);
		hismanager->Fill("MTAS_4301",this->TotalEnergy[0],this->currevttime/60.0);
		hismanager->Fill("MTAS_4302",this->TotalEnergy[0],this->currevttime/(60.0*60.0));

		hismanager->Fill("MTAS_4310",this->TotalEnergy[1],this->currevttime);
		hismanager->Fill("MTAS_4311",this->TotalEnergy[1],this->currevttime/60.0);
		hismanager->Fill("MTAS_4312",this->TotalEnergy[1],this->currevttime/(60.0*60.0));

		hismanager->Fill("MTAS_4320",this->TotalEnergy[2],this->currevttime);
		hismanager->Fill("MTAS_4321",this->TotalEnergy[2],this->currevttime/60.0);
		hismanager->Fill("MTAS_4322",this->TotalEnergy[2],this->currevttime/(60.0*60.0));

		hismanager->Fill("MTAS_4330",this->TotalEnergy[3],this->currevttime);
		hismanager->Fill("MTAS_4331",this->TotalEnergy[3],this->currevttime/60.0);
		hismanager->Fill("MTAS_4332",this->TotalEnergy[3],this->currevttime/(60.0*60.0));

		hismanager->Fill("MTAS_4340",this->TotalEnergy[4],this->currevttime);
		hismanager->Fill("MTAS_4341",this->TotalEnergy[4],this->currevttime/60.0);
		hismanager->Fill("MTAS_4342",this->TotalEnergy[4],this->currevttime/(60.0*60.0));

		hismanager->Fill("MTAS_43008",this->TotalEnergy[0],this->currevttime);
		hismanager->Fill("MTAS_43018",this->TotalEnergy[0],this->currevttime/60.0);
		hismanager->Fill("MTAS_43028",this->TotalEnergy[0],this->currevttime/(60.0*60.0));

		if( this->diagnosticplots ){
			auto MTAS_3431 = hismanager->GetPlot<TH2*>("MTAS_3431");
			auto MTAS_3432 = hismanager->GetPlot<TH2*>("MTAS_3432");
			auto MTAS_3433 = hismanager->GetPlot<TH2*>("MTAS_3433");
			auto MTAS_3434 = hismanager->GetPlot<TH2*>("MTAS_3434");
			auto MTAS_3531 = hismanager->GetPlot<TH2*>("MTAS_3531");
			auto MTAS_3532 = hismanager->GetPlot<TH2*>("MTAS_3532");
			auto MTAS_3533 = hismanager->GetPlot<TH2*>("MTAS_3533");
			auto MTAS_3534 = hismanager->GetPlot<TH2*>("MTAS_3534");
			for( int ii = 0; ii < 6; ++ii ){
				MTAS_3431->Fill(this->RawCenter[2*ii],2*ii);
				MTAS_3431->Fill(this->RawCenter[2*ii + 1],2*ii + 1);

				MTAS_3432->Fill(this->RawInner[2*ii],2*ii);
				MTAS_3432->Fill(this->RawInner[2*ii + 1],2*ii + 1);

				MTAS_3433->Fill(this->RawMiddle[2*ii],2*ii);
				MTAS_3433->Fill(this->RawMiddle[2*ii + 1],2*ii + 1);

				MTAS_3434->Fill(this->RawOuter[2*ii],2*ii);
				MTAS_3434->Fill(this->RawOuter[2*ii + 1],2*ii + 1);

				MTAS_3531->Fill(this->CalCenter[2*ii],2*ii);
				MTAS_3531->Fill(this->CalCenter[2*ii + 1],2*ii + 1);

				MTAS_3532->Fill(this->CalInner[2*ii],2*ii);
				MTAS_3532->Fill(this->CalInner[2*ii + 1],2*ii + 1);

				MTAS_3533->Fill(this->CalMiddle[2*ii],2*ii);
				MTAS_3533->Fill(this->CalMiddle[2*ii + 1],2*ii + 1);

				MTAS_3534->Fill(this->CalOuter[2*ii],2*ii);
				MTAS_3534->Fill(this->CalOuter[2*ii + 1],2*ii + 1);
			}
		}
	}
}

void MtasProcessor::FillNonBetaPlots(PLOTS::PlotRegistry* hismanager){
	if( (not this->AnySaturate) and (not this->AnyPileup) ){
		if( this->UseOldCenter ){
			hismanager->Fill("MTAS_5510",this->TotalEnergy[5],this->NumFire[5]);
		}
		hismanager->Fill("MTAS_3170",this->currevttime);
		hismanager->Fill("MTAS_3171",this->currevttime/60.0);
		hismanager->Fill("MTAS_3172",this->currevttime/(60.0*60.0));
		for( int ii = 0; ii < 6; ++ii ){
			auto currhx = this->HexagonShapes[ii].center;
			if( this->CenterHits[2*ii] ){
				for( size_t jj = 0; jj < this->CenterHits[2*ii]; ++jj ){
					this->MTAS_2401->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2403->Fill(currhx.first,currhx.second,1.0);
				}
			}
			if( this->CenterHits[2*ii+1] ){
				for( size_t jj = 0; jj < this->CenterHits[2*ii]; ++jj ){
					this->MTAS_2402->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2403->Fill(currhx.first,currhx.second,-1.0);
				}
			}
			if( this->CenterHits[2*ii] and this->CenterHits[2*ii+1] ){
				this->MTAS_2400->Fill(currhx.first,currhx.second,1.0);
			}

			currhx = this->HexagonShapes[ii+6].center;
			if( this->InnerHits[2*ii] ){
				for( size_t jj = 0; jj < this->InnerHits[2*ii]; ++jj ){
					this->MTAS_2401->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2403->Fill(currhx.first,currhx.second,1.0);
				}
			}
			if( this->InnerHits[2*ii+1] ){
				for( size_t jj = 0; jj < this->InnerHits[2*ii]; ++jj ){
					this->MTAS_2402->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2403->Fill(currhx.first,currhx.second,-1.0);
				}
			}
			if( this->InnerHits[2*ii] and this->InnerHits[2*ii+1] ){
				this->MTAS_2400->Fill(currhx.first,currhx.second,1.0);
			}

			currhx = this->HexagonShapes[ii+12].center;
			if( this->MiddleHits[2*ii] ){
				for( size_t jj = 0; jj < this->MiddleHits[2*ii]; ++jj ){
					this->MTAS_2401->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2403->Fill(currhx.first,currhx.second,1.0);
				}
			}
			if( this->MiddleHits[2*ii+1] ){
				for( size_t jj = 0; jj < this->MiddleHits[2*ii]; ++jj ){
					this->MTAS_2402->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2403->Fill(currhx.first,currhx.second,-1.0);
				}
			}
			if( this->MiddleHits[2*ii] and this->MiddleHits[2*ii+1] ){
				this->MTAS_2400->Fill(currhx.first,currhx.second,1.0);
			}

			currhx = this->HexagonShapes[ii+18].center;
			if( this->OuterHits[2*ii] ){
				for( size_t jj = 0; jj < this->OuterHits[2*ii]; ++jj ){
					this->MTAS_2401->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2403->Fill(currhx.first,currhx.second,1.0);
				}
			}
			if( this->OuterHits[2*ii+1] ){
				for( size_t jj = 0; jj < this->OuterHits[2*ii]; ++jj ){
					this->MTAS_2402->Fill(currhx.first,currhx.second,1.0);
					this->MTAS_2403->Fill(currhx.first,currhx.second,-1.0);
				}
			}
			if( this->OuterHits[2*ii] and this->OuterHits[2*ii+1] ){
				this->MTAS_2400->Fill(currhx.first,currhx.second,1.0);
			}
		}

		hismanager->Fill("MTAS_3100",this->TotalEnergy[0]);

		hismanager->Fill("MTAS_3110",this->TotalEnergy[1]);
		hismanager->Fill("MTAS_3120",this->TotalEnergy[2]);
		hismanager->Fill("MTAS_3130",this->TotalEnergy[3]);
		hismanager->Fill("MTAS_3140",this->TotalEnergy[4]);

		hismanager->Fill("MTAS_3151",this->TotalEnergy[0],this->TotalEnergy[1]);
		hismanager->Fill("MTAS_31518",this->TotalEnergy[0],this->TotalEnergy[1]);

		if( (not this->MiddleFire) and (not this->OuterFire) ){
			hismanager->Fill("MTAS_3157",this->TotalEnergy[0],this->TotalEnergy[1]);
			hismanager->Fill("MTAS_31578",this->TotalEnergy[0],this->TotalEnergy[1]);
			if( not this->InnerFire ){
				hismanager->Fill("MTAS_3156",this->TotalEnergy[0],this->TotalEnergy[1]);
				hismanager->Fill("MTAS_31568",this->TotalEnergy[0],this->TotalEnergy[1]);
			}
		}

		//these are the gamma-gamma matrices
		if( this->gammagammaplots ){
			auto MTAS_5100  = hismanager->GetPlot<TH2*>("MTAS_5100");
			auto MTAS_5101  = hismanager->GetPlot<TH2*>("MTAS_5101");
			auto MTAS_5102  = hismanager->GetPlot<TH2*>("MTAS_5102");
			auto MTAS_51008 = hismanager->GetPlot<TH2*>("MTAS_51008");
			auto MTAS_51018 = hismanager->GetPlot<TH2*>("MTAS_51018");
			auto MTAS_51028 = hismanager->GetPlot<TH2*>("MTAS_51028");
			for( int ii = 0; ii < 6; ++ii ){
				for( int jj = ii+1; jj < 6; ++jj ){
					MTAS_5101->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_5101->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_5102->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_5102->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_51018->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_51018->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_51028->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_51028->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
				}	
			}
			for( int ii = 6; ii < 24; ++ii ){
				for( int jj = ii+1; jj < 24; ++jj ){
					MTAS_5100->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_5100->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_5102->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_5102->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_51008->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_51008->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
					MTAS_51028->Fill(this->CrystalEnergy[ii],this->CrystalEnergy[jj]);
					MTAS_51028->Fill(this->CrystalEnergy[jj],this->CrystalEnergy[ii]);
				}
			}
		}

		auto MTAS_3101 = hismanager->GetPlot<TH2*>("MTAS_3101");
		for( int ii = 0; ii < 24; ++ii ){
			MTAS_3101->Fill(this->CrystalEnergy[ii],ii);
		}
		auto MTAS_3102 = hismanager->GetPlot<TH2*>("MTAS_3102");
		auto MTAS_3103 = hismanager->GetPlot<TH2*>("MTAS_3103");
		for( int ii = 0; ii < 12; ++ii ){
			MTAS_3102->Fill(this->RawCenter[ii],ii);
			MTAS_3102->Fill(this->RawInner[ii],ii+12);
			MTAS_3102->Fill(this->RawMiddle[ii],ii+24);
			MTAS_3102->Fill(this->RawOuter[ii],ii+36);
			MTAS_3103->Fill(this->Center[ii],ii);
			MTAS_3103->Fill(this->Inner[ii],ii+12);
			MTAS_3103->Fill(this->Middle[ii],ii+24);
			MTAS_3103->Fill(this->Outer[ii],ii+36);
		}
		auto MTAS_3150  = hismanager->GetPlot<TH2*>("MTAS_3150");
		auto MTAS_31508 = hismanager->GetPlot<TH2*>("MTAS_31508");
		auto MTAS_3115  = hismanager->GetPlot<TH1*>("MTAS_3115");
		auto MTAS_3125  = hismanager->GetPlot<TH1*>("MTAS_3125");
		auto MTAS_3135  = hismanager->GetPlot<TH1*>("MTAS_3135");
		auto MTAS_3145  = hismanager->GetPlot<TH1*>("MTAS_3145");
		auto MTAS_3152  = hismanager->GetPlot<TH2*>("MTAS_3152");
		auto MTAS_31528 = hismanager->GetPlot<TH2*>("MTAS_31528");
		auto MTAS_3153  = hismanager->GetPlot<TH2*>("MTAS_3153");
		auto MTAS_31538 = hismanager->GetPlot<TH2*>("MTAS_31538");
		auto MTAS_3154  = hismanager->GetPlot<TH2*>("MTAS_3154");
		auto MTAS_31548 = hismanager->GetPlot<TH2*>("MTAS_31548");
		auto MTAS_3155  = hismanager->GetPlot<TH2*>("MTAS_3155");
		auto MTAS_31558 = hismanager->GetPlot<TH2*>("MTAS_31558");
		for( int ii = 0; ii < 6; ++ii ){
			std::string id = std::to_string(ii);

			std::string name = "MTAS_316"+id+"_F";
			hismanager->Fill(name,this->Position[ii],this->RawCenter[2*ii]);

			name = "MTAS_316"+id+"_B";
			hismanager->Fill(name,this->Position[ii],this->RawCenter[2*ii + 1]);

			name = "MTAS_316"+id;
			hismanager->Fill(name,this->Position[ii],(this->RawCenter[2*ii] + this->RawCenter[2*ii + 1])/2.0);

			MTAS_3115->Fill(this->CrystalEnergy[ii]);
			MTAS_3125->Fill(this->CrystalEnergy[ii+6]);
			MTAS_3135->Fill(this->CrystalEnergy[ii+12]);
			MTAS_3145->Fill(this->CrystalEnergy[ii+18]);

			MTAS_3150->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+6]);
			MTAS_3150->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+12]);
			MTAS_3150->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+18]);

			MTAS_3152->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);

			MTAS_3153->Fill(this->TotalEnergy[1],this->CrystalEnergy[ii]);

			MTAS_31508->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+6]);
			MTAS_31508->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+12]);
			MTAS_31508->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii+18]);

			MTAS_31528->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);

			MTAS_31538->Fill(this->TotalEnergy[1],this->CrystalEnergy[ii]);

			if( (not this->MiddleFire) and (not this->OuterFire) ){
				MTAS_3155->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);
				MTAS_31558->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);
				if( not this->InnerFire ){
					MTAS_3154->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);
					MTAS_31548->Fill(this->TotalEnergy[0],this->CrystalEnergy[ii]);
				}
			}

		}
	}
}

void MtasProcessor::FillNoLogicNonBetaPlots(PLOTS::PlotRegistry* hismanager){
	if( (not this->AnySaturate) and (not this->AnyPileup) and this->logictimeplots ){
		hismanager->Fill("MTAS_4100",this->TotalEnergy[0],this->currevttime);
		hismanager->Fill("MTAS_4101",this->TotalEnergy[0],this->currevttime/60.0);
		hismanager->Fill("MTAS_4102",this->TotalEnergy[0],this->currevttime/(60.0*60.0));

		hismanager->Fill("MTAS_4110",this->TotalEnergy[1],this->currevttime);
		hismanager->Fill("MTAS_4111",this->TotalEnergy[1],this->currevttime/60.0);
		hismanager->Fill("MTAS_4112",this->TotalEnergy[1],this->currevttime/(60.0*60.0));

		hismanager->Fill("MTAS_4120",this->TotalEnergy[2],this->currevttime);
		hismanager->Fill("MTAS_4121",this->TotalEnergy[2],this->currevttime/60.0);
		hismanager->Fill("MTAS_4122",this->TotalEnergy[2],this->currevttime/(60.0*60.0));

		hismanager->Fill("MTAS_4130",this->TotalEnergy[3],this->currevttime);
		hismanager->Fill("MTAS_4131",this->TotalEnergy[3],this->currevttime/60.0);
		hismanager->Fill("MTAS_4132",this->TotalEnergy[3],this->currevttime/(60.0*60.0));

		hismanager->Fill("MTAS_4140",this->TotalEnergy[4],this->currevttime);
		hismanager->Fill("MTAS_4141",this->TotalEnergy[4],this->currevttime/60.0);
		hismanager->Fill("MTAS_4142",this->TotalEnergy[4],this->currevttime/(60.0*60.0));

		hismanager->Fill("MTAS_41008",this->TotalEnergy[0],this->currevttime);
		hismanager->Fill("MTAS_41018",this->TotalEnergy[0],this->currevttime/60.0);
		hismanager->Fill("MTAS_41028",this->TotalEnergy[0],this->currevttime/(60.0*60.0));

		if( this->diagnosticplots ){
			auto MTAS_3411 = hismanager->GetPlot<TH2*>("MTAS_3411");
			auto MTAS_3412 = hismanager->GetPlot<TH2*>("MTAS_3412");
			auto MTAS_3413 = hismanager->GetPlot<TH2*>("MTAS_3413");
			auto MTAS_3414 = hismanager->GetPlot<TH2*>("MTAS_3414");
			auto MTAS_3511 = hismanager->GetPlot<TH2*>("MTAS_3511");
			auto MTAS_3512 = hismanager->GetPlot<TH2*>("MTAS_3512");
			auto MTAS_3513 = hismanager->GetPlot<TH2*>("MTAS_3513");
			auto MTAS_3514 = hismanager->GetPlot<TH2*>("MTAS_3514");
			for( int ii = 0; ii < 6; ++ii ){
				MTAS_3411->Fill(this->RawCenter[2*ii],2*ii);
				MTAS_3411->Fill(this->RawCenter[2*ii + 1],2*ii + 1);

				MTAS_3412->Fill(this->RawInner[2*ii],2*ii);
				MTAS_3412->Fill(this->RawInner[2*ii + 1],2*ii + 1);

				MTAS_3413->Fill(this->RawMiddle[2*ii],2*ii);
				MTAS_3413->Fill(this->RawMiddle[2*ii + 1],2*ii + 1);

				MTAS_3414->Fill(this->RawOuter[2*ii],2*ii);
				MTAS_3414->Fill(this->RawOuter[2*ii + 1],2*ii + 1);

				MTAS_3511->Fill(this->CalCenter[2*ii],2*ii);
				MTAS_3511->Fill(this->CalCenter[2*ii + 1],2*ii + 1);

				MTAS_3512->Fill(this->CalInner[2*ii],2*ii);
				MTAS_3512->Fill(this->CalInner[2*ii + 1],2*ii + 1);

				MTAS_3513->Fill(this->CalMiddle[2*ii],2*ii);
				MTAS_3513->Fill(this->CalMiddle[2*ii + 1],2*ii + 1);

				MTAS_3514->Fill(this->CalOuter[2*ii],2*ii);
				MTAS_3514->Fill(this->CalOuter[2*ii + 1],2*ii + 1);
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

const double& MtasProcessor::GetIndividualCenterPMTEnergy(const int& idx) const{
	return this->CalCenter[idx];
}

const double& MtasProcessor::GetIndividualCenterPMTRawEnergy(const int& idx) const{
	return this->RawCenter[idx];
}

const double& MtasProcessor::GetIndividualInnerPMTEnergy(const int& idx) const{
	return this->CalInner[idx];
}

const double& MtasProcessor::GetIndividualInnerPMTRawEnergy(const int& idx) const{
	return this->RawInner[idx];
}

const double& MtasProcessor::GetIndividualMiddlePMTEnergy(const int& idx) const{
	return this->CalMiddle[idx];
}

const double& MtasProcessor::GetIndividualMiddlePMTRawEnergy(const int& idx) const{
	return this->RawMiddle[idx];
}

const double& MtasProcessor::GetIndividualOuterPMTEnergy(const int& idx) const{
	return this->CalOuter[idx];
}

const double& MtasProcessor::GetIndividualOuterPMTRawEnergy(const int& idx) const{
	return this->RawOuter[idx];
}

bool MtasProcessor::DidBack2BackCenterSegmentsFire() const{
	return this->Back2BackCenterFire;
}

bool MtasProcessor::DidBack2BackCenterSegmentsFireIn511Region() const{
	return this->Back2BackCenter511RegionFire;
}
