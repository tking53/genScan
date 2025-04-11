#include "MtasImplantProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>

MtasImplantProcessor::MtasImplantProcessor(const std::string& log) : Processor(log,"MtasImplantProcessor",{"mtasimplant"}){

	this->h2dsettings = {
		{7000,{16384,0,16384,64,0,64}},
		{7003,{16384,0,16384,64,0,64}},

		{7012,{10,0,10,10,0,10}},
		{7013,{10,0,10,10,0,10}},
		{7014,{1000,0,10,1000,0,10}},
		{7015,{1000,0,10,1000,0,10}},
		
		{7030,{10,0,10,4,0,4}},
		{7040,{10,0,10,64,0,64}},
		{7043,{10,0,10,64,0,64}}
	};

	this->lowgaintag = "lowgain";
	this->highgaintag = "highgain";
	
	this->HighGain = ProcessorStruct::DEFAULT_MTAS_IMPLANT_STRUCT;
	this->LowGain = ProcessorStruct::DEFAULT_MTAS_IMPLANT_STRUCT;

	this->YSOHGThreshold = 0.0;
	this->YSOLGThreshold = 0.0;

	this->Reset();
}

[[maybe_unused]] bool MtasImplantProcessor::PreProcess(EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();
	auto summary = 	eventhistory->GetCurrentEventSummary();
	summary->GetDetectorSummary(this->AllDefaultRegex["mtasimplant"],this->SummaryData);
	for( const auto& evt : this->SummaryData ){
		auto subtype = evt->GetSubType();
		auto group = evt->GetGroup();
		auto ishighgain = evt->HasTag(this->highgaintag);
		auto islowgain = evt->HasTag(this->lowgaintag);

		int pixelid = -1;

		if( (not ishighgain and not islowgain) or (islowgain and ishighgain) ){
			throw std::runtime_error("evt in MtasImplantProcessor is malformed in xml, and has either both highgain and lowgain tag or neither");
		}

		if( subtype.compare("dynode") == 0 and ishighgain ){
			++(this->HighGainDynodeHits);
			if( evt->GetEnergy() > this->HighGainDynode ){
				this->HighGainDynode = evt->GetEnergy();
				this->HighGainDynodeTS = evt->GetTimeStamp();
			}
		}else if(subtype.compare("dynode") == 0 and islowgain ){
			++(this->LowGainDynodeHits);
			if( evt->GetEnergy() > this->LowGainDynode ){
				this->LowGainDynode = evt->GetEnergy();
				this->LowGainDynodeTS = evt->GetTimeStamp();
			}
		}else if(subtype.compare("anode") == 0 and ishighgain ){
			pixelid = std::stoi(group);
			++(this->HighGainAnodeHitMap[pixelid]);
			++(this->HighGainAnodeHits);
			hismanager->Fill("MTASIMPLANT_7000",evt->GetEnergy(),pixelid);
			if( evt->GetEnergy() > this->YSOHGThreshold ){
				this->HighGainAnodes.at(pixelid) += evt->GetEnergy();
				//auto coarsepixel = this->CalcXY(pixelid);
				//hismanager->Fill("MTASIMPLANT_7012",coarsepixel.first,coarsepixel.second);
			}
		}else if(subtype.compare("anode") == 0 and islowgain ){
			pixelid = std::stoi(group);
			hismanager->Fill("MTASIMPLANT_7003",evt->GetEnergy(),pixelid);
			++(this->LowGainAnodeHitMap[pixelid]);
			++(this->LowGainAnodeHits);
			if( evt->GetEnergy() > this->YSOLGThreshold ){
				this->LowGainAnodes.at(pixelid) += evt->GetEnergy();
				//auto coarsepixel = this->CalcXY(pixelid);
				//hismanager->Fill("MTASIMPLANT_7013",coarsepixel.first,coarsepixel.second);
			}
		}else{
			throw std::runtime_error("unknown subtype"+subtype+" correct subtypes are anode, dynode");
		}
	}

	this->CalcPosition(this->HighGainAnodes,this->HighResHighGainPosition,this->LowResHighGainPosition);
	this->HighGain.highresx = this->HighResHighGainPosition.first;
	this->HighGain.highresy = this->HighResHighGainPosition.second;
	this->HighGain.lowresx = this->LowResHighGainPosition.first;
	this->HighGain.lowresy = this->LowResHighGainPosition.second;
	this->HighGain.dynodeerg = this->HighGainDynode;
	this->HighGain.dynodets = this->HighGainDynodeTS;
	hismanager->Fill("MTASIMPLANT_7012",this->LowResHighGainPosition.first,this->LowResHighGainPosition.second);
	hismanager->Fill("MTASIMPLANT_7014",this->HighResHighGainPosition.first,this->HighResHighGainPosition.second);

	this->CalcPosition(this->LowGainAnodes,this->HighResLowGainPosition,this->LowResLowGainPosition);
	this->LowGain.highresx = this->HighResLowGainPosition.first;
	this->LowGain.highresy = this->HighResLowGainPosition.second;
	this->LowGain.lowresx = this->LowResLowGainPosition.first;
	this->LowGain.lowresy = this->LowResLowGainPosition.second;
	this->LowGain.dynodeerg = this->LowGainDynode;
	this->LowGain.dynodets = this->LowGainDynodeTS;
	hismanager->Fill("MTASIMPLANT_7013",this->LowResLowGainPosition.first,this->LowResLowGainPosition.second);
	hismanager->Fill("MTASIMPLANT_7015",this->HighResLowGainPosition.first,this->HighResLowGainPosition.second);

	hismanager->Fill("MTASIMPLANT_7030",this->HighGainDynodeHits,0);
	hismanager->Fill("MTASIMPLANT_7030",this->LowGainDynodeHits,1);
	hismanager->Fill("MTASIMPLANT_7030",this->HighGainAnodeHits,2);
	hismanager->Fill("MTASIMPLANT_7030",this->LowGainAnodeHits,3);

	for( size_t ii = 0; ii < 64; ++ii ){
		hismanager->Fill("MTASIMPLANT_7040",this->HighGainAnodeHitMap[ii],ii);
		hismanager->Fill("MTASIMPLANT_7043",this->LowGainAnodeHitMap[ii],ii);
	}

	if( this->LowGainDynode > this->IsIonThresh.first and this->LowGainDynode < this->IsIonThresh.second ){
		summary->AddEventTag("ion");
	}

	if( this->HighGainDynode > this->IsBetaThresh.first and this->HighGainDynode < this->IsBetaThresh.second ){
		summary->AddEventTag("beta");
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool MtasImplantProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool MtasImplantProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->Reset();
	return true;
}

void MtasImplantProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");

	this->YSOHGThreshold = config.attribute("yso_thresh_hg").as_double(0.0);
	this->YSOLGThreshold = config.attribute("yso_thresh_lg").as_double(0.0);

	//high gain dynode must be between these two
	this->IsBetaThresh = {
		config.attribute("beta_thresh_low").as_double(50.0),
		config.attribute("beta_thresh_high").as_double(8192.0)
	};

	//low gain dynode must be between these two
	this->IsIonThresh = {
		config.attribute("ion_thresh_low").as_double(2000.0),
		config.attribute("ion_thresh_high").as_double(16384.0)
	};

	this->LoadHistogramSettings(config);
	this->LoadCustomCuts(config);
}
		
void MtasImplantProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void MtasImplantProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager){
	hismanager->RegisterPlot<TH2F>("MTASIMPLANT_7000","High Gain Anodes",this->h2dsettings.at(7000));
	
	hismanager->RegisterPlot<TH2F>("MTASIMPLANT_7003","Low Gain Anodes",this->h2dsettings.at(7003));

	hismanager->RegisterPlot<TH2F>("MTASIMPLANT_7012","High Gain Pixel Position",this->h2dsettings.at(7012));
	hismanager->RegisterPlot<TH2F>("MTASIMPLANT_7013","Low Gain Pixel Position",this->h2dsettings.at(7013));
	hismanager->RegisterPlot<TH2F>("MTASIMPLANT_7014","High Gain Position",this->h2dsettings.at(7014));
	hismanager->RegisterPlot<TH2F>("MTASIMPLANT_7015","Low Gain Position",this->h2dsettings.at(7015));
	
	hismanager->RegisterPlot<TH2F>("MTASIMPLANT_7030","SiPM Mults (DyH,DyL,AnH,AnL)",this->h2dsettings.at(7030));
	hismanager->RegisterPlot<TH2F>("MTASIMPLANT_7040","Individual High Gain Anode Mults",this->h2dsettings.at(7040));
	hismanager->RegisterPlot<TH2F>("MTASIMPLANT_7043","Individual Low Gain Anode Mults",this->h2dsettings.at(7043));

	this->console->info("Finished Declaring Plots");
}

void MtasImplantProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
	this->OutputTree = new TTree("MtasImplant","MtasImplant Processor output");
	this->OutputTree->Branch("highgain",&(this->HighGain));
	this->OutputTree->Branch("lowgain",&(this->LowGain));
	outputtrees[this->ProcessorName] = this->OutputTree;
}

void MtasImplantProcessor::CleanupTree(){
	this->HighGain = ProcessorStruct::DEFAULT_MTAS_IMPLANT_STRUCT;
	this->LowGain = ProcessorStruct::DEFAULT_MTAS_IMPLANT_STRUCT;
}

void MtasImplantProcessor::Reset(){
	this->HighGainAnodeHitMap = std::vector<short>(64,0);
	this->HighGainDynodeHits = 0;
	this->HighGainAnodeHits = 0;
	this->HighGainDynode = 0.0;
	this->HighGainDynodeOQDC = 0.0;
	this->HighGainDynodeTS = -1.0;
	this->HighGainAnodes = std::vector<double>(64,0.0);
	this->HighGainAnodesOQDC = std::vector<double>(64,0.0);
	this->HighResHighGainPosition = std::pair<double,double>(-99.0,-99.0);
	this->LowResHighGainPosition = std::pair<unsigned int,unsigned int>(-99,-99);
	
	this->LowGainAnodeHitMap = std::vector<short>(64,0);
	this->LowGainDynodeHits = 0;
	this->LowGainAnodeHits = 0;
	this->LowGainDynode = 0.0;
	this->LowGainDynodeOQDC = 0.0;
	this->LowGainDynodeTS = -1.0;
	this->LowGainAnodes = std::vector<double>(64,0.0);
	this->LowGainAnodesOQDC = std::vector<double>(64,0.0);
	this->HighResLowGainPosition = std::pair<double,double>(-99.0,-99.0);
	this->LowResLowGainPosition = std::pair<unsigned int,unsigned int>(-99,-99);
}
		
std::pair<unsigned int,unsigned int> MtasImplantProcessor::CalcXY(const unsigned int& idx) const{
	return std::make_pair(idx%8,8-idx/8);
}

void MtasImplantProcessor::CalcPosition(const std::vector<double>& ergs,std::pair<double,double>& highres,std::pair<unsigned int,unsigned int>& lowres){
	double max_erg = ergs.front();
	double esum = 0.0;
	unsigned int idx = 0;
	double xtmp = 0.0;
	double ytmp = 0.0;
	for( const auto& e : ergs ){
		auto pixel = this->CalcXY(idx);
		if( e > max_erg ){
			max_erg = e;
			lowres = pixel;
		}
		esum += e;
		xtmp += e*pixel.first;
		ytmp += e*pixel.second;
		++idx;
	}
	highres = std::make_pair(xtmp/esum,ytmp/esum);
}
