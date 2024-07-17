#include "MtasProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

MtasProcessor::MtasProcessor(const std::string& log) : Processor(log,"MtasProcessor",{"mtas"}){
	this->NewEvt = EventInfo();
	this->PrevEvt = this->NewEvt;
	this->CurrEvt = this->NewEvt;

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
		{3534, {8192,0.0,8192.0,12,0,12}}
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

	this->CenterFire = false;
	this->InnerFire = false;
	this->MiddleFire = false;
	this->OuterFire = false;

	for( size_t ii = 0; ii < 12; ++ii ){
		this->PosCorrectionMap.push_back(nullptr);
	}
}

[[maybe_unused]] bool MtasProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	summary.GetDetectorSummary(this->AllDefaultRegex["mtas"],this->SummaryData);
	for( const auto& evt : this->SummaryData ){
		this->CurrEvt.RealEvt = true;
		auto subtype = evt->GetSubType();
		auto group = evt->GetGroup();

		auto isfront = evt->HasTag(fronttag);
		auto isback = evt->HasTag(backtag);

		//this->console->info("{} {} {}",subtype,group,group.size());
		
		int segmentid = std::stoi(group);

		if( (not isfront and not isback) or (isfront and isback) ){
			throw std::runtime_error("evt in MtasProcessor is malformed in xml, and has either both front and back tag or neither");
		}

		if( subtype.compare("center") == 0 ){
			this->currsubtype = SUBTYPE::CENTER;
		}else if( subtype.compare("inner") == 0 ){
			this->currsubtype = SUBTYPE::INNER;
		}else if( subtype.compare("middle") == 0 ){
			this->currsubtype = SUBTYPE::MIDDLE;
		}else if( subtype.compare("outer") == 0 ){
			this->currsubtype = SUBTYPE::OUTER;
		}else{
			this->currsubtype = SUBTYPE::UNKNOWN;
			throw std::runtime_error("unknown subtype"+subtype+" correct subtypes are center, inner, middle, outer");
		}

		if( not foundfirstevt ){
			foundfirstevt = true;
			globalfirsttime = evt->GetTimeStamp();
		}

		if( evt->GetPileup() or evt->GetSaturation() ){
			//ignore the saturated channel, but keep everything else in this current event
			if( evt->GetPileup() ){
				this->CurrEvt.Pileup = true;
			}
			if( evt->GetSaturation() ){
				this->CurrEvt.Saturate = true;
			}
			continue;
		}

		int position = segmentid - 1; 	
		int detectorposition = 2*position + isback;

		if( currsubtype == SUBTYPE::CENTER ){
			if( !this->CenterHits[detectorposition] ){
				this->Center[detectorposition] = evt->GetEnergy();
				this->RawCenter[detectorposition] = evt->GetRawEnergyWRandom();
				this->CalCenter[detectorposition] = evt->GetEnergy();
				this->TimeStamps.push_back(evt->GetTimeStamp());
				++this->CenterHits[detectorposition];
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
			}else{
				++this->OuterHits[detectorposition];
			}
		}else{
			throw std::runtime_error("we shouldn't have gotten here");
		}

	}

	//for( size_t ii = 0; ii < this->CenterHits.size(); ++ii ){
	//	if( this->CenterHits[ii] > 1 ){
	//		this->console->warn("Linearized MTAS Center number {} has {} hits in a single event instead of 1",ii,this->CenterHits[ii]);
	//	}
	//	if( this->InnerHits[ii] > 1 ){
	//		this->console->warn("Linearized MTAS Inner number {} has {} hits in a single event instead of 1",ii,this->InnerHits[ii]);
	//	}
	//	if( this->MiddleHits[ii] > 1 ){
	//		this->console->warn("Linearized MTAS Middle number {} has {} hits in a single event instead of 1",ii,this->MiddleHits[ii]);
	//	}
	//	if( this->OuterHits[ii] > 1 ){
	//		this->console->warn("Linearized MTAS Outer number {} has {} hits in a single event instead of 1",ii,this->OuterHits[ii]);
	//	}
	//}

	this->CurrEvt.FirstTime = *(std::min_element(this->TimeStamps.begin(),this->TimeStamps.end()));
	this->CurrEvt.LastTime = *(std::max_element(this->TimeStamps.begin(),this->TimeStamps.end()));

	for( int ii = 0; ii < 6; ++ii ){
		if( this->CenterHits[2*ii] and this->CenterHits[2*ii + 1] ){
			this->CurrEvt.SumFrontBackEnergy[ii] = (this->Center[2*ii] + this->Center[2*ii + 1])/2.0;
			this->Position[ii] = this->CalcPosition(this->Center[2*ii],this->Center[2*ii + 1]);
			this->CenterFire = true;
		}
		if( this->InnerHits[2*ii] and this->InnerHits[2*ii + 1] ){
			this->CurrEvt.SumFrontBackEnergy[ii+6] = (this->Inner[2*ii] + this->Inner[2*ii + 1])/2.0;
			this->Position[ii + 6] = this->CalcPosition(this->Inner[2*ii],this->Inner[2*ii + 1]);
			this->InnerFire = true;
		}
		if( this->MiddleHits[2*ii] and this->MiddleHits[2*ii + 1] ){
			this->CurrEvt.SumFrontBackEnergy[ii+12] = (this->Middle[2*ii] + this->Middle[2*ii + 1])/2.0;
			this->Position[ii + 12] = this->CalcPosition(this->Middle[2*ii],this->Middle[2*ii + 1]);
			this->MiddleFire = true;
		}
		if( this->OuterHits[2*ii] and this->OuterHits[2*ii + 1] ){
			this->CurrEvt.SumFrontBackEnergy[ii+18] = (this->Outer[2*ii] + this->Outer[2*ii + 1])/2.0;
			this->Position[ii + 18] = this->CalcPosition(this->Outer[2*ii],this->Outer[2*ii + 1]);
			this->OuterFire = true;
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
				this->CurrEvt.SumFrontBackEnergy[ii] = (this->Center[2*ii] + this->Center[2*ii + 1])/2.0;
				//don't update the position
				//this->Position[ii] = this->CalcPosition(this->Center[2*ii],this->Center[2*ii + 1]);
			}
		}	
	}

	for( int ii = 0; ii < 6; ++ii ){
		this->CurrEvt.TotalEnergy[0] += this->CurrEvt.SumFrontBackEnergy[ii];
		this->CurrEvt.TotalEnergy[0] += this->CurrEvt.SumFrontBackEnergy[ii+6];
		this->CurrEvt.TotalEnergy[0] += this->CurrEvt.SumFrontBackEnergy[ii+12];
		this->CurrEvt.TotalEnergy[0] += this->CurrEvt.SumFrontBackEnergy[ii+18];
		
		this->CurrEvt.TotalEnergy[1] += this->CurrEvt.SumFrontBackEnergy[ii];
		this->CurrEvt.TotalEnergy[2] += this->CurrEvt.SumFrontBackEnergy[ii+6];
		this->CurrEvt.TotalEnergy[3] += this->CurrEvt.SumFrontBackEnergy[ii+12];
		this->CurrEvt.TotalEnergy[4] += this->CurrEvt.SumFrontBackEnergy[ii+18];
	}

	if( (not this->CurrEvt.Saturate) and (not this->CurrEvt.Pileup) ){
		hismanager->Fill("MTAS_3200",this->CurrEvt.TotalEnergy[0]);
		
		hismanager->Fill("MTAS_3210",this->CurrEvt.TotalEnergy[1]);
		hismanager->Fill("MTAS_3220",this->CurrEvt.TotalEnergy[2]);
		hismanager->Fill("MTAS_3230",this->CurrEvt.TotalEnergy[3]);
		hismanager->Fill("MTAS_3240",this->CurrEvt.TotalEnergy[4]);

		hismanager->Fill("MTAS_3251",this->CurrEvt.TotalEnergy[0],this->CurrEvt.TotalEnergy[1]);
		hismanager->Fill("MTAS_32518",this->CurrEvt.TotalEnergy[0],this->CurrEvt.TotalEnergy[1]);

		for( int ii = 0; ii < 24; ++ii ){
			hismanager->Fill("MTAS_3201",this->CurrEvt.SumFrontBackEnergy[ii],ii);
		}

		for( int ii = 0; ii < 6; ++ii ){
			std::string id = std::to_string(ii);

			std::string name = "MTAS_326"+id+"_F";
			hismanager->Fill(name,this->Position[ii],this->Center[2*ii]);
			
			name = "MTAS_326"+id+"_B";
			hismanager->Fill(name,this->Position[ii],this->Center[2*ii + 1]);
			
			name = "MTAS_326"+id;
			hismanager->Fill(name,this->Position[ii],this->CurrEvt.SumFrontBackEnergy[ii]);

			hismanager->Fill("MTAS_3215",this->CurrEvt.SumFrontBackEnergy[ii]);
			hismanager->Fill("MTAS_3225",this->CurrEvt.SumFrontBackEnergy[ii+6]);
			hismanager->Fill("MTAS_3235",this->CurrEvt.SumFrontBackEnergy[ii+12]);
			hismanager->Fill("MTAS_3245",this->CurrEvt.SumFrontBackEnergy[ii+18]);
		
			hismanager->Fill("MTAS_3250",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+6]);
			hismanager->Fill("MTAS_3250",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+12]);
			hismanager->Fill("MTAS_3250",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+18]);
			
			hismanager->Fill("MTAS_3252",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);
			
			hismanager->Fill("MTAS_3253",this->CurrEvt.TotalEnergy[1],this->CurrEvt.SumFrontBackEnergy[ii]);
			
			hismanager->Fill("MTAS_32508",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+6]);
			hismanager->Fill("MTAS_32508",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+12]);
			hismanager->Fill("MTAS_32508",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+18]);
			
			hismanager->Fill("MTAS_32528",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);
			
			hismanager->Fill("MTAS_32538",this->CurrEvt.TotalEnergy[1],this->CurrEvt.SumFrontBackEnergy[ii]);

			if( (not this->InnerFire) and (not this->MiddleFire) and (not this->OuterFire) ){
				hismanager->Fill("MTAS_3254",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);
				hismanager->Fill("MTAS_32548",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);
			}
		}
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool MtasProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool MtasProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->Reset();

	return true;
}

void MtasProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void MtasProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
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

	this->diagnosticplots = config.attribute("diagnostic").as_bool(true);

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}
		
void MtasProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void MtasProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
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

	//declare the beta gated and not-beta histograms, but we don't fill them until process after parent has told which we are
	
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

	if( this->diagnosticplots ){
		hismanager->RegisterPlot<TH2F>("MTAS_3431","Raw Individual C PMTs #beta-gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3431));
		hismanager->RegisterPlot<TH2F>("MTAS_3432","Raw Individual I PMTs #beta-gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3432));
		hismanager->RegisterPlot<TH2F>("MTAS_3433","Raw Individual M PMTs #beta-gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3433));
		hismanager->RegisterPlot<TH2F>("MTAS_3434","Raw Individual O PMTs #beta-gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3434));

		hismanager->RegisterPlot<TH2F>("MTAS_3531","Calibrated Individual C PMTs #beta-gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3531));
		hismanager->RegisterPlot<TH2F>("MTAS_3532","Calibrated Individual I PMTs #beta-gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3532));
		hismanager->RegisterPlot<TH2F>("MTAS_3533","Calibrated Individual M PMTs #beta-gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3533));
		hismanager->RegisterPlot<TH2F>("MTAS_3534","Calibrated Individual O PMTs #beta-gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3534));
	}

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

	if( this->diagnosticplots ){
		hismanager->RegisterPlot<TH2F>("MTAS_3411","Raw Individual C PMTs anti-#beta-gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3411));
		hismanager->RegisterPlot<TH2F>("MTAS_3412","Raw Individual I PMTs anti-#beta-gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3412));
		hismanager->RegisterPlot<TH2F>("MTAS_3413","Raw Individual M PMTs anti-#beta-gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3413));
		hismanager->RegisterPlot<TH2F>("MTAS_3414","Raw Individual O PMTs anti-#beta-gated; Energy (channel); PMT (arb.)",this->h2dsettings.at(3414));

		hismanager->RegisterPlot<TH2F>("MTAS_3511","Calibrated Individual C PMTs anti-#beta-gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3511));
		hismanager->RegisterPlot<TH2F>("MTAS_3512","Calibrated Individual I PMTs anti-#beta-gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3512));
		hismanager->RegisterPlot<TH2F>("MTAS_3513","Calibrated Individual M PMTs anti-#beta-gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3513));
		hismanager->RegisterPlot<TH2F>("MTAS_3514","Calibrated Individual O PMTs anti-#beta-gated; Energy (keV); PMT (arb.)",this->h2dsettings.at(3514));
	}

	//center position correction plots
	for( size_t ii = 0; ii < 6; ++ii ){
		std::string name = "MTAS_316"+std::to_string(ii)+"_F";
		std::string title = "Center Position vs C"+std::to_string(ii+1)+"F anti #beta-gated; Position (arb.); Energy (keV)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3160));

		name = "MTAS_316"+std::to_string(ii)+"_B";
		title = "Center Position vs C"+std::to_string(ii+1)+"B anti #beta-gated; Position (arb.); Energy (keV)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3160));

		name = "MTAS_316"+std::to_string(ii);
		title = "Center Position vs C"+std::to_string(ii+1)+" Sum anti #beta-gated; Position (arb.); Energy (keV)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3160));
		
		name = "MTAS_326"+std::to_string(ii)+"_F";
		title = "Center Position vs C"+std::to_string(ii+1)+"F; Position (arb.); Energy (keV)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3260));

		name = "MTAS_326"+std::to_string(ii)+"_B";
		title = "Center Position vs C"+std::to_string(ii+1)+"B; Position (arb.); Energy (keV)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3260));
		
		name = "MTAS_326"+std::to_string(ii);
		title = "Center Position vs C"+std::to_string(ii+1)+" Sum; Position (arb.); Energy (keV)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3260));

		name = "MTAS_336"+std::to_string(ii)+"_F";
		title = "Center Position vs C"+std::to_string(ii+1)+"F #beta-gated; Position (arb.); Energy (keV)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3360));

		name = "MTAS_336"+std::to_string(ii)+"_B";
		title = "Center Position vs C"+std::to_string(ii+1)+"B #beta-gated; Position (arb.); Energy (keV)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3360));
		
		name = "MTAS_336"+std::to_string(ii);
		title = "Center Position vs C"+std::to_string(ii+1)+" Sum #beta-gated; Position (arb.); Energy (keV)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3360));
	}


	this->console->info("Finished Declaring Plots");
}

void MtasProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
	this->OutputTree = new TTree("Mtas","Mtas Processor output");
	this->OutputTree->Branch("segment_vec",&(this->SegmentDataVec));
	this->OutputTree->Branch("total_vec",&(this->TotalDataVec));

	outputtrees[this->ProcessorName] = this->OutputTree;
}

void MtasProcessor::CleanupTree(){
	this->SegmentDataVec.clear();
	this->TotalDataVec.clear();
}

void MtasProcessor::Reset(){
	this->PrevEvt = this->CurrEvt;
	this->CurrEvt = this->NewEvt;

	this->TimeStamps.clear();

	this->Position = std::vector<double>(24,0.0);
	this->Center = std::vector<double>(12,0.0);
	this->Inner = std::vector<double>(12,0.0);
	this->Middle = std::vector<double>(12,0.0);
	this->Outer = std::vector<double>(12,0.0);

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

	this->CenterFire = false;
	this->InnerFire = false;
	this->MiddleFire = false;
	this->OuterFire = false;
}

void MtasProcessor::SetIsBeta(){
	this->CurrEvt.BetaTriggered = true;
}

MtasProcessor::EventInfo& MtasProcessor::GetCurrEvt(){
	return this->CurrEvt;
}

MtasProcessor::EventInfo& MtasProcessor::GetPrevEvt(){
	return this->PrevEvt;
}

void MtasProcessor::FillBetaPlots(PLOTS::PlotRegistry* hismanager) const{
	if( (not this->CurrEvt.Saturate) and (not this->CurrEvt.Pileup) ){
		hismanager->Fill("MTAS_3300",this->CurrEvt.TotalEnergy[0]);

		hismanager->Fill("MTAS_3310",this->CurrEvt.TotalEnergy[1]);
		hismanager->Fill("MTAS_3320",this->CurrEvt.TotalEnergy[2]);
		hismanager->Fill("MTAS_3330",this->CurrEvt.TotalEnergy[3]);
		hismanager->Fill("MTAS_3340",this->CurrEvt.TotalEnergy[4]);

		hismanager->Fill("MTAS_3351",this->CurrEvt.TotalEnergy[0],this->CurrEvt.TotalEnergy[1]);
		hismanager->Fill("MTAS_33518",this->CurrEvt.TotalEnergy[0],this->CurrEvt.TotalEnergy[1]);

		for( int ii = 0; ii < 24; ++ii ){
			hismanager->Fill("MTAS_3301",this->CurrEvt.SumFrontBackEnergy[ii],ii);
		}

		for( int ii = 0; ii < 6; ++ii ){
			std::string id = std::to_string(ii);

			std::string name = "MTAS_336"+id+"_F";
			hismanager->Fill(name,this->Position[ii],this->Center[2*ii]);
			
			name = "MTAS_336"+id+"_B";
			hismanager->Fill(name,this->Position[ii],this->Center[2*ii + 1]);
			
			name = "MTAS_336"+id;
			hismanager->Fill(name,this->Position[ii],this->CurrEvt.SumFrontBackEnergy[ii]);

			hismanager->Fill("MTAS_3315",this->CurrEvt.SumFrontBackEnergy[ii]);
			hismanager->Fill("MTAS_3325",this->CurrEvt.SumFrontBackEnergy[ii+6]);
			hismanager->Fill("MTAS_3335",this->CurrEvt.SumFrontBackEnergy[ii+12]);
			hismanager->Fill("MTAS_3345",this->CurrEvt.SumFrontBackEnergy[ii+18]);

			hismanager->Fill("MTAS_3350",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+6]);
			hismanager->Fill("MTAS_3350",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+12]);
			hismanager->Fill("MTAS_3350",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+18]);

			hismanager->Fill("MTAS_3352",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);
			
			hismanager->Fill("MTAS_3353",this->CurrEvt.TotalEnergy[1],this->CurrEvt.SumFrontBackEnergy[ii]);

			hismanager->Fill("MTAS_33508",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+6]);
			hismanager->Fill("MTAS_33508",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+12]);
			hismanager->Fill("MTAS_33508",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+18]);

			hismanager->Fill("MTAS_33528",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);
			
			hismanager->Fill("MTAS_33538",this->CurrEvt.TotalEnergy[1],this->CurrEvt.SumFrontBackEnergy[ii]);
			
			if( (not this->InnerFire) and (not this->MiddleFire) and (not this->OuterFire) ){
				hismanager->Fill("MTAS_3354",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);
				hismanager->Fill("MTAS_33548",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);
			}

			if( this->diagnosticplots ){
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

void MtasProcessor::FillNonBetaPlots(PLOTS::PlotRegistry* hismanager) const{
	if( (not this->CurrEvt.Saturate) and (not this->CurrEvt.Pileup) ){
		hismanager->Fill("MTAS_3100",this->CurrEvt.TotalEnergy[0]);

		hismanager->Fill("MTAS_3110",this->CurrEvt.TotalEnergy[1]);
		hismanager->Fill("MTAS_3120",this->CurrEvt.TotalEnergy[2]);
		hismanager->Fill("MTAS_3130",this->CurrEvt.TotalEnergy[3]);
		hismanager->Fill("MTAS_3140",this->CurrEvt.TotalEnergy[4]);

		hismanager->Fill("MTAS_3151",this->CurrEvt.TotalEnergy[0],this->CurrEvt.TotalEnergy[1]);
		hismanager->Fill("MTAS_31518",this->CurrEvt.TotalEnergy[0],this->CurrEvt.TotalEnergy[1]);

		for( int ii = 0; ii < 24; ++ii ){
			hismanager->Fill("MTAS_3101",this->CurrEvt.SumFrontBackEnergy[ii],ii);
		}

		for( int ii = 0; ii < 6; ++ii ){
			std::string id = std::to_string(ii);

			std::string name = "MTAS_316"+id+"_F";
			hismanager->Fill(name,this->Position[ii],this->Center[2*ii]);
			
			name = "MTAS_316"+id+"_B";
			hismanager->Fill(name,this->Position[ii],this->Center[2*ii + 1]);
			
			name = "MTAS_316"+id;
			hismanager->Fill(name,this->Position[ii],this->CurrEvt.SumFrontBackEnergy[ii]);

			hismanager->Fill("MTAS_3115",this->CurrEvt.SumFrontBackEnergy[ii]);
			hismanager->Fill("MTAS_3125",this->CurrEvt.SumFrontBackEnergy[ii+6]);
			hismanager->Fill("MTAS_3135",this->CurrEvt.SumFrontBackEnergy[ii+12]);
			hismanager->Fill("MTAS_3145",this->CurrEvt.SumFrontBackEnergy[ii+18]);

			hismanager->Fill("MTAS_3150",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+6]);
			hismanager->Fill("MTAS_3150",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+12]);
			hismanager->Fill("MTAS_3150",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+18]);

			hismanager->Fill("MTAS_3152",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);
			
			hismanager->Fill("MTAS_3153",this->CurrEvt.TotalEnergy[1],this->CurrEvt.SumFrontBackEnergy[ii]);

			hismanager->Fill("MTAS_31508",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+6]);
			hismanager->Fill("MTAS_31508",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+12]);
			hismanager->Fill("MTAS_31508",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+18]);

			hismanager->Fill("MTAS_31528",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);
			
			hismanager->Fill("MTAS_31538",this->CurrEvt.TotalEnergy[1],this->CurrEvt.SumFrontBackEnergy[ii]);
			
			if( (not this->InnerFire) and (not this->MiddleFire) and (not this->OuterFire) ){
				hismanager->Fill("MTAS_3154",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);
				hismanager->Fill("MTAS_31548",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);
			}

			if( this->diagnosticplots ){
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
