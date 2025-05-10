#include "MtasImplantProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>

MtasImplantProcessor::MtasImplantProcessor(const std::string& log) : Processor(log,"MtasImplantProcessor",{"mtasimplant"}){

	this->hgImage = { 0.0, 0.0, {0, 0}, {0.0, 0.0}, 0.0};
	this->lgImage = { 0.0, 0.0, {0, 0}, {0.0, 0.0}, 0.0};

	this->h1dsettings = {
		{7005,{16384,0,16384}},
		{7006,{16384,0,16384}}
	};

	this->h2dsettings = {
		{7000,{16384,0,16384,64,0,64}},
		{7003,{16384,0,16384,64,0,64}},

		{7007,{4096,0,4096,4096,0,4096}},
		{70078,{4096,0,32768,4096,0,32768}},

		{7008,{16384,0,16384,1024,-1,1}},
		{7009,{16384,0,16384,1024,-1,1}},

		{7012,{10,0,10,10,0,10}},
		{7013,{10,0,10,10,0,10}},
		{7014,{1000,0,10,1000,0,10}},
		{7015,{1000,0,10,1000,0,10}},
		
		{7020,{4096,0,4096,4096,0,4096}},
		{70208,{4096,0,32768,4096,0,32768}},
		{7021,{4096,0,4096,4096,0,4096}},
		{70218,{4096,0,32768,4096,0,32768}},

		{7030,{10,0,10,4,0,4}},
		{7040,{10,0,10,64,0,64}},
		{7043,{10,0,10,64,0,64}},
		
		{7050,{4096,0,65536,4096,0,64}}
	};

	this->lowgaintag = "lowgain";
	this->highgaintag = "highgain";
	
	this->HighGain = ProcessorStruct::DEFAULT_MTAS_IMPLANT_STRUCT;
	this->hgPSD = 0.0;
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
			if( evt->GetEnergy() > this->hgImage.dynode ){
				this->hgImage.dynode = evt->GetEnergy();
				this->hgImage.DynodeTimeStamp = evt->GetTimeStamp();
				this->hgPSD = std::get<0>(evt->GetTraceFixedPSD())/std::get<1>(evt->GetTraceFixedPSD());
			}
		}else if(subtype.compare("dynode") == 0 and islowgain ){
			++(this->LowGainDynodeHits);
			if( evt->GetEnergy() > this->lgImage.dynode ){
				this->lgImage.dynode = evt->GetEnergy();
				this->lgImage.DynodeTimeStamp = evt->GetTimeStamp();
			}
		}else if(subtype.compare("anode") == 0 and ishighgain ){
			pixelid = std::stoi(group);
			++(this->HighGainAnodeHitMap[pixelid]);
			++(this->HighGainAnodeHits);
			hismanager->Fill("IMPLANT_7000",evt->GetEnergy(),pixelid);
			if( evt->GetEnergy() > this->YSOHGThreshold ){
				this->HighGainAnodes.at(pixelid) += evt->GetEnergy();
				//auto coarsepixel = this->CalcXY(pixelid);
				//hismanager->Fill("IMPLANT_7012",coarsepixel.first,coarsepixel.second);
			}
		}else if(subtype.compare("anode") == 0 and islowgain ){
			pixelid = std::stoi(group);
			hismanager->Fill("IMPLANT_7003",evt->GetEnergy(),pixelid);
			++(this->LowGainAnodeHitMap[pixelid]);
			++(this->LowGainAnodeHits);
			if( evt->GetEnergy() > this->YSOLGThreshold ){
				this->LowGainAnodes.at(pixelid) += evt->GetEnergy();
				//auto coarsepixel = this->CalcXY(pixelid);
				//hismanager->Fill("IMPLANT_7013",coarsepixel.first,coarsepixel.second);
			}
		}else{
			throw std::runtime_error("unknown subtype"+subtype+" correct subtypes are anode, dynode");
		}
	}


	this->CalcPosition(this->HighGainAnodes,this->hgImage.highResPosition,this->hgImage.lowResPosition, this->hgImage.anodesum);
	this->HighGain.highresx = this->hgImage.highResPosition.first;
	this->HighGain.highresy = this->hgImage.highResPosition.second;
	this->HighGain.lowresx = this->hgImage.lowResPosition.first;
	this->HighGain.lowresy = this->hgImage.lowResPosition.second;
	this->HighGain.dynodeerg = this->hgImage.dynode;
	this->HighGain.dynodets = this->hgImage.DynodeTimeStamp;
	this->HighGain.anodesum = this->hgImage.anodesum;
	hismanager->Fill("IMPLANT_7005",this->hgImage.dynode);
	hismanager->Fill("IMPLANT_7012",this->hgImage.lowResPosition.first,this->hgImage.lowResPosition.second);
	hismanager->Fill("IMPLANT_7014",this->hgImage.highResPosition.first,this->hgImage.highResPosition.second);
	hismanager->Fill("IMPLANT_7050",this->hgImage.dynode,this->hgPSD);
	//need to gen psd
	//hismanager->Fill("IMPLANT_7008",this->hgImage.dynode,this->hgPSD);

	this->CalcPosition(this->LowGainAnodes,this->lgImage.highResPosition,this->lgImage.lowResPosition, this->lgImage.anodesum);
	this->LowGain.highresx = this->lgImage.highResPosition.first;
	this->LowGain.highresy = this->lgImage.highResPosition.second;
	this->LowGain.lowresx = this->lgImage.lowResPosition.first;
	this->LowGain.lowresy = this->lgImage.lowResPosition.second;
	this->LowGain.dynodeerg = this->lgImage.dynode;
	this->LowGain.dynodets = this->lgImage.DynodeTimeStamp;
	this->LowGain.anodesum = this->lgImage.anodesum;
	hismanager->Fill("IMPLANT_7006",this->lgImage.dynode);
	hismanager->Fill("IMPLANT_7013",this->lgImage.lowResPosition.first,this->lgImage.lowResPosition.second);
	hismanager->Fill("IMPLANT_7015",this->lgImage.highResPosition.first,this->lgImage.highResPosition.second);
	//need to gen psd
	//hismanager->Fill("IMPLANT_7009",this->lgImage.dynode,this->lgPSD);

	hismanager->Fill("IMPLANT_7007",this->lgImage.dynode,this->hgImage.dynode);
	hismanager->Fill("IMPLANT_70078",this->lgImage.dynode,this->hgImage.dynode);

	hismanager->Fill("IMPLANT_7020",this->hgImage.dynode,this->hgImage.anodesum);
	hismanager->Fill("IMPLANT_70208",this->hgImage.dynode,this->hgImage.anodesum);
	hismanager->Fill("IMPLANT_7021",this->lgImage.dynode,this->lgImage.anodesum);
	hismanager->Fill("IMPLANT_70218",this->lgImage.dynode,this->lgImage.anodesum);

	hismanager->Fill("IMPLANT_7030",this->HighGainDynodeHits,0);
	hismanager->Fill("IMPLANT_7030",this->LowGainDynodeHits,1);
	hismanager->Fill("IMPLANT_7030",this->HighGainAnodeHits,2);
	hismanager->Fill("IMPLANT_7030",this->LowGainAnodeHits,3);

	for( size_t ii = 0; ii < 64; ++ii ){
		hismanager->Fill("IMPLANT_7040",this->HighGainAnodeHitMap[ii],ii);
		hismanager->Fill("IMPLANT_7043",this->LowGainAnodeHitMap[ii],ii);
	}

	if( this->lgImage.dynode > this->IsIonThresh.first and this->lgImage.dynode < this->IsIonThresh.second ){
		summary->AddEventTag("ion");
		summary->AddEventObservable("ION_X",this->lgImage.highResPosition.first);
		summary->AddEventObservable("ION_Y",this->lgImage.highResPosition.second);
	}

	if( this->hgImage.dynode > this->IsBetaThresh.first and this->hgImage.dynode < this->IsBetaThresh.second ){
		summary->AddEventTag("beta");
		summary->AddEventObservable("BETA_X",this->hgImage.highResPosition.first);
		summary->AddEventObservable("BETA_Y",this->hgImage.highResPosition.second);
		summary->AddEventObservable("BETA_Energy",this->hgImage.dynode);
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
	hismanager->RegisterPlot<TH2F>("IMPLANT_7000","High Gain Anodes; Energy (keV); Anode Number (arb.)",this->h2dsettings.at(7000));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7003","Low Gain Anodes; Energy (keV); Anode Number (arb.)",this->h2dsettings.at(7003));
	hismanager->RegisterPlot<TH1F>("IMPLANT_7005","High Gain Dynode; Energy (keV)",this->h1dsettings.at(7005));	
	hismanager->RegisterPlot<TH1F>("IMPLANT_7006","Low Gain Dynode; Energy (keV)",this->h1dsettings.at(7005));	

	hismanager->RegisterPlot<TH2F>("IMPLANT_7007","High Gain dynode vs Low Gain dynode; Low Gain Dynode Energy (keV); High Gain Dynode Energy (keV);",this->h2dsettings.at(7007));
	hismanager->RegisterPlot<TH2F>("IMPLANT_70078","High Gain dynode vs Low Gain dynode; Low Gain Dynode Energy (8 keV/bin); High Gain Dynode Energy (8 keV/bin);",this->h2dsettings.at(70078));

	hismanager->RegisterPlot<TH2F>("IMPLANT_7008","High Gain Dynode PSD; Energy (keV); PSD (arb.)",this->h2dsettings.at(7008));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7009","Low Gain Dynode PSD; Energy (keV); PSD (arb.)",this->h2dsettings.at(7009));

	hismanager->RegisterPlot<TH2F>("IMPLANT_7012","High Gain Pixel Position; X (pixels); Y (pixels)",this->h2dsettings.at(7012));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7013","Low Gain Pixel Position; X (pixels); Y (pixels)",this->h2dsettings.at(7013));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7014","High Gain Position; X (pixels); Y (pixels)",this->h2dsettings.at(7014));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7015","Low Gain Position; X (pixels); Y (pixels)",this->h2dsettings.at(7015));
	
	
	hismanager->RegisterPlot<TH2F>("IMPLANT_7020","High Gain dynode vs High Gain Anode Sum; Anode Sum Energy (keV); Dynode Energy (keV);",this->h2dsettings.at(7020));
	hismanager->RegisterPlot<TH2F>("IMPLANT_70208","High Gain dynode vs High Gain Anode Sum; Anode Sum Energy (8 keV/bin); Dynode Energy (8 keV/bin);",this->h2dsettings.at(70208));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7021","Low Gain dynode vs Low Gain Anode Sum; Anode Sum Energy (keV); Dynode Energy (keV);",this->h2dsettings.at(7021));
	hismanager->RegisterPlot<TH2F>("IMPLANT_70218","Low Gain dynode vs Low Gain Anode Sum; Anode Sum Energy (8 keV/bin); Dynode Energy (8 keV/bin);",this->h2dsettings.at(70218));

	hismanager->RegisterPlot<TH2F>("IMPLANT_7030","SiPM Mults (DyH,DyL,AnH,AnL)",this->h2dsettings.at(7030));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7040","Individual High Gain Anode Mults",this->h2dsettings.at(7040));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7043","Individual Low Gain Anode Mults",this->h2dsettings.at(7043));
	
	hismanager->RegisterPlot<TH2F>("IMPLANT_7050","High Gain Dynode PSD (Head/Tail); Energy (keV); PSD (arb.)",this->h2dsettings.at(7050));

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
	this->HighGainAnodes = std::vector<double>(64,0.0);
	this->hgImage.ResetAnode();
	this->hgImage.ResetDynode();
	this->hgImage.ResetHighResPosition();
	this->hgImage.ResetLowResPosition();
	this->hgPSD = 0.0;
	
	this->LowGainAnodeHitMap = std::vector<short>(64,0);
	this->LowGainDynodeHits = 0;
	this->LowGainAnodeHits = 0;
	this->LowGainAnodes = std::vector<double>(64,0.0);
	this->lgImage.ResetAnode();
	this->lgImage.ResetDynode();
	this->lgImage.ResetHighResPosition();
	this->lgImage.ResetLowResPosition();
}
		
std::pair<unsigned int,unsigned int> MtasImplantProcessor::CalcXY(const unsigned int& idx) const{
	return std::make_pair(idx%8,8-idx/8);
}

void MtasImplantProcessor::CalcPosition(const std::vector<double>& ergs,std::pair<double,double>& highres,std::pair<unsigned int,unsigned int>& lowres, double& anodeSum){
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
	anodeSum = esum;
	highres = std::make_pair(xtmp/esum,ytmp/esum);
}

const SIPMIMP::Image& MtasImplantProcessor::GetLowGainImage() const{
	return this->lgImage;
}

const SIPMIMP::Image& MtasImplantProcessor::GetHighGainImage() const{
	return this->hgImage;
}

const double& MtasImplantProcessor::GetHighGainPSD() const{
	return this->hgPSD;
}
