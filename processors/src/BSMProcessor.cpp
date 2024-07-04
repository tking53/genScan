#include "BSMProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <string>

BSMProcessor::BSMProcessor(const std::string& log) : Processor(log,"BSMProcessor",{"bsm"}){
	this->NewEvt = EventInfo();
	this->PrevEvt = this->NewEvt;
	this->CurrEvt = this->NewEvt;

	this->fronttag = "front";
	this->backtag = "back";
	this->foundfirstevt = false;
	this->globalfirsttime = 0.0;

	this->h1dsettings = { 
				{3600 , {16384,0.0,16384}},
				{3601 , {16384,0.0,16384}},
				{3602 , {16384,0.0,16384}},
				{3610 , {16384,0.0,16384}},
				{3611 , {16384,0.0,16384}}
			    };

	this->h2dsettings = {
				{3620 , {1024,-1.0,1.0,8192,0.0,8192.0}},
				{3630 , {8192,0.0,8192.0,1024,0.0,1.0}},
				{3650 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36508 , {2048,0.0,16384.0,2048,0.0,16384.0}}
			    };
	
	this->BSMHits = std::vector<int>(12,0);
	this->TraceSettings = std::vector<TraceAnalysis*>(12,nullptr);
}

[[maybe_unused]] bool BSMProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	summary.GetDetectorSummary(this->AllDefaultRegex["bsm"],this->SummaryData);
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
			this->CurrEvt.UnCorrectedBSM[detectorposition] += evt->GetEnergy();
			this->TimeStamps.push_back(evt->GetTimeStamp());
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
	
	this->CurrEvt.FirstTime = *(std::min_element(this->TimeStamps.begin(),this->TimeStamps.end()));
	this->CurrEvt.LastTime = *(std::max_element(this->TimeStamps.begin(),this->TimeStamps.end()));

	for( int ii = 0; ii < 6; ++ii ){
		if( this->BSMHits[2*ii] and this->BSMHits[2*ii + 1] ){
			++this->CurrEvt.NumValidSegments;
			this->CurrEvt.UnCorrectedSumFrontBackEnergy[ii] = (this->CurrEvt.UnCorrectedBSM[2*ii] + this->CurrEvt.UnCorrectedBSM[2*ii + 1])/2.0;
			this->CurrEvt.Position[ii] = this->CalcPosition(this->CurrEvt.UnCorrectedBSM[2*ii],this->CurrEvt.UnCorrectedBSM[2*ii + 1]);
			if( (this->PosCorrectionMap.find(2*ii) != this->PosCorrectionMap.end()) and (this->PosCorrectionMap.find(2*ii + 1) != this->PosCorrectionMap.end() ) ){
				auto front = this->PosCorrectionMap[2*ii].Correct(this->CurrEvt.UnCorrectedBSM[2*ii],this->CurrEvt.Position[ii]);
				auto back = this->PosCorrectionMap[2*ii + 1].Correct(this->CurrEvt.UnCorrectedBSM[2*ii + 1],this->CurrEvt.Position[ii]);
				this->CurrEvt.SumFrontBackEnergy[ii] += (front + back)/2.0;
			}else{
				this->CurrEvt.SumFrontBackEnergy[ii] += (this->CurrEvt.UnCorrectedBSM[2*ii] + this->CurrEvt.UnCorrectedBSM[2*ii + 1])/2.0;
			}
		
			std::string id = std::to_string(ii);
			std::string name = "BSM_362"+id+"_F";

			hismanager->Fill(name,this->CurrEvt.Position[ii],this->CurrEvt.UnCorrectedBSM[2*ii]);
			
			name = "BSM_362"+id+"_B";
			hismanager->Fill(name,this->CurrEvt.Position[ii],this->CurrEvt.UnCorrectedBSM[2*ii + 1]);
			
			name = "BSM_362"+id;
			hismanager->Fill(name,this->CurrEvt.Position[ii],this->CurrEvt.SumFrontBackEnergy[ii]);
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
	for( pugi::xml_node trace = config.child("PulseAnalysis"); trace; trace = trace.next_sibling("PulseAnalysis") ){
		int id = trace.attribute("id").as_int(-1);
		if( id >= this->TraceSettings.size() or id < 0 ){
			std::string mess = "Invalid PulseAnalysis Setting in BSMProcessor id="+std::to_string(id)+" does not fall within the range [0,"+std::to_string(this->TraceSettings.size())+")";
			throw mess;
		}else{
			if( this->TraceSettings[id] != nullptr ){
				std::string mess = "Duplicate PulseAnalysis Setting in BSMProcessor id="+std::to_string(id);
				throw mess;
			}else{
				this->TraceSettings[id] = new TraceAnalysis();

				this->TraceSettings[id]->lowerbound = trace.attribute("lowerbound").as_int(-1);
				this->TraceSettings[id]->upperbound = trace.attribute("upperbound").as_int(-1);
				this->TraceSettings[id]->lowthresh = trace.attribute("lowthresh").as_float(0.0);
				this->TraceSettings[id]->highthresh = trace.attribute("highthresh").as_float(1.0e10);
				this->TraceSettings[id]->integralthreshold = trace.attribute("integralthreshold").as_float(0.0);
				if( not this->TraceSettings[id]->ValidateSelf() ){
					std::string mess = "PulseAnalysis Setting in BSMProcessor for id="+std::to_string(id)+" is misconfigured, doesn't pass (lowerbound < upperbound) and (lowerbound >= 0) and (upperbound >= 0) and (lowthresh <= highthresh)";
					throw mess;
				}
			}
		}
	}
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
	hismanager->RegisterPlot<TH1F>("BSM_3610","#betaSM Total; Energy (keV)",this->h1dsettings.at(3610));
	hismanager->RegisterPlot<TH1F>("BSM_3611","#betaSM Total [MTAS Pileup|MTAS Saturate]; Energy (keV)",this->h1dsettings.at(3611));

	for( size_t ii = 0; ii < 6; ++ii ){
		std::string name = "BSM_362"+std::to_string(ii)+"_F";
		std::string title = "#betaSM"+std::to_string(ii+1)+"_F Energy vs #betaSM Position; Position (arb.); Energy (keV)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3620));
		
		name = "BSM_362"+std::to_string(ii)+"_B";
		title = "#betaSM"+std::to_string(ii+1)+"_B Energy vs #betaSM Position; Position (arb.); Energy (keV)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3620));
		
		name = "BSM_362"+std::to_string(ii);
		title = "#betaSM"+std::to_string(ii+1)+" Energy vs #betaSM Position; Position (arb.); Energy (keV)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3620));
		
		name = "BSM_363"+std::to_string(ii)+"_F";
		title = "#betaSM"+std::to_string(ii+1)+"_F PSD vs #betaSM Integral; Integral (arb.); PSD (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3630));
		
		name = "BSM_363"+std::to_string(ii)+"_B";
		title = "#betaSM"+std::to_string(ii+1)+"_B PSD vs #betaSM Integral; Integral (arb.); PSD (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3630));
	}
	
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
	this->TimeStamps.clear();
	this->BSMHits = std::vector<int>(12,0);
	for( const auto& t : this->TraceSettings ){
		if( t != nullptr ){
			t->Reset();
		}
	}
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
