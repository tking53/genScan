#include "MtasImplantProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "ImageManipulation.hpp"
#include <TTree.h>
#include <stdexcept>

MtasImplantProcessor::MtasImplantProcessor(const std::string& log) : Processor(log,"MtasImplantProcessor",{"mtasimplant"}){

	this->hgImage = { 0.0, 0.0, {0, 0}, {0, 0}, {0.0, 0.0}, {0.0, 0.0}, 0.0};
	this->lgImage = { 0.0, 0.0, {0, 0}, {0, 0}, {0.0, 0.0}, {0.0, 0.0}, 0.0};

	this->h1dsettings = {
		{7005,{16384,0,16384}},
		{7006,{16384,0,16384}}
	};

	this->h2dsettings = {
		{7000,{16384,0,16384,64,0,64}},
		{7003,{16384,0,16384,64,0,64}},

		{7007,{4096,0,4096,4096,0,4096}},
		{70078,{4096,0,32768,4096,0,32768}},

		{7012,{10,-5,5,10,-5,5}},
		{7013,{10,-5,5,10,-5,5}},
		{7014,{1000,-5,5,1000,-5,5}},
		{7015,{1000,-5,5,1000,-5,5}},
		
		{7016,{10,-5,5,10,-5,5}},
		{7017,{10,-5,5,10,-5,5}},
		{7018,{1000,-5,5,1000,-5,5}},
		{7019,{1000,-5,5,1000,-5,5}},
		
		{7020,{4096,0,4096,4096,0,4096}},
		{70208,{4096,0,32768,4096,0,32768}},
		{7021,{4096,0,4096,4096,0,4096}},
		{70218,{4096,0,32768,4096,0,32768}},

		{7030,{64,0,64,4,0,4}},
		{7040,{10,0,10,64,0,64}},
		{7043,{10,0,10,64,0,64}},
		
		{7050,{4096,0,65536,4096,0,64}},
		{7051,{4096,0,65536,4096,0,64}},

		{7052,{64,0,64,8000,-4000,4000}},
		{7053,{64,0,64,8000,-4000,4000}},
		
		{7060,{1000,-5,5,1000,-5,5}},
		{7061,{1000,-5,5,1000,-5,5}},
		{7062,{4096,0,4096,1000,-5,5}},
		{70628,{4096,0,65536,1000,-5,5}},
		{7063,{4096,0,4096,1000,-5,5}},
		{70638,{4096,0,65536,1000,-5,5}},
		{7064,{4096,0,4096,1000,-5,5}},
		{70648,{4096,0,65536,1000,-5,5}},
		{7065,{4096,0,4096,1000,-5,5}},
		{70658,{4096,0,65536,1000,-5,5}},
		{7066,{64,0,64,1000,-5,5}},
		{7067,{64,0,64,1000,-5,5}},
		
		{7070,{1000,-5,5,1000,-5,5}},
		{7071,{1000,-5,5,1000,-5,5}},
		{7072,{4096,0,4096,1000,-5,5}},
		{70728,{4096,0,65536,1000,-5,5}},
		{7073,{4096,0,4096,1000,-5,5}},
		{70738,{4096,0,65536,1000,-5,5}},
		{7074,{4096,0,4096,1000,-5,5}},
		{70748,{4096,0,65536,1000,-5,5}},
		{7075,{4096,0,4096,1000,-5,5}},
		{70758,{4096,0,65536,1000,-5,5}},
		{7076,{64,0,64,1000,-5,5}},
		{7077,{64,0,64,1000,-5,5}}
	};

	this->lowgaintag = "lowgain";
	this->highgaintag = "highgain";
	
	this->HighGain = ProcessorStruct::DEFAULT_MTAS_IMPLANT_STRUCT;
	this->hgPSD = 0.0;
	this->LowGain = ProcessorStruct::DEFAULT_MTAS_IMPLANT_STRUCT;
	this->lgPSD = 0.0;

	this->YSOHGThreshold = 0.0;
	this->YSOLGThreshold = 0.0;
	
	this->HighGainAnodes = std::vector<AnodeHitInfo>(64,AnodeHitInfo{.energy=0.0,.timestamp=-1.0,.hits=0});
	this->LowGainAnodes = std::vector<AnodeHitInfo>(64,AnodeHitInfo{.energy=0.0,.timestamp=-1.0,.hits=0});
	
	for( size_t ii = 0; ii < 8; ++ii ){
		for( size_t jj = 0; jj < 8; ++jj ){
			this->PositionMap.push_back({-3.5+jj,3.5-ii});
			this->console->debug("PositionMap[{}] : ({},{})",ii*8+jj,-3.5+jj,3.5-ii);
		}
	}

	this->Reset();
}

