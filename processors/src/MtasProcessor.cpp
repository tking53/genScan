#include "MtasProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>
#include <vector>

MtasProcessor::MtasProcessor(const std::string& log) : Processor(log,"MtasProcessor",{"mtas"}){
	this->NewEvt = {
		.TotalEnergy = std::vector<double>(5,0.0),
		.SumFrontBackEnergy = std::vector<double>(24,0.0),
		.FirstTime = -1.0,
		.LastTime = -1.0,
		.Saturate = false,
		.Pileup = false,
		.BetaTriggered = false,
		.RealEvt = false
	};
	this->PrevEvt = this->NewEvt;
	this->CurrEvt = this->NewEvt;

	this->fronttag = "front";
	this->backtag = "back";
	this->currsubtype = SUBTYPE::UNKNOWN;
	this->foundfirstevt = false;
	this->globalfirsttime = 0.0;
}

[[maybe_unused]] bool MtasProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	this->Reset();

	summary.GetDetectorSummary(this->AllDefaultRegex["mtas"],this->SummaryData);
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

		if( currsubtype == SUBTYPE::CENTER ){
			if( !this->CenterHits[detectorposition] ){
				this->Center[detectorposition] += evt->GetEnergy();
				++this->CenterHits[detectorposition];
			}else{
				++this->CenterHits[detectorposition];
			}
		}else if( currsubtype == SUBTYPE::INNER ){
			if( !this->InnerHits[detectorposition] ){
				this->Inner[detectorposition] += evt->GetEnergy();
				++this->InnerHits[detectorposition];
			}else{
				++this->InnerHits[detectorposition];
			}
		}else if( currsubtype == SUBTYPE::MIDDLE ){
			if( !this->MiddleHits[detectorposition] ){
				this->Middle[detectorposition] += evt->GetEnergy();
				++this->MiddleHits[detectorposition];
			}else{
				++this->MiddleHits[detectorposition];
			}
		}else if( currsubtype == SUBTYPE::OUTER ){
			if( !this->OuterHits[detectorposition] ){
				this->Outer[detectorposition] += evt->GetEnergy();
				++this->OuterHits[detectorposition];
			}else{
				++this->OuterHits[detectorposition];
			}
		}else{
			throw std::runtime_error("we shouldn't have gotten here");
		}

	}

	this->CurrEvt.FirstTime = firsttime;
	this->CurrEvt.LastTime = lasttime;

	for( int ii = 0; ii < 6; ++ii ){
		if( this->CenterHits[2*ii] and this->CenterHits[2*ii + 1] ){
			this->CurrEvt.SumFrontBackEnergy[ii] += (this->Center[2*ii] + this->Center[2*ii + 1])/2.0;
		}
		if( this->InnerHits[2*ii] and this->InnerHits[2*ii + 1] ){
			this->CurrEvt.SumFrontBackEnergy[ii+6] += (this->Inner[2*ii] + this->Inner[2*ii + 1])/2.0;
		}
		if( this->MiddleHits[2*ii] and this->MiddleHits[2*ii + 1] ){
			this->CurrEvt.SumFrontBackEnergy[ii+12] += (this->Middle[2*ii] + this->Middle[2*ii + 1])/2.0;
		}
		if( this->OuterHits[2*ii] and this->OuterHits[2*ii + 1] ){
			this->CurrEvt.SumFrontBackEnergy[ii+18] += (this->Outer[2*ii] + this->Outer[2*ii + 1])/2.0;
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

	if( (not this->CurrEvt.Saturate) or (not this->CurrEvt.Pileup) ){
		hismanager->Fill("MTAS_3200",this->CurrEvt.TotalEnergy[0]);
		
		hismanager->Fill("MTAS_3210",this->CurrEvt.TotalEnergy[1]);
		hismanager->Fill("MTAS_3220",this->CurrEvt.TotalEnergy[2]);
		hismanager->Fill("MTAS_3230",this->CurrEvt.TotalEnergy[3]);
		hismanager->Fill("MTAS_3240",this->CurrEvt.TotalEnergy[4]);

		hismanager->Fill("MTAS_3251",this->CurrEvt.TotalEnergy[0],this->CurrEvt.TotalEnergy[1]);
		hismanager->Fill("MTAS_3251_8X",this->CurrEvt.TotalEnergy[0],this->CurrEvt.TotalEnergy[1]);

		for( int ii = 0; ii < 24; ++ii ){
			hismanager->Fill("MTAS_3201",this->CurrEvt.SumFrontBackEnergy[ii],ii);
		}

		for( int ii = 0; ii < 6; ++ii ){
			hismanager->Fill("MTAS_3215",this->CurrEvt.SumFrontBackEnergy[ii]);
			hismanager->Fill("MTAS_3225",this->CurrEvt.SumFrontBackEnergy[ii+6]);
			hismanager->Fill("MTAS_3235",this->CurrEvt.SumFrontBackEnergy[ii+12]);
			hismanager->Fill("MTAS_3245",this->CurrEvt.SumFrontBackEnergy[ii+18]);
		
			hismanager->Fill("MTAS_3250",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+6]);
			hismanager->Fill("MTAS_3250",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+12]);
			hismanager->Fill("MTAS_3250",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+18]);
			
			hismanager->Fill("MTAS_3252",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);
			
			hismanager->Fill("MTAS_3250_8X",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+6]);
			hismanager->Fill("MTAS_3250_8X",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+12]);
			hismanager->Fill("MTAS_3250_8X",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+18]);
			
			hismanager->Fill("MTAS_3252_8X",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);
		}
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool MtasProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::Process();
	if( (not this->CurrEvt.Saturate) or (not this->CurrEvt.Pileup) ){
		if( this->CurrEvt.BetaTriggered ){

			hismanager->Fill("MTAS_3300",this->CurrEvt.TotalEnergy[0]);

			hismanager->Fill("MTAS_3310",this->CurrEvt.TotalEnergy[1]);
			hismanager->Fill("MTAS_3320",this->CurrEvt.TotalEnergy[2]);
			hismanager->Fill("MTAS_3330",this->CurrEvt.TotalEnergy[3]);
			hismanager->Fill("MTAS_3340",this->CurrEvt.TotalEnergy[4]);

			hismanager->Fill("MTAS_3351",this->CurrEvt.TotalEnergy[0],this->CurrEvt.TotalEnergy[1]);
			hismanager->Fill("MTAS_3351_8X",this->CurrEvt.TotalEnergy[0],this->CurrEvt.TotalEnergy[1]);
		
			for( int ii = 0; ii < 24; ++ii ){
				hismanager->Fill("MTAS_3301",this->CurrEvt.SumFrontBackEnergy[ii],ii);
			}

			for( int ii = 0; ii < 6; ++ii ){
				hismanager->Fill("MTAS_3315",this->CurrEvt.SumFrontBackEnergy[ii]);
				hismanager->Fill("MTAS_3325",this->CurrEvt.SumFrontBackEnergy[ii+6]);
				hismanager->Fill("MTAS_3335",this->CurrEvt.SumFrontBackEnergy[ii+12]);
				hismanager->Fill("MTAS_3345",this->CurrEvt.SumFrontBackEnergy[ii+18]);

				hismanager->Fill("MTAS_3350",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+6]);
				hismanager->Fill("MTAS_3350",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+12]);
				hismanager->Fill("MTAS_3350",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+18]);

				hismanager->Fill("MTAS_3352",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);

				hismanager->Fill("MTAS_3350_8X",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+6]);
				hismanager->Fill("MTAS_3350_8X",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+12]);
				hismanager->Fill("MTAS_3350_8X",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+18]);

				hismanager->Fill("MTAS_3352_8X",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);
			}

		}else{
			hismanager->Fill("MTAS_3100",this->CurrEvt.TotalEnergy[0]);

			hismanager->Fill("MTAS_3110",this->CurrEvt.TotalEnergy[1]);
			hismanager->Fill("MTAS_3120",this->CurrEvt.TotalEnergy[2]);
			hismanager->Fill("MTAS_3130",this->CurrEvt.TotalEnergy[3]);
			hismanager->Fill("MTAS_3140",this->CurrEvt.TotalEnergy[4]);

			hismanager->Fill("MTAS_3151",this->CurrEvt.TotalEnergy[0],this->CurrEvt.TotalEnergy[1]);
			hismanager->Fill("MTAS_3151_8X",this->CurrEvt.TotalEnergy[0],this->CurrEvt.TotalEnergy[1]);
		
			for( int ii = 0; ii < 24; ++ii ){
				hismanager->Fill("MTAS_3101",this->CurrEvt.SumFrontBackEnergy[ii],ii);
			}

			for( int ii = 0; ii < 6; ++ii ){
				hismanager->Fill("MTAS_3115",this->CurrEvt.SumFrontBackEnergy[ii]);
				hismanager->Fill("MTAS_3125",this->CurrEvt.SumFrontBackEnergy[ii+6]);
				hismanager->Fill("MTAS_3135",this->CurrEvt.SumFrontBackEnergy[ii+12]);
				hismanager->Fill("MTAS_3145",this->CurrEvt.SumFrontBackEnergy[ii+18]);

				hismanager->Fill("MTAS_3150",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+6]);
				hismanager->Fill("MTAS_3150",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+12]);
				hismanager->Fill("MTAS_3150",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+18]);

				hismanager->Fill("MTAS_3152",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);

				hismanager->Fill("MTAS_3150_8X",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+6]);
				hismanager->Fill("MTAS_3150_8X",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+12]);
				hismanager->Fill("MTAS_3150_8X",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii+18]);

				hismanager->Fill("MTAS_3152_8X",this->CurrEvt.TotalEnergy[0],this->CurrEvt.SumFrontBackEnergy[ii]);
			}

		}
	}
	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool MtasProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

void MtasProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
}

void MtasProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
}

void MtasProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
}
		
void MtasProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void MtasProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	//MTAS diagnostic plots, always want these no matter what
	hismanager->RegisterPlot<TH1F>("MTAS_3200","Mtas Total; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH2F>("MTAS_3201","Sum F+B; Energy (keV); F+B Pair (arb.)",16384,0,16384,24,0,24);

	hismanager->RegisterPlot<TH1F>("MTAS_3210","Mtas Center Sum; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH1F>("MTAS_3215","Mtas Center Stack; Energy (keV)",16384,0,16384);

	hismanager->RegisterPlot<TH1F>("MTAS_3220","Mtas Inner Sum; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH1F>("MTAS_3225","Mtas Inner Stack; Energy (keV)",16384,0,16384);

	hismanager->RegisterPlot<TH1F>("MTAS_3230","Mtas Middle Sum; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH1F>("MTAS_3235","Mtas Middle Stack; Energy (keV)",16384,0,16384);

	hismanager->RegisterPlot<TH1F>("MTAS_3240","Mtas Outer Sum; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH1F>("MTAS_3245","Mtas Outer Stack; Energy (keV)",16384,0,16384);
	
	hismanager->RegisterPlot<TH2F>("MTAS_3250","Mtas Total vs I,M,O; Energy (keV); Energy (keV)",4096,0,4096,4096,0,4096);
	hismanager->RegisterPlot<TH2F>("MTAS_3251","Mtas Total vs C; Energy (keV); Energy (keV)",4096,0,4096,4096,0,4096);
	hismanager->RegisterPlot<TH2F>("MTAS_3252","Mtas Total vs C Segment; Energy (keV); Energy (keV)",4096,0,4096,4096,0,4096);

	hismanager->RegisterPlot<TH2F>("MTAS_3250_8X","Mtas Total vs I,M,O; Energy (8 keV/bin); Energy (8 keV/bin)",2048,0,16384,2048,0,16384);
	hismanager->RegisterPlot<TH2F>("MTAS_3251_8X","Mtas Total vs C; Energy (8 keV/bin); Energy (8 keV/bin)",2048,0,16384,2048,0,16384);
	hismanager->RegisterPlot<TH2F>("MTAS_3252_8X","Mtas Total vs C Segment; Energy (8 keV/bin); Energy (8 keV/bin)",2048,0,16384,2048,0,16384);

	//declare the beta gated and not-beta histograms, but we don't fill them until process after parent has told which we are
	
	//beta event
	hismanager->RegisterPlot<TH1F>("MTAS_3300","Mtas Total #beta-gated; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH2F>("MTAS_3301","Sum F+B; Energy (keV); F+B Pair (arb.)",16384,0,16384,24,0,24);

	hismanager->RegisterPlot<TH1F>("MTAS_3310","Mtas Center Sum #beta-gated; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH1F>("MTAS_3315","Mtas Center Stack #beta-gated; Energy (keV)",16384,0,16384);

	hismanager->RegisterPlot<TH1F>("MTAS_3320","Mtas Inner Sum #beta-gated; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH1F>("MTAS_3325","Mtas Inner Stack #beta-gated; Energy (keV)",16384,0,16384);

	hismanager->RegisterPlot<TH1F>("MTAS_3330","Mtas Middle Sum #beta-gated; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH1F>("MTAS_3335","Mtas Middle Stack #beta-gated; Energy (keV)",16384,0,16384);

	hismanager->RegisterPlot<TH1F>("MTAS_3340","Mtas Outer Sum #beta-gated; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH1F>("MTAS_3345","Mtas Outer Stack #beta-gated; Energy (keV)",16384,0,16384);
	
	hismanager->RegisterPlot<TH2F>("MTAS_3350","Mtas Total vs I,M,O #beta-gated; Energy (keV); Energy (keV)",4096,0,4096,4096,0,4096);
	hismanager->RegisterPlot<TH2F>("MTAS_3351","Mtas Total vs C #beta-gated; Energy (keV); Energy (keV)",4096,0,4096,4096,0,4096);
	hismanager->RegisterPlot<TH2F>("MTAS_3352","Mtas Total vs C Segment #beta-gated; Energy (keV); Energy (keV)",4096,0,4096,4096,0,4096);

	hismanager->RegisterPlot<TH2F>("MTAS_3350_8X","Mtas Total vs I,M,O #beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",2048,0,16384,2048,0,16384);
	hismanager->RegisterPlot<TH2F>("MTAS_3351_8X","Mtas Total vs C #beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",2048,0,16384,2048,0,16384);
	hismanager->RegisterPlot<TH2F>("MTAS_3352_8X","Mtas Total vs C Segment #beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",2048,0,16384,2048,0,16384);

	//not beta event
	hismanager->RegisterPlot<TH1F>("MTAS_3100","Mtas Total anti-#beta-gated; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH2F>("MTAS_3101","Sum F+B; Energy (keV); F+B Pair (arb.)",16384,0,16384,24,0,24);

	hismanager->RegisterPlot<TH1F>("MTAS_3110","Mtas Center Sum anti-#beta-gated; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH1F>("MTAS_3115","Mtas Center Stack anti-#beta-gated; Energy (keV)",16384,0,16384);

	hismanager->RegisterPlot<TH1F>("MTAS_3120","Mtas Inner Sum anti-#beta-gated; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH1F>("MTAS_3125","Mtas Inner Stack anti-#beta-gated; Energy (keV)",16384,0,16384);

	hismanager->RegisterPlot<TH1F>("MTAS_3130","Mtas Middle Sum anti-#beta-gated; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH1F>("MTAS_3135","Mtas Middle Stack anti-#beta-gated; Energy (keV)",16384,0,16384);

	hismanager->RegisterPlot<TH1F>("MTAS_3140","Mtas Outer Sum anti-#beta-gated; Energy (keV)",16384,0,16384);
	hismanager->RegisterPlot<TH1F>("MTAS_3145","Mtas Outer Stack anti-#beta-gated; Energy (keV)",16384,0,16384);
	
	hismanager->RegisterPlot<TH2F>("MTAS_3150","Mtas Total vs I,M,O anti-#beta-gated; Energy (keV); Energy (keV)",4096,0,4096,4096,0,4096);
	hismanager->RegisterPlot<TH2F>("MTAS_3151","Mtas Total vs C anti-#beta-gated; Energy (keV); Energy (keV)",4096,0,4096,4096,0,4096);
	hismanager->RegisterPlot<TH2F>("MTAS_3152","Mtas Total vs C Segment anti-#beta-gated; Energy (keV); Energy (keV)",4096,0,4096,4096,0,4096);

	hismanager->RegisterPlot<TH2F>("MTAS_3150_8X","Mtas Total vs I,M,O anti-#beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",2048,0,16384,2048,0,16384);
	hismanager->RegisterPlot<TH2F>("MTAS_3151_8X","Mtas Total vs C anti-#beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",2048,0,16384,2048,0,16384);
	hismanager->RegisterPlot<TH2F>("MTAS_3152_8X","Mtas Total vs C Segment anti-#beta-gated; Energy (8 keV/bin); Energy (8 keV/bin)",2048,0,16384,2048,0,16384);

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
	this->Center = std::vector<double>(12,0.0);
	this->Inner = std::vector<double>(12,0.0);
	this->Middle = std::vector<double>(12,0.0);
	this->Outer = std::vector<double>(12,0.0);
	this->CenterHits = std::vector<int>(12,0);
	this->InnerHits = std::vector<int>(12,0);
	this->MiddleHits = std::vector<int>(12,0);
	this->OuterHits = std::vector<int>(12,0);
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
