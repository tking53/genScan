#include "BSMProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <algorithm>
#include <string>
#include <tuple>

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
				{3603 , {16384,0.0,16384}},
				{3604 , {16384,0.0,16384}},
				{3605 , {16384,0.0,16384}},
				{3610 , {16384,0.0,16384}},
				{3611 , {16384,0.0,16384}},
				{3612 , {16384,0.0,16384}}
			    };

	this->h2dsettings = {
				{3620 , {1024,-1.0,1.0,8192,0.0,8192.0}},
				{3630 , {256,-1024,1023,8192,0.0,8192.0}},
				{3650 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36508 , {2048,0.0,16384.0,2048,0.0,16384.0}},
				{3651 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36518 , {2048,0.0,16384.0,2048,0.0,16384.0}},
				{3652 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36528 , {2048,0.0,16384.0,2048,0.0,16384.0}},
				{3653 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36538 , {2048,0.0,16384.0,2048,0.0,16384.0}},
				{3654 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36548 , {2048,0.0,16384.0,2048,0.0,16384.0}},
				{3655 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36558 , {2048,0.0,16384.0,2048,0.0,16384.0}},
				{3656 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36568 , {2048,0.0,16384.0,2048,0.0,16384.0}},
				{3657 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36578 , {2048,0.0,16384.0,2048,0.0,16384.0}},
				{3658 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36588 , {2048,0.0,16384.0,2048,0.0,16384.0}},
				{3660 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36608 , {2048,0.0,16384.0,2048,0.0,16384.0}},
				{3661 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36618 , {2048,0.0,16384.0,2048,0.0,16384.0}},
				{3670 , {4096,0.0,4096.0,4096,0.0,4096.0}},
				{36708 , {2048,0.0,16384.0,2048,0.0,16384.0}},
				{3700 , {512,0,512,16384,0.0,16384.0}},
				{3710 , {16384,0.0,16384.0,1024,0,10.0}},
				{3720 , {1024,0,32,16384,0.0,16384.0}},
				{3730 , {1024,0,1024,16384,0.0,16384.0}},
				{3740 , {1024,0,1024,8192,0,128}},
				{3750 , {16384,0,16384.0,1024,0,1024}},
				{3760 , {512,0,512,16384,-16384.0,16384.0}},
				{3800 , {512,0,512,16384,0.0,16384.0}},
				{3810 , {16384,0.0,16384.0,1024,0,10.0}},
				{3820 , {1024,0,32,16384,0.0,16384.0}},
				{3830 , {1024,0,1024,16384,0.0,16384.0}},
				{3840 , {1024,0,1024,8192,0,128}},
				{3860 , {512,0,512,16384,-16384.0,16384.0}},
				{3900 , {512,0,512,16384,0.0,16384.0}},
				
				{4000 , {8192,0,8192.0,1024,0.0,1024.0}},
				{4001 , {8192,0,8192.0,1024,0.0,1024.0}},
				{4002 , {8192,0,8192.0,1024,0.0,1024.0}},
				{4003 , {8192,0,8192.0,1024,0.0,1024.0}},
				{4004 , {8192,0,8192.0,1024,0.0,1024.0}}
			    };
	
	this->NumPairs = 1;
	this->NumPMTs = 2*this->NumPairs;
}