[[maybe_unused]] bool MtasImplantProcessor::PreProcess(EventHistoryManager* eventhistory,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();
	auto summary = 	eventhistory->GetCurrentEventSummary();
	summary->GetDetectorSummary(this->AllDefaultRegex["mtasimplant"],this->SummaryData);
	auto IMPLANT_7000 = hismanager->GetPlot<TH2*>("IMPLANT_7000");
	auto IMPLANT_7003 = hismanager->GetPlot<TH2*>("IMPLANT_7003");

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
			++(this->HighGainAnodes[pixelid].hits);
			this->HighGainAnodes[pixelid].energy += evt->GetEnergy();
			this->HighGainAnodes[pixelid].timestamp = evt->GetTimeStamp();
			++(this->HighGainAnodeHits);
			IMPLANT_7000->Fill(evt->GetEnergy(),pixelid);
			if( evt->GetEnergy() > this->YSOHGThreshold ){
				this->HighGainAnodes.at(pixelid).energy += evt->GetEnergy();
			}

		}else if(subtype.compare("anode") == 0 and islowgain ){
			pixelid = std::stoi(group);
			IMPLANT_7003->Fill(evt->GetEnergy(),pixelid);
			++(this->LowGainAnodes[pixelid].hits);
			this->LowGainAnodes[pixelid].energy += evt->GetEnergy();
			this->LowGainAnodes[pixelid].timestamp = evt->GetTimeStamp();
			++(this->LowGainAnodeHits);
			if( evt->GetEnergy() > this->YSOLGThreshold ){
				this->LowGainAnodes.at(pixelid).energy += evt->GetEnergy();
			}

		}else{
			throw std::runtime_error("unknown subtype"+subtype+" correct subtypes are anode, dynode");
		}
	}

	this->CalcPosition(this->HighGainAnodes,this->hgImage);
	for( auto& anode : this->HighGainAnodes ){
		if( anode.energy > 0.0 ){
			this->hgImage.numanodes += 1;
		}
	}
	//if( this->HighGainAnodeHits > 1 ){
	//	auto HGIndices = this->get_sorted_indices(this->HighGainAnodes);
	//	this->hgImage.secondarylowResPosition = this->CalcXY(HGIndices[1]);
	//}else{
		this->hgImage.secondarylowResPosition = {-999,-999};
	//}
	this->HighGain.highresx = this->hgImage.highResPosition.first;
	this->HighGain.highresy = this->hgImage.highResPosition.second;
	this->HighGain.lowresx = this->hgImage.lowResPosition.first;
	this->HighGain.lowresy = this->hgImage.lowResPosition.second;
	this->HighGain.dynodeerg = this->hgImage.dynode;
	this->HighGain.dynodets = this->hgImage.DynodeTimeStamp;
	this->HighGain.anodesum = this->hgImage.anodesum;
	this->HighGain.numanodes = this->hgImage.numanodes;
	hismanager->Fill("IMPLANT_7005",this->hgImage.dynode);
	hismanager->Fill("IMPLANT_7012",this->hgImage.lowResPosition.first,this->hgImage.lowResPosition.second);
	hismanager->Fill("IMPLANT_7014",this->hgImage.highResPosition.first,this->hgImage.highResPosition.second);
	hismanager->Fill("IMPLANT_7016",this->hgImage.secondarylowResPosition.first,this->hgImage.secondarylowResPosition.second);
	hismanager->Fill("IMPLANT_7018",this->hgImage.highResStdDev.first,this->hgImage.highResStdDev.second);
	hismanager->Fill("IMPLANT_7050",this->hgImage.dynode,this->hgPSD);
	
	hismanager->Fill("IMPLANT_7060",this->hgImage.highResPosition.first,this->hgImage.highResStdDev.first);
	hismanager->Fill("IMPLANT_7061",this->hgImage.highResPosition.second,this->hgImage.highResStdDev.second);
	hismanager->Fill("IMPLANT_7062",this->hgImage.dynode,this->hgImage.highResStdDev.first);
	hismanager->Fill("IMPLANT_70628",this->hgImage.dynode,this->hgImage.highResStdDev.first);
	hismanager->Fill("IMPLANT_7063",this->hgImage.dynode,this->hgImage.highResStdDev.second);
	hismanager->Fill("IMPLANT_70638",this->hgImage.dynode,this->hgImage.highResStdDev.second);
	hismanager->Fill("IMPLANT_7064",this->hgImage.anodesum,this->hgImage.highResStdDev.first);
	hismanager->Fill("IMPLANT_70648",this->hgImage.anodesum,this->hgImage.highResStdDev.first);
	hismanager->Fill("IMPLANT_7065",this->hgImage.anodesum,this->hgImage.highResStdDev.second);
	hismanager->Fill("IMPLANT_70658",this->hgImage.anodesum,this->hgImage.highResStdDev.second);
	hismanager->Fill("IMPLANT_7066",this->HighGainAnodeHits,this->hgImage.highResStdDev.first);
	hismanager->Fill("IMPLANT_7067",this->HighGainAnodeHits,this->hgImage.highResStdDev.second);

	this->CalcPosition(this->LowGainAnodes,this->lgImage);
	for( auto& anode : this->LowGainAnodes ){
		if( anode.energy > 0.0 ){
			this->lgImage.numanodes += 1;
		}
	}
	//if( this->LowGainAnodeHits > 1 ){
	//	auto LGIndices = this->get_sorted_indices(this->LowGainAnodes);
	//	this->lgImage.secondarylowResPosition = this->CalcXY(LGIndices[1]);
	//}else{
		this->lgImage.secondarylowResPosition = {-999,-999};
	//}
	this->LowGain.highresx = this->lgImage.highResPosition.first;
	this->LowGain.highresy = this->lgImage.highResPosition.second;
	this->LowGain.lowresx = this->lgImage.lowResPosition.first;
	this->LowGain.lowresy = this->lgImage.lowResPosition.second;
	this->LowGain.dynodeerg = this->lgImage.dynode;
	this->LowGain.dynodets = this->lgImage.DynodeTimeStamp;
	this->LowGain.anodesum = this->lgImage.anodesum;
	this->LowGain.numanodes = this->lgImage.numanodes;
	hismanager->Fill("IMPLANT_7006",this->lgImage.dynode);
	hismanager->Fill("IMPLANT_7013",this->lgImage.lowResPosition.first,this->lgImage.lowResPosition.second);
	hismanager->Fill("IMPLANT_7015",this->lgImage.highResPosition.first,this->lgImage.highResPosition.second);
	hismanager->Fill("IMPLANT_7017",this->lgImage.secondarylowResPosition.first,this->lgImage.secondarylowResPosition.second);
	hismanager->Fill("IMPLANT_7019",this->lgImage.highResStdDev.first,this->lgImage.highResStdDev.second);
	hismanager->Fill("IMPLANT_7051",this->lgImage.dynode,this->lgPSD);

	hismanager->Fill("IMPLANT_7070",this->lgImage.highResPosition.first,this->lgImage.highResStdDev.first);
	hismanager->Fill("IMPLANT_7071",this->lgImage.highResPosition.second,this->lgImage.highResStdDev.second);
	hismanager->Fill("IMPLANT_7072",this->lgImage.dynode,this->lgImage.highResStdDev.first);
	hismanager->Fill("IMPLANT_70728",this->lgImage.dynode,this->lgImage.highResStdDev.first);
	hismanager->Fill("IMPLANT_7073",this->lgImage.dynode,this->lgImage.highResStdDev.second);
	hismanager->Fill("IMPLANT_70738",this->lgImage.dynode,this->lgImage.highResStdDev.second);
	hismanager->Fill("IMPLANT_7074",this->lgImage.anodesum,this->lgImage.highResStdDev.first);
	hismanager->Fill("IMPLANT_70748",this->lgImage.anodesum,this->lgImage.highResStdDev.first);
	hismanager->Fill("IMPLANT_7075",this->lgImage.anodesum,this->lgImage.highResStdDev.second);
	hismanager->Fill("IMPLANT_70758",this->lgImage.anodesum,this->lgImage.highResStdDev.second);
	hismanager->Fill("IMPLANT_7076",this->LowGainAnodeHits,this->lgImage.highResStdDev.first);
	hismanager->Fill("IMPLANT_7077",this->LowGainAnodeHits,this->lgImage.highResStdDev.first);
	hismanager->Fill("IMPLANT_7076",this->LowGainAnodeHits,this->lgImage.highResStdDev.first);
	hismanager->Fill("IMPLANT_7077",this->LowGainAnodeHits,this->lgImage.highResStdDev.second);

	hismanager->Fill("IMPLANT_7007",this->lgImage.dynode,this->hgImage.dynode);
	hismanager->Fill("IMPLANT_70078",this->lgImage.dynode,this->hgImage.dynode);

	hismanager->Fill("IMPLANT_7020" ,this->hgImage.anodesum,this->hgImage.dynode);
	hismanager->Fill("IMPLANT_70208",this->hgImage.anodesum,this->hgImage.dynode);
	hismanager->Fill("IMPLANT_7021" ,this->lgImage.anodesum,this->lgImage.dynode);
	hismanager->Fill("IMPLANT_70218",this->lgImage.anodesum,this->lgImage.dynode);

	//prefetch since we fill more than once
	auto IMPLANT_7030 = hismanager->GetPlot<TH2*>("IMPLANT_7030");
	IMPLANT_7030->Fill(this->HighGainDynodeHits,0);
	IMPLANT_7030->Fill(this->LowGainDynodeHits,1);
	IMPLANT_7030->Fill(this->HighGainAnodeHits,2);
	IMPLANT_7030->Fill(this->LowGainAnodeHits,3);

	//prefetch since we fill more than once
	auto IMPLANT_7040 = hismanager->GetPlot<TH2*>("IMPLANT_7040");
	auto IMPLANT_7043 = hismanager->GetPlot<TH2*>("IMPLANT_7043");
	auto IMPLANT_7052 = hismanager->GetPlot<TH2 *>("IMPLANT_7052");
	auto IMPLANT_7053 = hismanager->GetPlot<TH2*>("IMPLANT_7053");
	for( size_t ii = 0; ii < 64; ++ii ){
		IMPLANT_7040->Fill(this->HighGainAnodes[ii].hits,ii);
		IMPLANT_7043->Fill(this->LowGainAnodes[ii].hits,ii);
		IMPLANT_7052->Fill(ii,this->HighGainAnodes[ii].timestamp - this->hgImage.DynodeTimeStamp);
		IMPLANT_7053->Fill(ii,this->LowGainAnodes[ii].timestamp - this->lgImage.DynodeTimeStamp);
	}

	if( this->lgImage.dynode > this->IsIonThresh.first and this->lgImage.dynode < this->IsIonThresh.second ){
		summary->AddEventTag("ion");
		summary->AddEventObservable("ION_X",this->lgImage.highResPosition.first);
		summary->AddEventObservable("ION_Y",this->lgImage.highResPosition.second);
		summary->AddEventObservable("ION_Energy",this->lgImage.DynodeTimeStamp);
		summary->AddEventObservable("ION_TS",this->lgImage.DynodeTimeStamp);
	}

	if( this->hgImage.dynode > this->IsBetaThresh.first and this->hgImage.dynode < this->IsBetaThresh.second ){
		summary->AddEventTag("beta");
		summary->AddEventObservable("BETA_X",this->hgImage.highResPosition.first);
		summary->AddEventObservable("BETA_Y",this->hgImage.highResPosition.second);
		summary->AddEventObservable("BETA_Energy",this->hgImage.dynode);
		summary->AddEventObservable("BETA_TS",this->hgImage.DynodeTimeStamp);
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

	hismanager->RegisterPlot<TH2F>("IMPLANT_7012","Max High Gain Pixel Position; X (pixels); Y (pixels)",this->h2dsettings.at(7012));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7013","Max Low Gain Pixel Position; X (pixels); Y (pixels)",this->h2dsettings.at(7013));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7014","High Gain Position; X (pixels); Y (pixels)",this->h2dsettings.at(7014));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7015","Low Gain Position; X (pixels); Y (pixels)",this->h2dsettings.at(7015));
	
	hismanager->RegisterPlot<TH2F>("IMPLANT_7016","Secondary Max High Gain Pixel Position; X (pixels); Y (pixels)",this->h2dsettings.at(7016));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7017","Secondary Max Low Gain Pixel Position; X (pixels); Y (pixels)",this->h2dsettings.at(7017));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7018","High Gain StdDev; X (pixels); Y (pixels)",this->h2dsettings.at(7018));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7019","Low Gain StdDev; X (pixels); Y (pixels)",this->h2dsettings.at(7019));
	
	
	hismanager->RegisterPlot<TH2F>("IMPLANT_7020","High Gain dynode vs High Gain Anode Sum; Anode Sum Energy (keV); Dynode Energy (keV);",this->h2dsettings.at(7020));
	hismanager->RegisterPlot<TH2F>("IMPLANT_70208","High Gain dynode vs High Gain Anode Sum; Anode Sum Energy (8 keV/bin); Dynode Energy (8 keV/bin);",this->h2dsettings.at(70208));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7021","Low Gain dynode vs Low Gain Anode Sum; Anode Sum Energy (keV); Dynode Energy (keV);",this->h2dsettings.at(7021));
	hismanager->RegisterPlot<TH2F>("IMPLANT_70218","Low Gain dynode vs Low Gain Anode Sum; Anode Sum Energy (8 keV/bin); Dynode Energy (8 keV/bin);",this->h2dsettings.at(70218));

	hismanager->RegisterPlot<TH2F>("IMPLANT_7030","SiPM Mults (DyH,DyL,AnH,AnL)",this->h2dsettings.at(7030));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7040","Individual High Gain Anode Mults",this->h2dsettings.at(7040));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7043","Individual Low Gain Anode Mults",this->h2dsettings.at(7043));
	
	hismanager->RegisterPlot<TH2F>("IMPLANT_7050","High Gain Dynode PSD (Head/Tail); Energy (keV); PSD (arb.)",this->h2dsettings.at(7050));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7051","Low Gain Dynode PSD (Head/Tail); Energy (keV); PSD (arb.)",this->h2dsettings.at(7051));

	hismanager->RegisterPlot<TH2F>("IMPLANT_7052" ,"High Gain: TDiff: Anode - Dynode ; Pixel Number (arb.); TDiff (ns) ",this->h2dsettings.at(7052));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7053" , "Low Gain: TDiff: Anode - Dynode ; Pixel Number (arb.); TDiff (ns) ",this->h2dsettings.at(7053));

	hismanager->RegisterPlot<TH2F>("IMPLANT_7060", "High Gain X StdDev. vs X Position; Pixel (arb.); Pixel (arb.)",this->h2dsettings.at(7060));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7061" ,"High Gain Y StdDev. vs Y Position; Pixel (arb.); Pixel (arb.)",this->h2dsettings.at(7061));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7062" ,"High Gain X StdDev. vs Dynode; Energy (keV); Pixel (arb.)",this->h2dsettings.at(7062));
	hismanager->RegisterPlot<TH2F>("IMPLANT_70628","High Gain X StdDev. vs Dynode; Energy (keV); Pixel (arb.)",this->h2dsettings.at(70628));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7063" ,"High Gain Y StdDev. vs Dynode; Energy (keV); Pixel (arb.)",this->h2dsettings.at(7063));
	hismanager->RegisterPlot<TH2F>("IMPLANT_70638","High Gain Y StdDev. vs Dynode; Energy (keV); Pixel (arb.)",this->h2dsettings.at(70638));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7064" ,"High Gain X StdDev. vs Anode Sum; Energy (keV); Pixel (arb.)",this->h2dsettings.at(7064));
	hismanager->RegisterPlot<TH2F>("IMPLANT_70648","High Gain X StdDev. vs Anode Sum; Energy (keV); Pixel (arb.)",this->h2dsettings.at(70648));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7065" ,"High Gain Y StdDev. vs Anode Sum; Energy (keV); Pixel (arb.)",this->h2dsettings.at(7065));
	hismanager->RegisterPlot<TH2F>("IMPLANT_70658","High Gain Y StdDev. vs Anode Sum; Energy (keV); Pixel (arb.)",this->h2dsettings.at(70658));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7066" ,"High Gain X StdDev. vs Anode Mult.; Mult. (arb.); Pixel (arb.)",this->h2dsettings.at(7066));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7067" ,"High Gain Y StdDev. vs Anode Mult.; Mult. (arb.); Pixel (arb.)",this->h2dsettings.at(7067));

	hismanager->RegisterPlot<TH2F>("IMPLANT_7070" ,"Low Gain X StdDev. vs X Position; Pixel (arb.); Pixel (arb.)",this->h2dsettings.at(7070));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7071" ,"Low Gain Y StdDev. vs Y Position; Pixel (arb.); Pixel (arb.)",this->h2dsettings.at(7071));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7072" ,"Low Gain X StdDev. vs Dynode; Energy (keV); Pixel (arb.)",this->h2dsettings.at(7072));
	hismanager->RegisterPlot<TH2F>("IMPLANT_70728","Low Gain X StdDev. vs Dynode; Energy (keV); Pixel (arb.)",this->h2dsettings.at(70728));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7073" ,"Low Gain Y StdDev. vs Dynode; Energy (keV); Pixel (arb.)",this->h2dsettings.at(7073));
	hismanager->RegisterPlot<TH2F>("IMPLANT_70738","Low Gain Y StdDev. vs Dynode; Energy (keV); Pixel (arb.)",this->h2dsettings.at(70738));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7074" ,"Low Gain X StdDev. vs Anode Sum; Energy (keV); Pixel (arb.)",this->h2dsettings.at(7074));
	hismanager->RegisterPlot<TH2F>("IMPLANT_70748","Low Gain X StdDev. vs Anode Sum; Energy (keV); Pixel (arb.)",this->h2dsettings.at(70748));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7075" ,"Low Gain Y StdDev. vs Anode Sum; Energy (keV); Pixel (arb.)",this->h2dsettings.at(7075));
	hismanager->RegisterPlot<TH2F>("IMPLANT_70758","Low Gain Y StdDev. vs Anode Sum; Energy (keV); Pixel (arb.)",this->h2dsettings.at(70758));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7076" ,"Low Gain X StdDev. vs Anode Mult.; Mult. (arb.); Pixel (arb.)",this->h2dsettings.at(7076));
	hismanager->RegisterPlot<TH2F>("IMPLANT_7077" ,"Low Gain Y StdDev. vs Anode Mult.; Mult. (arb.); Pixel (arb.)",this->h2dsettings.at(7077));

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

	for( auto& hit : this->HighGainAnodes ){
		hit.energy=0.0;
		hit.timestamp=-1.0;
		hit.hits=0;
	}
	for( auto& hit : this->LowGainAnodes ){
		hit.energy=0.0;
		hit.timestamp=-1.0;
		hit.hits=0;
	}

	this->HighGainDynodeHits = 0;
	this->HighGainAnodeHits = 0;
	this->hgImage.ResetAnode();
	this->hgImage.ResetDynode();
	this->hgImage.ResetHighResPosition();
	this->hgImage.ResetHighResStdDev();
	this->hgImage.ResetLowResPosition();
	this->hgImage.ResetSecondaryLowResPosition();
	this->hgPSD = 0.0;
	
	this->LowGainDynodeHits = 0;
	this->LowGainAnodeHits = 0;
	this->lgImage.ResetAnode();
	this->lgImage.ResetDynode();
	this->lgImage.ResetHighResPosition();
	this->lgImage.ResetHighResStdDev();
	this->lgImage.ResetLowResPosition();
	this->lgImage.ResetSecondaryLowResPosition();
	this->lgPSD = 0.0;
}
		
