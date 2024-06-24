#include "BSMProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>

BSMProcessor::BSMProcessor(const std::string& log) : Processor(log,"BSMProcessor",{"bsm"}){
	this->NewEvt = {
		.TotalEnergy = 0.0,
		.SumFrontBackEnergy = std::vector<double>(6,0.0),
		.Position = std::vector<double>(6,0.0),
		.UnCorrectedTotalEnergy = 0.0,
		.UnCorrectedSumFrontBackEnergy = std::vector<double>(6,0.0),
		.FirstTime = -1.0,
		.LastTime = -1.0,
		.Saturate = false,
		.Pileup = false,
		.RealEvt = false
	};
	this->PrevEvt = this->NewEvt;
	this->CurrEvt = this->NewEvt;

	this->fronttag = "front";
	this->backtag = "back";
	this->foundfirstevt = false;
	this->globalfirsttime = 0.0;

	this->h1dsettings = { 
				{3600 , {16384,0.0,16384}},
				{3601 , {16384,0.0,16384}},
				{3602 , {16384,0.0,16384}}
			    };

	this->h2dsettings = {
				{3650 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36508 , {2048,0.0,16384.0,2048,0.0,16384.0}}
			    };
	
	this->UnCorrectedBSM = std::vector<double>(12,0.0);
	this->BSMHits = std::vector<int>(12,0);
}

[[maybe_unused]] bool BSMProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	summary.GetDetectorSummary(this->AllDefaultRegex["bsm"],this->SummaryData);
	bool setfirsttime = false;
	double firsttime = 0.0;
	double lasttime = 0.0;
	for( const auto& evt : this->SummaryData ){
		auto subtype = evt->GetSubType();
		auto group = evt->GetGroup();

		auto isfront = evt->HasTag(fronttag);
		auto isback = evt->HasTag(backtag);

		//this->console->info("{} {} {}",subtype,group,group.size());
		
		int segmentid = std::stoi(group);

		if( (not isfront and not isback) or (isfront and isback) ){
			throw std::runtime_error("evt in MtasProcessor is malformed in xml, and has either both front and back tag or neither");
		}

		if( not setfirsttime ){
			setfirsttime = true;
			firsttime = evt->GetTimeStamp();
		}
		lasttime = evt->GetTimeStamp();
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

		if( !this->BSMHits[detectorposition] ){
			this->UnCorrectedBSM[detectorposition] += evt->GetEnergy();
			++this->BSMHits[detectorposition];
		}else{
			++this->BSMHits[detectorposition];
		}
	}
	//for( size_t ii = 0; ii < this->BSMHits.size(); ++ii ){
	//	if( this->BSMHits[ii] > 1 ){
	//		this->console->warn("Linearized BSM number {} has {} hits in a single event instead of 1",ii,this->BSMHits[ii]);
	//	}
	//}
	
	this->CurrEvt.FirstTime = firsttime;
	this->CurrEvt.LastTime = lasttime;

	for( int ii = 0; ii < 6; ++ii ){
		if( this->BSMHits[2*ii] and this->BSMHits[2*ii + 1] ){
			this->CurrEvt.UnCorrectedSumFrontBackEnergy[ii] = (this->UnCorrectedBSM[2*ii] + this->UnCorrectedBSM[2*ii + 1])/2.0;
			this->CurrEvt.Position[ii] = this->CalcPosition(this->UnCorrectedBSM[2*ii],this->UnCorrectedBSM[2*ii + 1]);
			if( (this->PosCorrectionMap.find(2*ii) != this->PosCorrectionMap.end()) and (this->PosCorrectionMap.find(2*ii + 1) != this->PosCorrectionMap.end() ) ){
				auto front = this->PosCorrectionMap[2*ii].Correct(this->UnCorrectedBSM[2*ii],this->CurrEvt.Position[ii]);
				auto back = this->PosCorrectionMap[2*ii + 1].Correct(this->UnCorrectedBSM[2*ii + 1],this->CurrEvt.Position[ii]);
				this->CurrEvt.SumFrontBackEnergy[ii] += (front + back)/2.0;
			}else{
				this->CurrEvt.SumFrontBackEnergy[ii] += (this->UnCorrectedBSM[2*ii] + this->UnCorrectedBSM[2*ii + 1])/2.0;
			}
		}
	}

	for( int ii = 0; ii < 6; ++ii ){
		this->CurrEvt.TotalEnergy += this->CurrEvt.SumFrontBackEnergy[ii];
		this->CurrEvt.UnCorrectedTotalEnergy += this->CurrEvt.UnCorrectedSumFrontBackEnergy[ii];
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool BSMProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool BSMProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->Reset();

	return true;
}

void BSMProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
	this->LoadHistogramSettings(config);
}

void BSMProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
	this->LoadHistogramSettings(config);
}

void BSMProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
	this->LoadHistogramSettings(config);
}
		
void BSMProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void BSMProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	//BSM diagnostic plots, always want these no matter what
	hismanager->RegisterPlot<TH1F>("BSM_3600","#betaSM Total; Energy (keV)",this->h1dsettings.at(3600));
	hismanager->RegisterPlot<TH1F>("BSM_3601","#betaSM Total No MTAS; Energy (keV)",this->h1dsettings.at(3601));
	hismanager->RegisterPlot<TH1F>("BSM_3602","#betaSM Total + MTAS Total; Energy (keV)",this->h1dsettings.at(3602));
	
	hismanager->RegisterPlot<TH2F>("BSM_3650","#betaSM Total vs MTAS Total; MTAS Total Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3650));

	hismanager->RegisterPlot<TH2F>("BSM_36508","#betaSM Total vs MTAS Total; MTAS Total Energy (8 keV/bin) #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36508));

	this->console->info("Finished Declaring Plots");
}

void BSMProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
}

void BSMProcessor::CleanupTree(){
}

void BSMProcessor::Reset(){
	this->PrevEvt = this->CurrEvt;
	this->CurrEvt = this->NewEvt;
	this->UnCorrectedBSM = std::vector<double>(12,0.0);
	this->BSMHits = std::vector<int>(12,0);
}

double BSMProcessor::CalcPosition(double front,double back){
	return (front - back)/(front + back);
}

BSMProcessor::EventInfo& BSMProcessor::GetCurrEvt(){
	return this->CurrEvt;
}

BSMProcessor::EventInfo& BSMProcessor::GetPrevEvt(){
	return this->PrevEvt;
}