[[maybe_unused]] bool BSMProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	summary.GetDetectorSummary(this->AllDefaultRegex["bsm"],this->SummaryData);

	//std::vector<std::vector<PhysicsData*>> FrontHits = std::vector<std::vector<PhysicsData*>>(6,std::vector<PhysicsData*>());
	//std::vector<std::vector<PhysicsData*>> BackHits = std::vector<std::vector<PhysicsData*>>(6,std::vector<PhysicsData*>());

	//for( auto& evt : this->SummaryData ){
	//	auto group = evt->GetGroup();
	//	int segmentid = std::stoi(group);
	//	int position = segmentid - 1; 	
	//	if( evt->HasTag(fronttag) ){
	//		FrontHits[position].push_back(evt);
	//	}else if( evt->HasTag(backtag) ){
	//		BackHits[position].push_back(evt);
	//	}else{
	//		throw std::runtime_error("evt in BSMProcessor is malformed in xml, and has neither front tag or back tag");
	//	}
	//}

	//std::vector<PhysicsData*> GoodHits;
	//for( size_t ii = 0; ii < this->NumPairs; ++ii ){
	//	std::vector<std::tuple<size_t,size_t,double>> TDiff;
	//	for( size_t jj = 0; jj < FrontHits[ii].size(); ++jj ){
	//		for( size_t kk = 0; kk < BackHits[ii].size(); ++kk ){
	//			TDiff.push_back(std::make_tuple(jj,kk,std::abs(FrontHits[ii][jj]->GetTimeStamp() - BackHits[ii][kk]->GetTimeStamp())));
	//		}
	//	}

	//	for( const auto& e : TDiff ){
	//		this->console->info("before: {} {} {}",std::get<0>(e),std::get<1>(e),std::get<2>(e));
	//	}

	//	std::sort(TDiff.begin(),TDiff.end(),[](const std::tuple<size_t,size_t,double>& a,const std::tuple<size_t,size_t,double>& b){
	//			return std::get<2>(a) < std::get<2>(b);
	//			} );

	//	for( const auto& e : TDiff ){
	//		this->console->info("after: {} {} {}",std::get<0>(e),std::get<1>(e),std::get<2>(e));
	//	}
	//	//throw "help";
	//}
	
	//if( this->SummaryData.size() > 2 ){
	//	this->console->info("{}",this->SummaryData.size());
	//}
	
	for( const auto& evt : this->SummaryData ){
		auto subtype = evt->GetSubType();
		auto group = evt->GetGroup();

		auto isfront = evt->HasTag(fronttag);
		auto isback = evt->HasTag(backtag);

		//this->console->info("{} {} {}",subtype,group,group.size());
		
		int segmentid = std::stoi(group);
		int position = segmentid - 1; 	
		int detectorposition = 2*position + isback;

		++(this->TotalMult[detectorposition]);

		if( (not isfront and not isback) or (isfront and isback) ){
			throw std::runtime_error("evt in BSMProcessor is malformed in xml, and has either both front and back tag or neither");
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

				std::string tracederivativehis = (isfront) ? ("BSM_386"+std::to_string(position)+"_F") :  ("BSM_386"+std::to_string(position)+"_B");
				idx = 0;
				for( const auto& tracevalue : evt->GetTraceDerivative() ){
					hismanager->Fill(tracederivativehis,idx,tracevalue);
					++idx;
				}

				auto psdvals = evt->GetTraceFixedPSD();

				auto integral = std::get<0>(psdvals)+std::get<1>(psdvals);
				auto tmax = evt->GetBaselineSubtractedPSDBoundedMaxValue();
				auto psd = tmax/integral;
				auto baseline = evt->GetTracePreTriggerBaseline();
				auto pk = evt->GetPSDBoundedTraceMaxInfo();

				std::string psdhis = (isfront) ?  ("BSM_381"+std::to_string(position)+"_F") :  ("BSM_381"+std::to_string(position)+"_B");
				hismanager->Fill(psdhis,integral,psd);

				std::string baselinehis = (isfront) ?  ("BSM_382"+std::to_string(position)+"_F") :  ("BSM_382"+std::to_string(position)+"_B");
				hismanager->Fill(baselinehis,baseline.second,baseline.first);

				std::string peakhis = (isfront) ?  ("BSM_383"+std::to_string(position)+"_F") :  ("BSM_383"+std::to_string(position)+"_B");
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
			if( this->Pairs[detectorposition] == nullptr ){
				this->Pairs[detectorposition] = evt;
			}
			if( this->PlotAllTraces ){
				std::string tracehis = (isfront) ? ("BSM_370"+std::to_string(position)+"_F") :  ("BSM_370"+std::to_string(position)+"_B");
				size_t idx = 0;
				for( const auto& tracevalue : evt->GetRawTrace() ){
					hismanager->Fill(tracehis,idx,tracevalue);
					++idx;
				}

				std::string tracederivativehis = (isfront) ? ("BSM_376"+std::to_string(position)+"_F") :  ("BSM_376"+std::to_string(position)+"_B");
				idx = 0;
				for( const auto& tracevalue : evt->GetTraceDerivative() ){
					hismanager->Fill(tracederivativehis,idx,tracevalue);
					++idx;
				}

			}

			auto psdvals = evt->GetTraceFixedPSD();

			auto integral = std::get<0>(psdvals)+std::get<1>(psdvals);
			auto tmax = evt->GetBaselineSubtractedPSDBoundedMaxValue();
			auto baseline = evt->GetTracePreTriggerBaseline();
			auto pk = evt->GetPSDBoundedTraceMaxInfo();
			auto psd = tmax/integral;
			
			std::string psdhis = (isfront) ?  ("BSM_371"+std::to_string(position)+"_F") :  ("BSM_371"+std::to_string(position)+"_B");
			hismanager->Fill(psdhis,integral,psd);
			//hismanager->Fill(psdhis,evt->GetRawEnergyWRandom(),psd);
			if( this->TraceSettings[detectorposition] != nullptr ){
				if( integral < this->TraceSettings[detectorposition]->integralthreshold ){
					continue;
				}else{
					if( cutmanager->IsWithin(this->TraceSettings[detectorposition]->cutid,integral,psd) ){
						continue;
					}
				}
			}

			std::string baselinehis = (isfront) ?  ("BSM_372"+std::to_string(position)+"_F") :  ("BSM_372"+std::to_string(position)+"_B");
			hismanager->Fill(baselinehis,baseline.second,baseline.first);
			
			std::string peakhis = (isfront) ?  ("BSM_373"+std::to_string(position)+"_F") :  ("BSM_373"+std::to_string(position)+"_B");
			hismanager->Fill(peakhis,pk.first,pk.second);
			
			std::string pklocbstddevhis = (isfront) ?  ("BSM_374"+std::to_string(position)+"_F") :  ("BSM_374"+std::to_string(position)+"_B");
			hismanager->Fill(pklocbstddevhis,pk.first,baseline.second);
			
			std::string pkmaxerghis = (isfront) ?  ("BSM_375"+std::to_string(position)+"_F") :  ("BSM_375"+std::to_string(position)+"_B");
			hismanager->Fill(pkmaxerghis,evt->GetRawEnergyWRandom(),pk.first);
			
			this->Traces[detectorposition] = evt->GetRawTraceData();
			this->CurrEvt.UnCorrectedBSM[detectorposition] = evt->GetEnergy();
			this->HitTimeStamps[detectorposition] = evt->GetTimeStamp();
			this->TimeStamps.push_back(evt->GetTimeStamp());
			this->RawBSM[detectorposition] = evt->GetRawEnergyWRandom();
			++this->BSMHits[detectorposition];
		}else{
			++this->BSMHits[detectorposition];
		}
	}
	
	if( not this->TimeStamps.empty() ){
		this->CurrEvt.FirstTime = *(std::min_element(this->TimeStamps.begin(),this->TimeStamps.end()));
		this->CurrEvt.LastTime = *(std::max_element(this->TimeStamps.begin(),this->TimeStamps.end()));

		this->currevttime = (this->CurrEvt.FirstTime - globalfirsttime)*1.0e-9;

		if( this->Pairs[0] != nullptr and this->Pairs[1] != nullptr ){
			auto psdvals = this->Pairs[0]->GetTraceFixedPSD();
			auto integral = std::get<0>(psdvals)+std::get<1>(psdvals);
			auto tmax = this->Pairs[0]->GetBaselineSubtractedPSDBoundedMaxValue();
			auto psd = tmax/integral;
			std::string psdhis = "BSM_3710_F_CORRELATED";
			hismanager->Fill(psdhis,integral,psd);
			
			psdvals = this->Pairs[1]->GetTraceFixedPSD();
			integral = std::get<0>(psdvals)+std::get<1>(psdvals);
			tmax = this->Pairs[1]->GetBaselineSubtractedPSDBoundedMaxValue();
			psd = tmax/integral;
			psdhis = "BSM_3710_B_CORRELATED";
			hismanager->Fill(psdhis,integral,psd);
		}

		if( (this->Pairs[0] != nullptr and this->Pairs[1] != nullptr) and (this->BSMHits[0] and not this->BSMHits[1]) ){
			auto psdvals = this->Pairs[0]->GetTraceFixedPSD();
			auto integral = std::get<0>(psdvals)+std::get<1>(psdvals);
			auto tmax = this->Pairs[0]->GetBaselineSubtractedPSDBoundedMaxValue();
			auto psd = tmax/integral;
			std::string psdhis = "BSM_3710_F_SINGLE_PASS";
			hismanager->Fill(psdhis,integral,psd);
			
			psdvals = this->Pairs[1]->GetTraceFixedPSD();
			integral = std::get<0>(psdvals)+std::get<1>(psdvals);
			tmax = this->Pairs[1]->GetBaselineSubtractedPSDBoundedMaxValue();
			psd = tmax/integral;
			psdhis = "BSM_3710_B_SINGLE_PASS";
			hismanager->Fill(psdhis,integral,psd);
		}

		for( int ii = 0; ii < this->NumPairs; ++ii ){
			if( this->BSMHits[2*ii] and this->BSMHits[2*ii + 1] ){
				this->CurrEvt.RealEvt = true;
				this->CurrEvt.TDiff[ii] = (this->HitTimeStamps[2*ii] - this->HitTimeStamps[2*ii + 1]);
				++this->CurrEvt.NumValidSegments;
				this->CurrEvt.UnCorrectedSumFrontBackEnergy[ii] = (this->CurrEvt.UnCorrectedBSM[2*ii] + this->CurrEvt.UnCorrectedBSM[2*ii + 1])/2.0;
				this->CurrEvt.Position[ii] = this->CalcPosition(this->RawBSM[2*ii],this->RawBSM[2*ii + 1]);
				if( (this->PosCorrectionMap[2*ii] != nullptr) and (this->PosCorrectionMap[2*ii + 1] != nullptr ) ){
					auto front = this->PosCorrectionMap[2*ii]->Correct(this->CurrEvt.UnCorrectedBSM[2*ii],this->CurrEvt.Position[ii]);
					auto back = this->PosCorrectionMap[2*ii + 1]->Correct(this->CurrEvt.UnCorrectedBSM[2*ii + 1],this->CurrEvt.Position[ii]);
					this->CurrEvt.CorrectedBSM[2*ii] = front;
					this->CurrEvt.CorrectedBSM[2*ii + 1] = back;
					this->CurrEvt.SumFrontBackEnergy[ii] = (front + back)/2.0;
				}else{
					this->CurrEvt.SumFrontBackEnergy[ii] = (this->CurrEvt.UnCorrectedBSM[2*ii] + this->CurrEvt.UnCorrectedBSM[2*ii + 1])/2.0;
					this->CurrEvt.CorrectedBSM[2*ii] = this->CurrEvt.UnCorrectedBSM[2*ii];
					this->CurrEvt.CorrectedBSM[2*ii + 1] = this->CurrEvt.UnCorrectedBSM[2*ii + 1];
				}

				std::string id = std::to_string(ii);
				std::string name = "BSM_362"+id+"_F";

				hismanager->Fill(name,this->CurrEvt.Position[ii],this->RawBSM[2*ii]);

				name = "BSM_362"+id+"_B";
				hismanager->Fill(name,this->CurrEvt.Position[ii],this->RawBSM[2*ii + 1]);

				name = "BSM_362"+id;
				hismanager->Fill(name,this->CurrEvt.Position[ii],(this->RawBSM[2*ii] + this->RawBSM[2*ii + 1])/2.0);

				name = "BSM_363"+id;
				hismanager->Fill(name,this->CurrEvt.TDiff[ii],this->CurrEvt.SumFrontBackEnergy[ii]);

				name = "BSM_363"+id+"_F";
				hismanager->Fill(name,this->CurrEvt.TDiff[ii],this->CurrEvt.UnCorrectedBSM[2*ii]);

				name = "BSM_363"+id+"_B";
				hismanager->Fill(name,this->CurrEvt.TDiff[ii],this->CurrEvt.UnCorrectedBSM[2*ii + 1]);
			//}else if( this->BSMHits[2*ii] and this->TotalMult[2*ii + 1] ){
			}
		}

		for( int ii = 0; ii < this->NumPairs; ++ii ){
			this->CurrEvt.TotalEnergy += this->CurrEvt.SumFrontBackEnergy[ii];
			this->CurrEvt.UnCorrectedTotalEnergy += this->CurrEvt.UnCorrectedSumFrontBackEnergy[ii];
		}

		hismanager->Fill("BSM_4000",this->CurrEvt.TotalEnergy,this->currevttime*1000.0);
		hismanager->Fill("BSM_4001",this->CurrEvt.TotalEnergy,this->currevttime);
		hismanager->Fill("BSM_4002",this->CurrEvt.TotalEnergy,this->currevttime/60.0);
		hismanager->Fill("BSM_4003",this->CurrEvt.TotalEnergy,this->currevttime/(60.0*60.0));
		hismanager->Fill("BSM_4004",this->CurrEvt.TotalEnergy,this->currevttime/(60.0*60.0*24.0));

		for( int ii = 0; ii < this->NumPairs; ++ii ){
			std::string id = std::to_string(ii);
			std::string name = "BSM_367"+id;

			hismanager->Fill(name,this->CurrEvt.CorrectedBSM[2*ii+1],this->CurrEvt.CorrectedBSM[2*ii]);

			name = "BSM_367"+id+"8";
			hismanager->Fill(name,this->CurrEvt.CorrectedBSM[2*ii+1],this->CurrEvt.CorrectedBSM[2*ii]);
		}
	}


	for( int ii = 0; ii < this->NumPMTs; ++ii ){
		hismanager->Fill("BSM_3500",this->TotalMult[ii],ii);
		hismanager->Fill("BSM_3501",this->BSMHits[ii],ii);
	}
	if( this->SummaryData.size() > 2 ){
		int sum = 0;
		for( int ii = 0; ii < this->NumPMTs; ++ii ){
			sum += this->TotalMult[ii];
			this->console->error("channel {} : {}",ii,this->TotalMult[ii]);
		}
		for( const auto& evt : this->SummaryData ){
			this->console->error("id : {}, ts : {} erg : {} pileup : {} saturate : {}",evt->GetSpillID(),evt->GetTimeStamp(),evt->GetRawEnergy(),evt->GetPileup(),evt->GetSaturation());
		}
		this->console->info("Total: {}/{}",sum,this->SummaryData.size());
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
	this->PlotAllTraces = config.attribute("PlotAllTraces").as_bool(true);
	this->NumPairs = config.attribute("NumSegments").as_int(1);
	this->NumPMTs = 2*this->NumPairs;

	this->BSMHits = std::vector<int>(this->NumPMTs,0);
	this->RawBSM = std::vector<double>(this->NumPMTs,0.0);
	this->TotalMult = std::vector<int>(this->NumPMTs,0);
	this->HitTimeStamps = std::vector<double>(this->NumPMTs,0.0);
	this->Traces = std::vector<std::vector<uint16_t>>(this->NumPMTs,std::vector<uint16_t>());
	this->Pairs = std::vector<PhysicsData*>(this->NumPMTs,nullptr);
	for( size_t ii = 0; ii < this->NumPMTs; ++ii ){
		this->TraceSettings.push_back(nullptr);
		this->PosCorrectionMap.push_back(nullptr);
	}

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

				this->TraceSettings[id]->integralthreshold = trace.attribute("integralthreshold").as_float(0.0);
				this->TraceSettings[id]->cutid = trace.attribute("cutid").as_string("");
				if( this->TraceSettings[id]->cutid.empty() ){
					std::string mess = "Invalid PulseAnalysis Setting in BSMProcessor id="+std::to_string(id)+" missing cutid attribute";
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
	hismanager->RegisterPlot<TH2F>("BSM_3500","#betaSM Channel Hit Mult. (event by event); Channel Hit Multiplicity",20,-1,9,this->NumPMTs,0,this->NumPMTs);
	hismanager->RegisterPlot<TH2F>("BSM_3501","#betaSM Channel Hit Mult. (event by event, survive cuts); Channel Hit Multiplicity",20,-1,9,this->NumPMTs,0,this->NumPMTs);
	hismanager->RegisterPlot<TH1F>("BSM_3600","#betaSM Total; Energy (keV)",this->h1dsettings.at(3600));
	hismanager->RegisterPlot<TH1F>("BSM_3601","#betaSM Total No MTAS; Energy (keV)",this->h1dsettings.at(3601));
	hismanager->RegisterPlot<TH1F>("BSM_3602","#betaSM Total + MTAS Total; Energy (keV)",this->h1dsettings.at(3602));
	hismanager->RegisterPlot<TH1F>("BSM_3603","#betaSM Total + MTAS Total veto M,O; Energy (keV)",this->h1dsettings.at(3603));
	hismanager->RegisterPlot<TH1F>("BSM_3604","#betaSM Total + MTAS C Sum veto M,O; Energy (keV)",this->h1dsettings.at(3604));
	hismanager->RegisterPlot<TH1F>("BSM_3605","#betaSM Total + MTAS C Sum veto I,M,O; Energy (keV)",this->h1dsettings.at(3605));
	hismanager->RegisterPlot<TH1F>("BSM_3610","#betaSM Total; Energy (keV)",this->h1dsettings.at(3610));
	hismanager->RegisterPlot<TH1F>("BSM_3611","#betaSM Total [MTAS Pileup]; Energy (keV)",this->h1dsettings.at(3611));
	hismanager->RegisterPlot<TH1F>("BSM_3612","#betaSM Total [MTAS Saturate]; Energy (keV)",this->h1dsettings.at(3612));

	for( size_t ii = 0; ii < this->NumPairs; ++ii ){
		std::string name = "BSM_362"+std::to_string(ii)+"_F";
		std::string title = "#betaSM"+std::to_string(ii+1)+"_F Energy vs #betaSM Position; Position (arb.); Energy (channel)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3620));
		
		name = "BSM_362"+std::to_string(ii)+"_B";
		title = "#betaSM"+std::to_string(ii+1)+"_B Energy vs #betaSM Position; Position (arb.); Energy (channel)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3620));
		
		name = "BSM_362"+std::to_string(ii);
		title = "#betaSM"+std::to_string(ii+1)+" Energy vs #betaSM Position; Position (arb.); Energy (channel)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3620));
		
		name = "BSM_362"+std::to_string(ii)+"_F_NOMUON";
		title = "#betaSM"+std::to_string(ii+1)+"_F Energy vs #betaSM Position; Position (arb.); Energy (channel)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3620));
		
		name = "BSM_362"+std::to_string(ii)+"_B_NOMUON";
		title = "#betaSM"+std::to_string(ii+1)+"_B Energy vs #betaSM Position; Position (arb.); Energy (channel)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3620));
		
		name = "BSM_362"+std::to_string(ii)+"_NOMUON";
		title = "#betaSM"+std::to_string(ii+1)+" Energy vs #betaSM Position; Position (arb.); Energy (channel)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3620));
		
		name = "BSM_363"+std::to_string(ii);
		title = "#betaSM"+std::to_string(ii+1)+" Energy vs #betaSM TDiff(F-B); TDiff(F-B) (pixie ticks); Energy (keV)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3630));
		
		name = "BSM_363"+std::to_string(ii)+"_F";
		title = "#betaSM"+std::to_string(ii+1)+"_F Energy vs #betaSM TDiff(F-B); TDiff(F-B) (pixie ticks); Energy (keV)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3630));
		
		name = "BSM_363"+std::to_string(ii)+"_B";
		title = "#betaSM"+std::to_string(ii+1)+"_B Energy vs #betaSM TDiff(F-B); TDiff(F-B) (pixie ticks); Energy (keV)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3630));
		
		name = "BSM_367"+std::to_string(ii);
		title = "#betaSM"+std::to_string(ii+1)+"_F Energy vs #betaSM"+std::to_string(ii+1)+"_B; Energy (keV); Energy (keV)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3670));

		name = "BSM_367"+std::to_string(ii)+"8";
		title = "#betaSM"+std::to_string(ii+1)+"_F Energy vs #betaSM"+std::to_string(ii+1)+"_B; Energy (8 keV/bin); Energy (8 keV/bin)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(36708));

		name = "BSM_367"+std::to_string(ii)+"_NOMUON";
		title = "#betaSM"+std::to_string(ii+1)+"_F Energy vs #betaSM"+std::to_string(ii+1)+"_B; Energy (keV); Energy (keV)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3670));

		name = "BSM_367"+std::to_string(ii)+"8_NOMUON";
		title = "#betaSM"+std::to_string(ii+1)+"_F Energy vs #betaSM"+std::to_string(ii+1)+"_B; Energy (8 keV/bin); Energy (8 keV/bin)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(36708));


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
		
		name = "BSM_371"+std::to_string(ii)+"_F_CORRELATED";
		title = "#betaSM"+std::to_string(ii+1)+"_F (Peak/Integral) vs Integral; Integral (arb.); Ratio (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3710));
		
		name = "BSM_371"+std::to_string(ii)+"_B_CORRELATED";
		title = "#betaSM"+std::to_string(ii+1)+"_B (Peak/Integral) vs Integral; Integral (arb.); Ratio (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3710));
		
		name = "BSM_371"+std::to_string(ii)+"_F_SINGLE_PASS";
		title = "#betaSM"+std::to_string(ii+1)+"_F (Peak/Integral) vs Integral; Integral (arb.); Ratio (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3710));
		
		name = "BSM_371"+std::to_string(ii)+"_B_SINGLE_PASS";
		title = "#betaSM"+std::to_string(ii+1)+"_B (Peak/Integral) vs Integral; Integral (arb.); Ratio (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3710));
		
		name = "BSM_372"+std::to_string(ii)+"_F";
		title = "#betaSM"+std::to_string(ii+1)+"_F Baseline Avg vs Baseline StdDev; StdDev. (arb.); Avg. (arb.)";
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

		if( this->PlotAllTraces ){
			name = "BSM_376"+std::to_string(ii)+"_F";
			title = "#betaSM"+std::to_string(ii+1)+"_F Trace Derivative; Clock Ticks (arb.); adc (arb.)";
			hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3760));

			name = "BSM_376"+std::to_string(ii)+"_B";
			title = "#betaSM"+std::to_string(ii+1)+"_B Trace Derivative; Clock Ticks (arb.); adc (arb.)";
			hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3760));
		}
	
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
	
		name = "BSM_386"+std::to_string(ii)+"_F";
		title = "#betaSM"+std::to_string(ii+1)+"_F Pileup Trace Derivative; Clock Ticks (arb.); adc (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3860));

		name = "BSM_386"+std::to_string(ii)+"_B";
		title = "#betaSM"+std::to_string(ii+1)+"_B Pileup Trace Derivative; Clock Ticks (arb.); adc (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3860));

		name = "BSM_390"+std::to_string(ii)+"_F";
		title = "#betaSM"+std::to_string(ii+1)+"_F G.S. Pileup; Clock Ticks (arb.); adc (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3900));

		name = "BSM_390"+std::to_string(ii)+"_B";
		title = "#betaSM"+std::to_string(ii+1)+"_B G.S. Pileup; Clock Ticks (arb.); adc (arb.)";
		hismanager->RegisterPlot<TH2F>(name,title,this->h2dsettings.at(3900));

	}
	
	hismanager->RegisterPlot<TH2F>("BSM_3650","#betaSM Total vs MTAS Total; MTAS Total Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3650));
	hismanager->RegisterPlot<TH2F>("BSM_36508","#betaSM Total vs MTAS Total; MTAS Total Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36508));

	hismanager->RegisterPlot<TH2F>("BSM_3651","#betaSM Total vs MTAS I,M,O; MTAS I,M,O Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3651));
	hismanager->RegisterPlot<TH2F>("BSM_36518","#betaSM Total vs MTAS I,M,O; MTAS I,M,O Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36518));

	hismanager->RegisterPlot<TH2F>("BSM_3652","#betaSM Total vs MTAS Center Sum; MTAS Center Sum Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3651));
	hismanager->RegisterPlot<TH2F>("BSM_36528","#betaSM Total vs MTAS Center Sum; MTAS Center Sum Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36518));

	hismanager->RegisterPlot<TH2F>("BSM_3653","#betaSM Total vs MTAS C; MTAS C Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3653));
	hismanager->RegisterPlot<TH2F>("BSM_36538","#betaSM Total vs MTAS C; MTAS C Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36538));

	hismanager->RegisterPlot<TH2F>("BSM_3654","#betaSM Total vs MTAS Total Veto M,O; MTAS Total Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3654));
	hismanager->RegisterPlot<TH2F>("BSM_36548","#betaSM Total vs MTAS Total Veot M,O; MTAS Total Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36548));

	hismanager->RegisterPlot<TH2F>("BSM_3655","#betaSM Total vs MTAS Center Sum Veto M,O; MTAS Center Sum Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3655));
	hismanager->RegisterPlot<TH2F>("BSM_36558","#betaSM Total vs MTAS Center Sum Veto M,O; MTAS Center Sum Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36558));

	hismanager->RegisterPlot<TH2F>("BSM_3656","#betaSM Total vs MTAS C Veto M,O; MTAS C Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3656));
	hismanager->RegisterPlot<TH2F>("BSM_36568","#betaSM Total vs MTAS C Veto M,O; MTAS C Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36568));
	
	hismanager->RegisterPlot<TH2F>("BSM_3657","#betaSM Total vs MTAS Center Sum Veto I,M,O; MTAS Center Sum Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3657));
	hismanager->RegisterPlot<TH2F>("BSM_36578","#betaSM Total vs MTAS Center Sum Veto I,M,O; MTAS Center Sum Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36578));
	
	hismanager->RegisterPlot<TH2F>("BSM_3658","#betaSM Total vs MTAS C Veto I,M,O; MTAS C Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3658));
	hismanager->RegisterPlot<TH2F>("BSM_36588","#betaSM Total vs MTAS C Veto I,M,O; MTAS C Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36588));
	
	hismanager->RegisterPlot<TH2F>("BSM_3660","#betaSM Total + MTAS Total vs MTAS Total; MTAS Total Energy (keV); #betaSM Energy + MTAS Total Energy (keV)",this->h2dsettings.at(3660));
	hismanager->RegisterPlot<TH2F>("BSM_36608","#betaSM Total + MTAS Total vs MTAS Total; MTAS Total Energy (8 keV/bin); #betaSM Energy + MTAS Total Energy (8 keV/bin)",this->h2dsettings.at(36608));

	hismanager->RegisterPlot<TH2F>("BSM_3661","#betaSM Total vs #betaSM Total + MTAS Total; #betaSM Energy + MTAS Total Energy (keV); #betaSM Energy (keV)",this->h2dsettings.at(3661));
	hismanager->RegisterPlot<TH2F>("BSM_36618","#betaSM Total vs #betaSM Total + MTAS Total; #betaSM Energy + MTAS Total Energy (8 keV/bin); #betaSM Energy (8 keV/bin)",this->h2dsettings.at(36618));

	hismanager->RegisterPlot<TH2F>("BSM_4000","Run Time vs #betaSM Total; #betaSM Energy (keV); Run Time (ms)",this->h2dsettings.at(4000));
	hismanager->RegisterPlot<TH2F>("BSM_4001","Run Time vs #betaSM Total; #betaSM Energy (keV); Run Time (s)",this->h2dsettings.at(4001));
	hismanager->RegisterPlot<TH2F>("BSM_4002","Run Time vs #betaSM Total; #betaSM Energy (keV); Run Time (min)",this->h2dsettings.at(4002));
	hismanager->RegisterPlot<TH2F>("BSM_4003","Run Time vs #betaSM Total; #betaSM Energy (keV); Run Time (hr)",this->h2dsettings.at(4003));
	hismanager->RegisterPlot<TH2F>("BSM_4004","Run Time vs #betaSM Total; #betaSM Energy (keV); Run Time (day)",this->h2dsettings.at(4004));

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
	this->BSMHits = std::vector<int>(this->NumPMTs,0);
	this->RawBSM = std::vector<double>(this->NumPMTs,0.0);
	this->HitTimeStamps = std::vector<double>(this->NumPMTs,0.0);
	this->TotalMult = std::vector<int>(this->NumPMTs,0);
	this->Pairs = std::vector<PhysicsData*>(this->NumPMTs,nullptr);
}

void BSMProcessor::FillGSPileupTracePlots(PLOTS::PlotRegistry* hismanager) const{
	for( int ii = 0; ii < this->NumPairs; ++ii ){
		std::string fronthis = "BSM_390"+std::to_string(ii)+"_F";
		size_t idx = 0;
		for( const auto& val : this->Traces[2*ii] ){
			hismanager->Fill(fronthis,idx,val);
			++idx;
		}
		
		std::string backhis = "BSM_390"+std::to_string(ii)+"_B";
		idx = 0;
		for( const auto& val : this->Traces[2*ii + 1] ){
			hismanager->Fill(backhis,idx,val);
			++idx;
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

void BSMProcessor::FillPositionPlots(PLOTS::PlotRegistry* hismanager) const{
	if( not this->TimeStamps.empty() ){
		for( int ii = 0; ii < this->NumPairs; ++ii ){
			if( this->Pairs[2*ii] != nullptr and this->Pairs[2*ii + 1] != nullptr and this->BSMHits[2*ii] and this->BSMHits[2*ii + 1] ){
				std::string id = std::to_string(ii);
				std::string name = "BSM_362"+id+"_F_NOMUON";

				hismanager->Fill(name,this->CurrEvt.Position[ii],this->RawBSM[2*ii]);

				name = "BSM_362"+id+"_B_NOMUON";
				hismanager->Fill(name,this->CurrEvt.Position[ii],this->RawBSM[2*ii + 1]);

				name = "BSM_362"+id+"_NOMUON";
				hismanager->Fill(name,this->CurrEvt.Position[ii],(this->RawBSM[2*ii]+this->RawBSM[2*ii + 1])/2.0);

				name = "BSM_367"+id+"_NOMUON";
				hismanager->Fill(name,this->CurrEvt.CorrectedBSM[2*ii+1],this->CurrEvt.CorrectedBSM[2*ii]);

				name = "BSM_367"+id+"8_NOMUON";
				hismanager->Fill(name,this->CurrEvt.CorrectedBSM[2*ii+1],this->CurrEvt.CorrectedBSM[2*ii]);
			}
		}
	}
}
