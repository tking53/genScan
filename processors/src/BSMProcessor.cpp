#include "BSMProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <string>

namespace PulseFit{

	double BSMSingleTraceFit(double* x,double* par){
		return PulseFit::Constant(x,par)+PulseFit::Sin(x,par+1)+PulseFit::Pulse(x,par+2);
	}

	double BSMDoubleTraceFit(double* x,double* par){
		return PulseFit::Constant(x,par)+PulseFit::Sin(x,par+1)+PulseFit::Pulse(x,par+3)+PulseFit::Pulse(x,par+7);
	}

}

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
				{3611 , {16384,0.0,16384}},
				{3612 , {16384,0.0,16384}}
			    };

	this->h2dsettings = {
				{3620 , {1024,-1.0,1.0,8192,0.0,8192.0}},
				{3650 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36508 , {2048,0.0,16384.0,2048,0.0,16384.0}},
				{3700 , {512,0,512,16384,0.0,16384.0}},
				{3710 , {16384,0.0,16384.0,1024,0,1.0}},
				{3720 , {1024,0,32,16384,0.0,16384.0}},
				{3730 , {1024,0,1024,16384,0.0,16384.0}},
				{3740 , {1024,0,1024,1024,0,32}},
				{3750 , {16384,0,16384.0,1024,0,1024}},
				{3800 , {512,0,512,16384,0.0,16384.0}},
				{3810 , {16384,0.0,16384.0,1024,0,1.0}},
				{3820 , {1024,0,32,16384,0.0,16384.0}},
				{3830 , {1024,0,1024,16384,0.0,16384.0}},
				{3840 , {1024,0,1024,1024,0,32}}
			    };
	
	this->BSMHits = std::vector<int>(12,0);
	for( size_t ii = 0; ii < 12; ++ii ){
		this->TraceSettings.push_back(nullptr);
		this->PosCorrectionMap.push_back(nullptr);
	}
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
		int position = segmentid - 1; 	
		int detectorposition = 2*position + isback;

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
				std::string tracehis = (isfront) ? ("BSM_380"+std::to_string(position)+"_F") :  ("BSM_380"+std::to_string(position)+"_B");
				size_t idx = 0;
				for( const auto& tracevalue : evt->GetRawTrace() ){
					hismanager->Fill(tracehis,idx,tracevalue);
					++idx;
				}

				auto psdvals = evt->GetTraceFixedPSD();

				auto integral = std::get<0>(psdvals)+std::get<1>(psdvals);
				auto tmax = evt->GetBaselineSubtractedMaxValue();
				auto psd = tmax/integral;

				std::string psdhis = (isfront) ?  ("BSM_381"+std::to_string(position)+"_F") :  ("BSM_381"+std::to_string(position)+"_B");
				hismanager->Fill(psdhis,integral,psd);

				std::string baselinehis = (isfront) ?  ("BSM_382"+std::to_string(position)+"_F") :  ("BSM_382"+std::to_string(position)+"_B");
				auto baseline = evt->GetTracePreTriggerBaseline();
				hismanager->Fill(baselinehis,baseline.second,baseline.first);

				std::string peakhis = (isfront) ?  ("BSM_383"+std::to_string(position)+"_F") :  ("BSM_383"+std::to_string(position)+"_B");
				auto pk = evt->GetTraceMaxInfo();
				hismanager->Fill(peakhis,pk.first,pk.second);

				std::string pklocbstddevhis = (isfront) ?  ("BSM_384"+std::to_string(position)+"_F") :  ("BSM_384"+std::to_string(position)+"_B");
				hismanager->Fill(pklocbstddevhis,pk.first,baseline.second);
			}
			if( evt->GetSaturation() ){
				this->CurrEvt.Saturate = true;
			}
			continue;
		}

		if( !this->BSMHits[detectorposition] ){
			if( this->PlotAllTraces ){
				std::string tracehis = (isfront) ? ("BSM_370"+std::to_string(position)+"_F") :  ("BSM_370"+std::to_string(position)+"_B");
				size_t idx = 0;
				for( const auto& tracevalue : evt->GetRawTrace() ){
					hismanager->Fill(tracehis,idx,tracevalue);
					++idx;
				}
			}

			auto psdvals = evt->GetTraceFixedPSD();

			auto integral = std::get<0>(psdvals)+std::get<1>(psdvals);
			auto tmax = evt->GetBaselineSubtractedMaxValue();
			auto psd = tmax/integral;
			
			std::string psdhis = (isfront) ?  ("BSM_371"+std::to_string(position)+"_F") :  ("BSM_371"+std::to_string(position)+"_B");
			hismanager->Fill(psdhis,integral,psd);

			std::string baselinehis = (isfront) ?  ("BSM_372"+std::to_string(position)+"_F") :  ("BSM_372"+std::to_string(position)+"_B");
			auto baseline = evt->GetTracePreTriggerBaseline();
			hismanager->Fill(baselinehis,baseline.second,baseline.first);
			
			std::string peakhis = (isfront) ?  ("BSM_373"+std::to_string(position)+"_F") :  ("BSM_373"+std::to_string(position)+"_B");
			auto pk = evt->GetTraceMaxInfo();
			hismanager->Fill(peakhis,pk.first,pk.second);
			
			std::string pklocbstddevhis = (isfront) ?  ("BSM_374"+std::to_string(position)+"_F") :  ("BSM_374"+std::to_string(position)+"_B");
			hismanager->Fill(pklocbstddevhis,pk.first,baseline.second);
			
			std::string pkmaxerghis = (isfront) ?  ("BSM_375"+std::to_string(position)+"_F") :  ("BSM_375"+std::to_string(position)+"_B");
			hismanager->Fill(pkmaxerghis,evt->GetRawEnergyWRandom(),pk.first);
			
			this->CurrEvt.UnCorrectedBSM[detectorposition] = evt->GetEnergy();
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
			if( (this->PosCorrectionMap[2*ii] != nullptr) and (this->PosCorrectionMap[2*ii + 1] != nullptr ) ){
				auto front = this->PosCorrectionMap[2*ii]->Correct(this->CurrEvt.UnCorrectedBSM[2*ii],this->CurrEvt.Position[ii]);
				auto back = this->PosCorrectionMap[2*ii + 1]->Correct(this->CurrEvt.UnCorrectedBSM[2*ii + 1],this->CurrEvt.Position[ii]);
				this->CurrEvt.SumFrontBackEnergy[ii] = (front + back)/2.0;
			}else{
				this->CurrEvt.SumFrontBackEnergy[ii] = (this->CurrEvt.UnCorrectedBSM[2*ii] + this->CurrEvt.UnCorrectedBSM[2*ii + 1])/2.0;
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
	this->LoadCustomCuts(config);
}

void BSMProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}

void BSMProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
	this->PlotAllTraces = config.attribute("PlotAllTraces").as_bool(false);
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
				this->TraceSettings[id].reset(new TraceAnalysis());

				this->TraceSettings[id]->lowerbound = trace.attribute("lowerbound").as_int(20);
				this->TraceSettings[id]->upperbound = trace.attribute("upperbound").as_int(50);
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

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
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
	hismanager->RegisterPlot<TH1F>("BSM_3611","#betaSM Total [MTAS Pileup]; Energy (keV)",this->h1dsettings.at(3611));
	hismanager->RegisterPlot<TH1F>("BSM_3612","#betaSM Total [MTAS Saturate]; Energy (keV)",this->h1dsettings.at(3612));

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

		if( this->PlotAllTraces ){
			name = "BSM_370"+std::to_string(ii)+"_F";
			title = "#betaSM"+std::to_string(ii+1)+"_F Trace; Clock Ticks (arb.); adc (arb.)";
			hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3700));

			name = "BSM_370"+std::to_string(ii)+"_B";
			title = "#betaSM"+std::to_string(ii+1)+"_B Trace; Clock Ticks (arb.); adc (arb.)";
			hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3700));
		}
		
		name = "BSM_371"+std::to_string(ii)+"_F";
		title = "#betaSM"+std::to_string(ii+1)+"_F (Peak/Integral) vs Integral; Integral (arb.); Ratio (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3710));
		
		name = "BSM_371"+std::to_string(ii)+"_B";
		title = "#betaSM"+std::to_string(ii+1)+"_B (Peak/Integral) vs Integral; Integral (arb.); Ratio (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3710));
		
		name = "BSM_372"+std::to_string(ii)+"_F";
		title = "#betaSM"+std::to_string(ii+1)+"_F (Peak/Integral) vs Integral; StdDev. (arb.); Avg. (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3720));
		
		name = "BSM_372"+std::to_string(ii)+"_B";
		title = "#betaSM"+std::to_string(ii+1)+"_B Baseline Avg vs Baseline StdDev; StdDev. (arb.); Avg. (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3720));

		name = "BSM_373"+std::to_string(ii)+"_F";
		title = "#betaSM"+std::to_string(ii+1)+"_F Max Peak Value vs Peak Location; Peak Location (ticks); Max (adc)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3730));
		
		name = "BSM_373"+std::to_string(ii)+"_B";
		title = "#betaSM"+std::to_string(ii+1)+"_B Max Peak Value vs Peak Location; Peak Location (ticks); Max (adc)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3730));

		name = "BSM_374"+std::to_string(ii)+"_F";
		title = "#betaSM"+std::to_string(ii+1)+"_F Baseline StdDev vs Peak Location; Peak Location (ticks); StdDev. (arb.);";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3740));
		
		name = "BSM_374"+std::to_string(ii)+"_B";
		title = "#betaSM"+std::to_string(ii+1)+"_B Baseline StdDev vs Peak Location; Peak Location (ticks); StdDev. (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3740));

		name = "BSM_375"+std::to_string(ii)+"_F";
		title = "#betaSM"+std::to_string(ii+1)+"_F Peak Max Location vs Raw Energy; Energy (channel); Peak Max Location (ticks);";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3750));
		
		name = "BSM_375"+std::to_string(ii)+"_B";
		title = "#betaSM"+std::to_string(ii+1)+"_B Peak Max Location vs Raw Energy; Energy (channel); Peak Max Location (ticks);";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3750));

		name = "BSM_380"+std::to_string(ii)+"_F";
		title = "#betaSM"+std::to_string(ii+1)+"_F Pileup Trace; Clock Ticks (arb.); adc (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3800));

		name = "BSM_380"+std::to_string(ii)+"_B";
		title = "#betaSM"+std::to_string(ii+1)+"_B Pileup Trace; Clock Ticks (arb.); adc (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3800));

		name = "BSM_381"+std::to_string(ii)+"_F";
		title = "#betaSM"+std::to_string(ii+1)+"_F Pileup Trace (Peak/Integral) vs Integral; Integral (arb.); Ratio (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3810));
		
		name = "BSM_381"+std::to_string(ii)+"_B";
		title = "#betaSM"+std::to_string(ii+1)+"_B Pileup Trace (Peak/Integral) vs Integral; Integral (arb.); Ratio (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3810));
		
		name = "BSM_382"+std::to_string(ii)+"_F";
		title = "#betaSM"+std::to_string(ii+1)+"_F Pileup Trace (Peak/Integral) vs Integral; StdDev. (arb.); Avg. (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3820));
		
		name = "BSM_382"+std::to_string(ii)+"_B";
		title = "#betaSM"+std::to_string(ii+1)+"_B Pileup Trace Baseline Avg vs Baseline StdDev; StdDev. (arb.); Avg. (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3820));

		name = "BSM_383"+std::to_string(ii)+"_F";
		title = "#betaSM"+std::to_string(ii+1)+"_F Pileup Trace Max Peak Value vs Peak Location; Peak Location (ticks); Max (adc)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3830));
		
		name = "BSM_383"+std::to_string(ii)+"_B";
		title = "#betaSM"+std::to_string(ii+1)+"_B Pileup Trace Max Peak Value vs Peak Location; Peak Location (ticks); Max (adc)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3830));

		name = "BSM_384"+std::to_string(ii)+"_F";
		title = "#betaSM"+std::to_string(ii+1)+"_F Pileup Trace Baseline StdDev vs Peak Location; Peak Location (ticks); StdDev. (arb.);";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3840));
		
		name = "BSM_384"+std::to_string(ii)+"_B";
		title = "#betaSM"+std::to_string(ii+1)+"_B Pileup Trace Baseline StdDev vs Peak Location; Peak Location (ticks); StdDev. (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3840));
	}
	
	hismanager->RegisterPlot<TH2F>("BSM_3650","#betaSM Total vs MTAS Total; MTAS Total Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3650));

	hismanager->RegisterPlot<TH2F>("BSM_36508","#betaSM Total vs MTAS Total; MTAS Total Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36508));

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
