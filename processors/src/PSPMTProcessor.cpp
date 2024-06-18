#include "PSPMTProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>

PSPMTProcessor::PSPMTProcessor(const std::string& log) : Processor(log,"PSPMTProcessor",{"pspmt"}){
	this->NewEvt = {
		.hg = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, { -10.0, -10.0 }},
		.lg = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, { -10.0, -10.0 }},
		.ampdynode = 0.0,
		.Pileup = false,
		.Saturate = false,
		.RealEvt = false
	};
	this->CurrEvt = this->NewEvt;
	this->PrevEvt = this->CurrEvt;
	this->highgaintag = "highgain";
	this->lowgaintag = "lowgain";
	this->Reset();
}

[[maybe_unused]] bool PSPMTProcessor::PreProcess(EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	Processor::PreProcess();

	summary.GetDetectorSummary(this->AllDefaultRegex["pspmt"],this->SummaryData);
	for( const auto& evt : this->SummaryData ){
		auto subtype = evt->GetSubType();
		auto group = evt->GetGroup();
		bool islowgain = evt->HasTag(this->lowgaintag);
		bool ishighgain = evt->HasTag(this->highgaintag);

		if( (ishighgain and islowgain) or (not ishighgain and not islowgain) ){
			throw std::runtime_error("evt in PSPMTProcessor has malformed xml, has both lowgain and highgain tag or neither");
		}

		if( evt->GetSaturation() ){
			this->CurrEvt.Saturate = true;
		}
		if( evt->GetPileup() ){
			this->CurrEvt.Pileup = true;
		}

		if( subtype.compare("anode") == 0 ){
			if( group.compare("xa") == 0 ){
				if( islowgain ){
					if( not this->AnodeLowHits[0] ){
						++this->AnodeLowHits[0];
						this->CurrEvt.lg.xa += evt->GetEnergy();
						++this->CurrEvt.lg.numanodes;
					}else{
						++this->AnodeLowHits[0];
					}
				}else{
					if( not this->AnodeHighHits[0] ){
						++this->AnodeHighHits[0];
						this->CurrEvt.hg.xa += evt->GetEnergy();
						++this->CurrEvt.hg.numanodes;
					}else{
						++this->AnodeHighHits[0];
					}
				}
			}else if( group.compare("xb") == 0 ){
				if( islowgain ){
					if( not this->AnodeLowHits[1] ){
						++this->AnodeLowHits[1];
						this->CurrEvt.lg.xb += evt->GetEnergy();
						++this->CurrEvt.lg.numanodes;
					}else{
						++this->AnodeLowHits[1];
					}
				}else{
					if( not this->AnodeHighHits[1] ){
						++this->AnodeHighHits[1];
						this->CurrEvt.hg.xb += evt->GetEnergy();
						++this->CurrEvt.hg.numanodes;
					}else{
						++this->AnodeHighHits[1];
					}
				}
			}else if( group.compare("ya") == 0 ){
				if( islowgain ){
					if( not this->AnodeLowHits[2] ){
						++this->AnodeLowHits[2];
						this->CurrEvt.lg.ya += evt->GetEnergy();
						++this->CurrEvt.lg.numanodes;
					}else{
						++this->AnodeLowHits[2];
					}
				}else{
					if( not this->AnodeHighHits[2] ){
						++this->AnodeHighHits[2];
						this->CurrEvt.hg.ya += evt->GetEnergy();
						++this->CurrEvt.hg.numanodes;
					}else{
						++this->AnodeHighHits[2];
					}
				}
			}else if( group.compare("yb") == 0 ){
				if( islowgain ){
					if( not this->AnodeLowHits[3] ){
						++this->AnodeLowHits[3];
						this->CurrEvt.lg.yb += evt->GetEnergy();
						++this->CurrEvt.lg.numanodes;
					}else{
						++this->AnodeLowHits[3];
					}
				}else{
					if( not this->AnodeHighHits[3] ){
						++this->AnodeHighHits[3];
						this->CurrEvt.hg.yb += evt->GetEnergy();
						++this->CurrEvt.hg.numanodes;
					}else{
						++this->AnodeHighHits[3];
					}
				}
			}else{
				throw std::runtime_error("evt in PSPMTProcessor has malformed xml, anode is missing xa,xb,ya,yb group");
			}
		}else if( subtype.compare("dynode") == 0 ){
			if( group.compare("amp") == 0 ){
				if( not AmpDynodeHits ){
					++this->AmpDynodeHits;
					this->CurrEvt.ampdynode += evt->GetEnergy();
				}else{
					++this->AmpDynodeHits;
				}
			}else{
				if( islowgain ){
					if( not this->DynodeLowHits ){
						++this->DynodeLowHits;
						this->CurrEvt.lg.dynode += evt->GetEnergy();
					}else{
						++this->DynodeLowHits;
					}
				}else{
					if( not this->DynodeHighHits ){
						++this->DynodeHighHits;
						this->CurrEvt.lg.dynode += evt->GetEnergy();
					}else{
						++this->DynodeHighHits;
					}
				}
			}
		}else{
			throw std::runtime_error("evt in PSPMTProcessor has malformed xml, is neither anode or dynode");
		}
	}

	if( this->CurrEvt.lg.numanodes == 4 ){
		this->CalculatePosition(this->CurrEvt.lg,0.0,0.0,0.0,false);
		hismanager->Fill("PSPMT_1901",this->CurrEvt.lg.position.first,this->CurrEvt.lg.position.second);

		hismanager->Fill("PSPMT_2001",this->CurrEvt.lg.dynode);

		hismanager->Fill("PSPMT_2101",this->CurrEvt.lg.dynode,this->CurrEvt.lg.xa);
		hismanager->Fill("PSPMT_2101",this->CurrEvt.lg.dynode,this->CurrEvt.lg.xb);
		hismanager->Fill("PSPMT_2101",this->CurrEvt.lg.dynode,this->CurrEvt.lg.ya);
		hismanager->Fill("PSPMT_2101",this->CurrEvt.lg.dynode,this->CurrEvt.lg.yb);
		
		hismanager->Fill("PSPMT_2201",this->CurrEvt.lg.xa,0);
		hismanager->Fill("PSPMT_2201",this->CurrEvt.lg.xb,1);
		hismanager->Fill("PSPMT_2201",this->CurrEvt.lg.ya,2);
		hismanager->Fill("PSPMT_2201",this->CurrEvt.lg.yb,3);

		hismanager->Fill("PSPMT_2301",this->CurrEvt.lg.dynode,this->CurrEvt.lg.anodesum);
	}
		
	if( this->CurrEvt.hg.numanodes == 4 ){
		this->CalculatePosition(this->CurrEvt.hg,0.0,0.0,0.0,false);
		hismanager->Fill("PSPMT_1902",this->CurrEvt.hg.position.first,this->CurrEvt.hg.position.second);

		hismanager->Fill("PSPMT_2002",this->CurrEvt.hg.dynode);

		hismanager->Fill("PSPMT_2102",this->CurrEvt.hg.dynode,this->CurrEvt.hg.xa);
		hismanager->Fill("PSPMT_2102",this->CurrEvt.hg.dynode,this->CurrEvt.hg.xb);
		hismanager->Fill("PSPMT_2102",this->CurrEvt.hg.dynode,this->CurrEvt.hg.ya);
		hismanager->Fill("PSPMT_2102",this->CurrEvt.hg.dynode,this->CurrEvt.hg.yb);
		
		hismanager->Fill("PSPMT_2202",this->CurrEvt.hg.xa,0);
		hismanager->Fill("PSPMT_2202",this->CurrEvt.hg.xb,1);
		hismanager->Fill("PSPMT_2202",this->CurrEvt.hg.ya,2);
		hismanager->Fill("PSPMT_2202",this->CurrEvt.hg.yb,3);

		hismanager->Fill("PSPMT_2302",this->CurrEvt.hg.dynode,this->CurrEvt.hg.anodesum);
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool PSPMTProcessor::Process([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	return true;
}

[[maybe_unused]] bool PSPMTProcessor::PostProcess([[maybe_unused]] EventSummary& summary,[[maybe_unused]] PLOTS::PlotRegistry* hismanager,[[maybe_unused]] CUTS::CutRegistry* cutmanager){
	this->Reset();
	return true;
}

void PSPMTProcessor::Init(const YAML::Node& config){
	this->console->info("Init called with YAML::Node");
}

void PSPMTProcessor::Init(const Json::Value& config){
	this->console->info("Init called with Json::Value");
}

void PSPMTProcessor::Init(const pugi::xml_node& config){
	this->console->info("Init called with pugi::xml_node");
}
		
void PSPMTProcessor::Finalize(){
	this->console->info("{} has been finalized",this->ProcessorName);
}

void PSPMTProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) const{
	hismanager->RegisterPlot<TH2F>("PSPMT_1901","Low Gain Image; Position (arb.); Position (arb.)",1024,0,1,1024,0,1);
	hismanager->RegisterPlot<TH2F>("PSPMT_1902","High Gain Image; Position (arb.); Position (arb.)",1024,0,1,1024,0,1);

	hismanager->RegisterPlot<TH1F>("PSPMT_2001","Low Gain Dynode; Energy (arb.)",65536,0,65536);
	hismanager->RegisterPlot<TH1F>("PSPMT_2002","High Gain Dynode; Energy (arb.)",65536,0,65536);
	
	hismanager->RegisterPlot<TH2F>("PSPMT_2101","Low Gain Anodes vs Low Gain Dynode; Energy (arb.)",8192,0,65536,8192,0,65536);
	hismanager->RegisterPlot<TH2F>("PSPMT_2102","High Gain Anodes vs High Gain Dynode; Energy (arb.)",8192,0,65536,8192,0,65536);

	hismanager->RegisterPlot<TH2F>("PSPMT_2201","Inividual Low Gain Anode; Energy (arb.); Position (arb.)",16384,0,65536,4,0,4);
	hismanager->RegisterPlot<TH2F>("PSPMT_2202","Inividual High Gain Anode; Energy (arb.); Position (arb.)",16384,0,65536,4,0,4);
	
	hismanager->RegisterPlot<TH2F>("PSPMT_2301","Low Gain Anodes vs Low Gain Dynode; Energy (arb.)",8192,0,65536,8192,0,65536);
	hismanager->RegisterPlot<TH2F>("PSPMT_2302","High Gain Anodes vs High Gain Dynode; Energy (arb.)",8192,0,65536,8192,0,65536);
	this->console->info("Finished Declaring Plots");
}

void PSPMTProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>& outputtrees){
}

void PSPMTProcessor::CleanupTree(){
}

void PSPMTProcessor::Reset(){
	this->PrevEvt = this->CurrEvt;
	this->CurrEvt = this->NewEvt;
	this->DynodeHighHits = 0;
	this->DynodeLowHits = 0;
	this->AnodeHighHits = std::vector<int>(4,0);
	this->AnodeLowHits = std::vector<int>(4,0);
}

void PSPMTProcessor::CalculatePosition(PSPMTProcessor::Image& img,double rotation,double xcenter,double ycenter,bool xflip){
	double x = 0.0;
	double y = 0.0;
	img.anodesum = img.xa+img.ya+img.xb+img.yb;
	
	if( xflip ){
		x = (img.ya+img.xb)/img.anodesum;
		y = (img.xa+img.xb)/img.anodesum;
	}else{
		x = (img.yb+img.xa)/img.anodesum;
		y = (img.xa+img.xb)/img.anodesum;
	}

	x -= xcenter;
	y -= ycenter;

	img.position = { x*std::cos(rotation) + xcenter -y*std::sin(rotation) + ycenter, x*std::sin(rotation) + xcenter + y*std::cos(rotation) + ycenter};
}