std::pair<double,double> MtasImplantProcessor::CalcXY(const unsigned int& idx) const{
	//return std::make_pair(idx%8,8-idx/8);
	return this->PositionMap[idx];
}

void MtasImplantProcessor::CalcPosition(const std::vector<AnodeHitInfo>& anodes,SIPMIMP::Image& img){
	double max_erg = anodes.front().energy;
	double esum = 0.0;
	unsigned int idx = 0;
	double xtmp = 0.0;
	double xtmp2 = 0.0;
	double ytmp = 0.0;
	double ytmp2 = 0.0;
	for( const auto& e : anodes ){
		auto pixel = this->CalcXY(idx);
		if( e.energy > max_erg ){
			max_erg = e.energy;
			img.lowResPosition = pixel;
		}
		esum += e.energy;
		//need to get the edges to be -4,-3,-2,-1,0,1,2,3,4
		//with the centers being -3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5
		xtmp += e.energy*pixel.first;
		xtmp2 += e.energy*pixel.first*pixel.first;
		ytmp += e.energy*pixel.second;
		ytmp2 += e.energy*pixel.second*pixel.second;
		++idx;
	}
	img.anodesum = esum;
	img.highResPosition = std::make_pair(xtmp/esum,ytmp/esum);
	std::pair<double,double> pos = std::make_pair(xtmp/esum,ytmp/esum);
	std::pair<double,double> possqr = std::make_pair(xtmp2/esum,ytmp2/esum);
	double xdev = (std::sqrt(possqr.first) > pos.first) ? std::sqrt(possqr.first - pos.first*pos.first) : -std::sqrt(possqr.first - pos.first*pos.first);
	double ydev = (std::sqrt(possqr.second) > pos.second) ? std::sqrt(possqr.second - pos.second*pos.second) : -std::sqrt(possqr.second - pos.second*pos.second);
	img.highResStdDev = std::make_pair(xdev,ydev);
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

const double& MtasImplantProcessor::GetLowGainPSD() const{
	return this->lgPSD;
}

std::vector<size_t> MtasImplantProcessor::get_sorted_indices(const std::vector<double>& values) {
	// Create a vector of indices
	std::vector<size_t> indices(values.size());
	std::iota(indices.begin(), indices.end(), 0);

	// Sort the indices based on the values
	std::sort(indices.begin(), indices.end(),[&](size_t i1, size_t i2) { return values[i1] > values[i2]; });
	return indices;
}
