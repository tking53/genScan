#include "PSPMTProcessor.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include <TTree.h>
#include <stdexcept>

PSPMTProcessor::PSPMTProcessor(const std::string& log)
	: Processor(log, "PSPMTProcessor", {"pspmt"}) {
	this->h1dsettings = {
		{2001, {65536, 0, 65536}},
		{2002, {65536, 0, 65536}}};

	this->h2dsettings = {
		{1901, {1024, 0, 1, 1024, 0, 1}},
		{1902, {1024, 0, 1, 1024, 0, 1}},
		{1903, {1024, 0, 1, 1024, 0, 1}},
		{1904, {1024, 0, 1, 1024, 0, 1}},

		{2101, {8192, 0, 65536, 8192, 0, 65536}},
		{2102, {8192, 0, 65536, 8192, 0, 65536}},

		{2003, {30, 0, 30, 5, 0, 5}},
		{2004, {30, 0, 30, 5, 0, 5}},

		{2201, {16384, 0, 65536, 4, 0, 4}},
		{2202, {16384, 0, 65536, 4, 0, 4}},

		{2201, {16384, 0, 65536, 4, 0, 4}},
		{2202, {16384, 0, 65536, 4, 0, 4}},


		{2203, {1024, 0, 1,16384, 0, 65536}},
		{2204, {1024, 0, 1,16384, 0, 65536}},
		{2205, {1024, 0, 1,16384, 0, 65536}},
		{2206, {1024, 0, 1,16384, 0, 65536}},

		{2211, {16384, 0, 65536, 16, 0, 16}},
		{2212, {16384, 0, 65536, 16, 0, 16}},

		{2301, {8192, 0, 65536, 8192, 0, 4 * 65536}},
		{2302, {8192, 0, 65536, 8192, 0, 4 * 65536}},
		{2303, {8192, 0, 65536, 8192, 0, 4 * 65536}},
		{2304, {8192, 0, 65536, 8192, 0, 4 * 65536}}};

	this->NumRequiredHighAnodes = 4;
	this->NumRequiredLowAnodes = 4;

	this->hgImage = {0.0, 0, {-10.0, -10.0}, 0.0,{0.0,false,false},{0.0,false,false},{0.0,false,false},{0.0,false,false},{0.0,false,false}};
	this->lgImage = {0.0, 0, {-10.0, -10.0}, 0.0,{0.0,false,false},{0.0,false,false},{0.0,false,false},{0.0,false,false},{0.0,false,false}};
	this->hgImageQdc = {0.0, 0, {-10.0, -10.0}, 0.0,{0.0,false,false},{0.0,false,false},{0.0,false,false},{0.0,false,false},{0.0,false,false}};
	this->lgImageQdc = {0.0, 0, {-10.0, -10.0}, 0.0,{0.0,false,false},{0.0,false,false},{0.0,false,false},{0.0,false,false},{0.0,false,false}};
	this->ampdynode = 0.0;
	this->highgaintag = "highgain";
	this->lowgaintag = "lowgain";
	this->AnodeHighHits = std::vector<int>(4, 0);
	this->AnodeLowHits = std::vector<int>(4, 0);
	this->Reset();
}

[[maybe_unused]] bool PSPMTProcessor::PreProcess(EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	Processor::PreProcess();

	auto summary = eventhistory->GetCurrentEventSummary();
	summary->GetDetectorSummary(this->AllDefaultRegex["pspmt"], this->SummaryData);
	for (const auto& evt : this->SummaryData) {
		auto subtype = evt->GetSubType();
		auto group = evt->GetGroup();
		bool islowgain = evt->HasTag(this->lowgaintag);
		bool ishighgain = evt->HasTag(this->highgaintag);
		bool hasQDCs = !evt->GetRawQDCSums().empty();

		if ((ishighgain and islowgain) or (not ishighgain and not islowgain)) {
			this->console->error("evt: {} in PSPMTProcessor has malformed xml, has both lowgain and highgain tag or neither", *evt);
			throw std::runtime_error("invalid xml config");
		}

		if (subtype.compare("anode") == 0) {
			// The logic for the QDCs here comes directly from PAASS and the UTK folks for how they setup things
			if (group.compare("xa") == 0) {
				if (islowgain) {
					if (not this->AnodeLowHits[0]) {
						++this->AnodeLowHits[0];
						this->lgImage.xa.energy += evt->GetEnergy();
						this->lgImage.xa.saturation = evt->GetSaturation();
						this->lgImage.xa.pileup = evt->GetPileup();
						++this->lgImage.numanodes;
						if (hasQDCs) {
							this->lgImageQdc.xa.energy += (evt->GetRawQDCSums()[0] - evt->GetRawQDCSums()[2]);
							++this->lgImageQdc.numanodes;
						}
					} else {
						++this->AnodeLowHits[0];
					}
				} else {
					if (not this->AnodeHighHits[0]) {
						++this->AnodeHighHits[0];
						this->hgImage.xa.energy += evt->GetEnergy();
						this->hgImage.xa.saturation = evt->GetSaturation();
						this->hgImage.xa.pileup = evt->GetPileup();
						++this->hgImage.numanodes;
						if (hasQDCs) {
							this->hgImageQdc.xa.energy += (evt->GetRawQDCSums()[0] - evt->GetRawQDCSums()[2]);
							++this->hgImageQdc.numanodes;
						}
					} else {
						++this->AnodeHighHits[0];
					}
				}
			} else if (group.compare("xb") == 0) {
				if (islowgain) {
					if (not this->AnodeLowHits[1]) {
						++this->AnodeLowHits[1];
						this->lgImage.xb.energy += evt->GetEnergy();
						this->lgImage.xb.saturation = evt->GetSaturation();
						this->lgImage.xb.pileup = evt->GetPileup();
						++this->lgImage.numanodes;
						if (hasQDCs) {
							this->lgImageQdc.xb.energy += (evt->GetRawQDCSums()[0] - evt->GetRawQDCSums()[2]);
							++this->lgImageQdc.numanodes;
						}
					} else {
						++this->AnodeLowHits[1];
					}
				} else {
					if (not this->AnodeHighHits[1]) {
						++this->AnodeHighHits[1];
						this->hgImage.xb.energy += evt->GetEnergy();
						this->hgImage.xb.saturation = evt->GetSaturation();
						this->hgImage.xb.pileup = evt->GetPileup();
						++this->hgImage.numanodes;
						if (hasQDCs) {
							this->hgImageQdc.xb.energy += (evt->GetRawQDCSums()[0] - evt->GetRawQDCSums()[2]);
							++this->hgImageQdc.numanodes;
						}
					} else {
						++this->AnodeHighHits[1];
					}
				}
			} else if (group.compare("ya") == 0) {
				if (islowgain) {
					if (not this->AnodeLowHits[2]) {
						++this->AnodeLowHits[2];
						this->lgImage.ya.energy += evt->GetEnergy();
						this->lgImage.ya.saturation = evt->GetSaturation();
						this->lgImage.ya.pileup = evt->GetPileup();
						++this->lgImage.numanodes;
						if (hasQDCs) {
							this->lgImageQdc.ya.energy += (evt->GetRawQDCSums()[0] - evt->GetRawQDCSums()[2]);
							++this->lgImageQdc.numanodes;
						}
					} else {
						++this->AnodeLowHits[2];
					}
				} else {
					if (not this->AnodeHighHits[2]) {
						++this->AnodeHighHits[2];
						this->hgImage.ya.energy += evt->GetEnergy();
						this->hgImage.ya.saturation = evt->GetSaturation();
						this->hgImage.ya.pileup = evt->GetPileup();
						++this->hgImage.numanodes;
						if (hasQDCs) {
							this->hgImageQdc.ya.energy += (evt->GetRawQDCSums()[0] - evt->GetRawQDCSums()[2]);
							++this->hgImageQdc.numanodes;
						}
					} else {
						++this->AnodeHighHits[2];
					}
				}
			} else if (group.compare("yb") == 0) {
				if (islowgain) {
					if (not this->AnodeLowHits[3]) {
						++this->AnodeLowHits[3];
						this->lgImage.yb.energy += evt->GetEnergy();
						this->lgImage.yb.saturation = evt->GetSaturation();
						this->lgImage.yb.pileup = evt->GetPileup();
						++this->lgImage.numanodes;
						if (hasQDCs) {
							this->lgImageQdc.yb.energy += (evt->GetRawQDCSums()[0] - evt->GetRawQDCSums()[2]);
							++this->lgImageQdc.numanodes;
						}
					} else {
						++this->AnodeLowHits[3];
					}
				} else {
					if (not this->AnodeHighHits[3]) {
						++this->AnodeHighHits[3];
						this->hgImage.yb.energy += evt->GetEnergy();
						this->hgImage.yb.saturation = evt->GetSaturation();
						this->hgImage.yb.pileup = evt->GetPileup();
						++this->hgImage.numanodes;
						if (hasQDCs) {
							this->hgImageQdc.yb.energy += (evt->GetRawQDCSums()[0] - evt->GetRawQDCSums()[2]);
							++this->hgImageQdc.numanodes;
						}
					} else {
						++this->AnodeHighHits[3];
					}
				}
			} else {
				throw std::runtime_error("evt in PSPMTProcessor has malformed xml, anode is missing xa,xb,ya,yb group");
			}
		} else if (subtype.compare("dynode") == 0) {
			if (group.compare("amp") == 0) {
				if (not AmpDynodeHits) {
					++this->AmpDynodeHits;
					this->ampdynode += evt->GetEnergy();
				} else {
					++this->AmpDynodeHits;
				}
			} else {
				if (islowgain) {
					if (not this->DynodeLowHits) {
						++this->DynodeLowHits;
						this->lgImage.dynode.energy += evt->GetEnergy();
						this->lgImage.DynodeTimeStamp = evt->GetTimeStamp();
						this->lgImage.dynode.saturation = evt->GetSaturation();
						this->lgImage.dynode.pileup = evt->GetPileup();
						if (hasQDCs) {
							this->lgImageQdc.dynode.energy += (evt->GetRawQDCSums()[0] - evt->GetRawQDCSums()[2]);
							this->lgImageQdc.DynodeTimeStamp = evt->GetTimeStamp();
						}
					} else {
						++this->DynodeLowHits;
					}
				} else {
					if (not this->DynodeHighHits) {
						++this->DynodeHighHits;
						this->hgImage.dynode.energy += evt->GetEnergy();
						this->hgImage.dynode.saturation = evt->GetSaturation();
						this->hgImage.dynode.pileup = evt->GetPileup();
						this->hgImage.DynodeTimeStamp = evt->GetTimeStamp();
						if (hasQDCs) {
							this->hgImageQdc.dynode.energy += (evt->GetRawQDCSums()[0] - evt->GetRawQDCSums()[2]);
							this->hgImageQdc.DynodeTimeStamp = evt->GetTimeStamp();
						}
					} else {
						++this->DynodeHighHits;
					}
				}
			}
		} else {
			throw std::runtime_error("evt in PSPMTProcessor has malformed xml, is neither anode or dynode");
		}
	}

	this->lgImage.anodesum = this->lgImage.xa.energy + this->lgImage.ya.energy + this->lgImage.xb.energy + this->lgImage.yb.energy;
	this->lgImageQdc.anodesum = this->lgImageQdc.xa.energy + this->lgImageQdc.ya.energy + this->lgImageQdc.xb.energy + this->lgImageQdc.yb.energy;
	hismanager->Fill("PSPMT_2211", this->lgImage.anodesum, this->lgImage.numanodes);
	hismanager->Fill("PSPMT_2003_U", this->DynodeLowHits, 0);
	hismanager->Fill("PSPMT_2003_U", this->AnodeLowHits[0], 1);
	hismanager->Fill("PSPMT_2003_U", this->AnodeLowHits[1], 2);
	hismanager->Fill("PSPMT_2003_U", this->AnodeLowHits[2], 3);
	hismanager->Fill("PSPMT_2003_U", this->AnodeLowHits[3], 4);

	if (not this->lgImage.dynode.saturation && not this->lgImage.dynode.pileup) {
		hismanager->Fill("PSPMT_2003", this->DynodeLowHits, 0);
		hismanager->Fill("PSPMT_2003", this->AnodeLowHits[0], 1);
		hismanager->Fill("PSPMT_2003", this->AnodeLowHits[1], 2);
		hismanager->Fill("PSPMT_2003", this->AnodeLowHits[2], 3);
		hismanager->Fill("PSPMT_2003", this->AnodeLowHits[3], 4);
	}

	if (this->lgImage.numanodes == this->NumRequiredLowAnodes) {
		this->CalculatePosition(this->lgImage, 0.0, 0.0, 0.0, false, this->CurrMethod);
		this->CalculatePosition(this->lgImageQdc, 0.0, 0.0, 0.0, false, this->CurrMethod);
		//! QDC Plot Fills
		hismanager->Fill("PSPMT_1903_U", this->lgImageQdc.position.first, this->lgImageQdc.position.second);
		hismanager->Fill("PSPMT_2303_U", this->lgImageQdc.dynode.energy, this->lgImageQdc.anodesum);
		hismanager->Fill("PSPMT_2301_U", this->lgImageQdc.dynode.energy, this->lgImageQdc.anodesum);

		//! Filter Energy Plots
		hismanager->Fill("PSPMT_1901_U", this->lgImage.position.first, this->lgImage.position.second);
		hismanager->Fill("PSPMT_2001_U", this->lgImage.dynode.energy);
		hismanager->Fill("PSPMT_2101_U", this->lgImage.dynode.energy, this->lgImage.xa.energy);
		hismanager->Fill("PSPMT_2101_U", this->lgImage.dynode.energy, this->lgImage.xb.energy);
		hismanager->Fill("PSPMT_2101_U", this->lgImage.dynode.energy, this->lgImage.ya.energy);
		hismanager->Fill("PSPMT_2101_U", this->lgImage.dynode.energy, this->lgImage.yb.energy);

		hismanager->Fill("PSPMT_2201", this->lgImage.xa.energy, 0);
		hismanager->Fill("PSPMT_2201", this->lgImage.xb.energy, 1);
		hismanager->Fill("PSPMT_2201", this->lgImage.ya.energy, 2);
		hismanager->Fill("PSPMT_2201", this->lgImage.yb.energy, 3);

		hismanager->Fill("PSPMT_2203_U", this->lgImage.position.first, this->lgImage.dynode.energy);
		hismanager->Fill("PSPMT_2204_U", this->lgImage.position.second, this->lgImage.dynode.energy);

		hismanager->Fill("PSPMT_2301_U", this->lgImage.dynode.energy, this->lgImage.anodesum);

		if (not this->lgImage.dynode.saturation && not this->lgImage.dynode.pileup){
			hismanager->Fill("PSPMT_1901", this->lgImage.position.first, this->lgImage.position.second);
			hismanager->Fill("PSPMT_2001", this->lgImage.dynode.energy);
			hismanager->Fill("PSPMT_2101", this->lgImage.dynode.energy, this->lgImage.xa.energy);
			hismanager->Fill("PSPMT_2101", this->lgImage.dynode.energy, this->lgImage.xb.energy);
			hismanager->Fill("PSPMT_2101", this->lgImage.dynode.energy, this->lgImage.ya.energy);
			hismanager->Fill("PSPMT_2101", this->lgImage.dynode.energy, this->lgImage.yb.energy);
			hismanager->Fill("PSPMT_2203", this->lgImage.position.first, this->lgImage.dynode.energy);
			hismanager->Fill("PSPMT_2204", this->lgImage.position.second, this->lgImage.dynode.energy);
			hismanager->Fill("PSPMT_2301", this->lgImage.dynode.energy, this->lgImage.anodesum);

			//! QDC Plot Fills
			hismanager->Fill("PSPMT_1903", this->lgImageQdc.position.first, this->lgImageQdc.position.second);
			hismanager->Fill("PSPMT_2303", this->lgImageQdc.dynode.energy, this->lgImageQdc.anodesum);
		}

		

		FillRootStruct(this->lowgain, this->lgImage, this->lgImageQdc);
	}

	this->hgImage.anodesum = this->hgImage.xa.energy + this->hgImage.ya.energy + this->hgImage.xb.energy + this->hgImage.yb.energy;
	this->hgImageQdc.anodesum = this->hgImageQdc.xa.energy + this->hgImageQdc.ya.energy + this->hgImageQdc.xb.energy + this->hgImageQdc.yb.energy;
	hismanager->Fill("PSPMT_2212", this->hgImage.anodesum, this->hgImage.numanodes);
	
	hismanager->Fill("PSPMT_2004_U", this->DynodeHighHits, 0);
	hismanager->Fill("PSPMT_2004_U", this->AnodeHighHits[0], 1);
	hismanager->Fill("PSPMT_2004_U", this->AnodeHighHits[1], 2);
	hismanager->Fill("PSPMT_2004_U", this->AnodeHighHits[2], 3);
	hismanager->Fill("PSPMT_2004_U", this->AnodeHighHits[3], 4);

	if (not this->hgImage.dynode.saturation && not this->hgImage.dynode.pileup) {
		hismanager->Fill("PSPMT_2004", this->DynodeHighHits, 0);
		hismanager->Fill("PSPMT_2004", this->AnodeHighHits[0], 1);
		hismanager->Fill("PSPMT_2004", this->AnodeHighHits[1], 2);
		hismanager->Fill("PSPMT_2004", this->AnodeHighHits[2], 3);
		hismanager->Fill("PSPMT_2004", this->AnodeHighHits[3], 4);
	}
	
	// this->console->info("num anodes : {}",this->hgImage.numanodes);
	if (this->hgImage.numanodes == this->NumRequiredHighAnodes) {
		this->CalculatePosition(this->hgImage, 0.0, 0.0, 0.0, false, this->CurrMethod);
		this->CalculatePosition(this->hgImageQdc, 0.0, 0.0, 0.0, false, this->CurrMethod);
		//! QDC Plot Fills
		hismanager->Fill("PSPMT_1903_U", this->hgImageQdc.position.first, this->hgImageQdc.position.second);
		hismanager->Fill("PSPMT_2303_U", this->hgImageQdc.dynode.energy, this->hgImageQdc.anodesum);
		hismanager->Fill("PSPMT_2301_U", this->hgImageQdc.dynode.energy, this->hgImageQdc.anodesum);

		//! Filter Energy Plots
		hismanager->Fill("PSPMT_1902_U", this->hgImage.position.first, this->hgImage.position.second);
		hismanager->Fill("PSPMT_2002_U", this->hgImage.dynode.energy);

		hismanager->Fill("PSPMT_2102_U", this->hgImage.dynode.energy, this->hgImage.xa.energy);
		hismanager->Fill("PSPMT_2102_U", this->hgImage.dynode.energy, this->hgImage.xb.energy);
		hismanager->Fill("PSPMT_2102_U", this->hgImage.dynode.energy, this->hgImage.ya.energy);
		hismanager->Fill("PSPMT_2102_U", this->hgImage.dynode.energy, this->hgImage.yb.energy);

		hismanager->Fill("PSPMT_2202", this->hgImage.xa.energy, 0);
		hismanager->Fill("PSPMT_2202", this->hgImage.xb.energy, 1);
		hismanager->Fill("PSPMT_2202", this->hgImage.ya.energy, 2);
		hismanager->Fill("PSPMT_2202", this->hgImage.yb.energy, 3);

		hismanager->Fill("PSPMT_2205_U", this->hgImage.position.first, this->hgImage.dynode.energy);
		hismanager->Fill("PSPMT_2206_U", this->hgImage.position.second, this->hgImage.dynode.energy);

		hismanager->Fill("PSPMT_2302", this->hgImage.dynode.energy, this->hgImage.anodesum);

		if (not this->hgImage.dynode.saturation && not this->hgImage.dynode.pileup) {
			hismanager->Fill("PSPMT_1902", this->hgImage.position.first, this->hgImage.position.second);
			hismanager->Fill("PSPMT_2002", this->hgImage.dynode.energy);
			hismanager->Fill("PSPMT_2102", this->hgImage.dynode.energy, this->hgImage.xa.energy);
			hismanager->Fill("PSPMT_2102", this->hgImage.dynode.energy, this->hgImage.xb.energy);
			hismanager->Fill("PSPMT_2102", this->hgImage.dynode.energy, this->hgImage.ya.energy);
			hismanager->Fill("PSPMT_2102", this->hgImage.dynode.energy, this->hgImage.yb.energy);

			hismanager->Fill("PSPMT_2205", this->hgImage.position.first, this->hgImage.dynode.energy);
			hismanager->Fill("PSPMT_2206", this->hgImage.position.second, this->hgImage.dynode.energy);

			//! QDC Plot Fills
			hismanager->Fill("PSPMT_1904", this->hgImageQdc.position.first, this->hgImageQdc.position.second);
			hismanager->Fill("PSPMT_2304", this->hgImageQdc.dynode.energy, this->hgImageQdc.anodesum);
		}

		FillRootStruct(this->highgain, this->hgImage, this->hgImageQdc);
	}

	Processor::EndProcess();
	return true;
}

[[maybe_unused]] bool PSPMTProcessor::Process([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	return true;
}

[[maybe_unused]] bool PSPMTProcessor::PostProcess([[maybe_unused]] EventHistoryManager* eventhistory, [[maybe_unused]] PLOTS::PlotRegistry* hismanager, [[maybe_unused]] CUTS::CutRegistry* cutmanager) {
	this->Reset();
	return true;
}

void PSPMTProcessor::Init(const pugi::xml_node& config) {
	this->console->info("Init called with pugi::xml_node");
	std::string methodname = config.attribute("method").as_string("corners");
	if (methodname.compare("corners") == 0) {
		this->CurrMethod = PSPMTProcessor::IMAGEMETHOD::CORNERS;
	} else if (methodname.compare("sides") == 0) {
		this->CurrMethod = PSPMTProcessor::IMAGEMETHOD::SIDES;
	} else {
		this->console->error("Unknown image calculation method: {} not corners or sides", methodname);
		throw std::runtime_error("invalid xml config");
	}

	pugi::xml_node hg = config.child("HighGain");
	if (hg) {
		this->console->info("Found HighGain tag for PSPMTProcessor");
		this->NumRequiredHighAnodes = hg.attribute("numanodes").as_int(4);
		this->hgDynodeECuts.first = hg.attribute("dyLowE").as_float(0.0);
		this->hgDynodeECuts.second = hg.attribute("dyHighE").as_float(65536.0);
	}

	pugi::xml_node lg = config.child("LowGain");
	if (lg) {
		this->console->info("Found LowGain tag for PSPMTProcessor");
		this->NumRequiredLowAnodes = lg.attribute("numanodes").as_int(4);
		this->lgDynodeECuts.first = lg.attribute("dyLowE").as_float(0.0);
	}
}

void PSPMTProcessor::Finalize() {
	this->console->info("{} has been finalized", this->ProcessorName);
}

void PSPMTProcessor::DeclarePlots(PLOTS::PlotRegistry* hismanager) {
	hismanager->RegisterPlot<TH2F>("PSPMT_1901", "Low Gain Image; Position (arb.); Position (arb.)", this->h2dsettings.at(1901));
	hismanager->RegisterPlot<TH2F>("PSPMT_1902", "High Gain Image; Position (arb.); Position (arb.)", this->h2dsettings.at(1902));
	hismanager->RegisterPlot<TH2F>("PSPMT_1903", "Low Gain QDC::Image; Position (arb.); Position (arb.)", this->h2dsettings.at(1903));
	hismanager->RegisterPlot<TH2F>("PSPMT_1904", "High Gain QDC::Image; Position (arb.); Position (arb.)", this->h2dsettings.at(1904));

	hismanager->RegisterPlot<TH2F>("PSPMT_1901_U", "Low Gain Image : Ungated; Position (arb.); Position (arb.)", this->h2dsettings.at(1901));
	hismanager->RegisterPlot<TH2F>("PSPMT_1902_U", "High Gain Image : Ungated; Position (arb.); Position (arb.)", this->h2dsettings.at(1902));
	hismanager->RegisterPlot<TH2F>("PSPMT_1903_U", "Low Gain QDC::Image : Ungated; Position (arb.); Position (arb.)", this->h2dsettings.at(1903));
	hismanager->RegisterPlot<TH2F>("PSPMT_1904_U", "High Gain QDC::Image : Ungated; Position (arb.); Position (arb.)", this->h2dsettings.at(1904));

	hismanager->RegisterPlot<TH1F>("PSPMT_2001", "Low Gain Dynode; Energy (arb.)", this->h1dsettings.at(2001));
	hismanager->RegisterPlot<TH1F>("PSPMT_2002", "High Gain Dynode; Energy (arb.)", this->h1dsettings.at(2002));
	hismanager->RegisterPlot<TH2F>("PSPMT_2003", "Low Gain Multiplicities (Dynode, xa, xb, ya, yb); Multiplicity (arb.)", this->h2dsettings.at(2003));
	hismanager->RegisterPlot<TH2F>("PSPMT_2004", "High Gain Multiplicities (Dynode, xa, xb, ya, yb); Multiplicity (arb.)", this->h2dsettings.at(2004));

	hismanager->RegisterPlot<TH1F>("PSPMT_2001_U", "Low Gain Dynode: Ungated; Energy (arb.)", this->h1dsettings.at(2001));
	hismanager->RegisterPlot<TH1F>("PSPMT_2002_U", "High Gain Dynode: Ungated; Energy (arb.)", this->h1dsettings.at(2002));
	hismanager->RegisterPlot<TH2F>("PSPMT_2003_U", "Low Gain Multiplicities: Ungated (Dynode, xa, xb, ya, yb); Multiplicity (arb.)", this->h2dsettings.at(2003));
	hismanager->RegisterPlot<TH2F>("PSPMT_2004_U", "High Gain Multiplicities: Ungated (Dynode, xa, xb, ya, yb); Multiplicity (arb.)", this->h2dsettings.at(2004));

	hismanager->RegisterPlot<TH2F>("PSPMT_2101", "Low Gain Anodes vs Low Gain Dynode; Energy (arb.)", this->h2dsettings.at(2101));
	hismanager->RegisterPlot<TH2F>("PSPMT_2102", "High Gain Anodes vs High Gain Dynode; Energy (arb.)", this->h2dsettings.at(2102));

	hismanager->RegisterPlot<TH2F>("PSPMT_2101_U", "Low Gain Anodes vs Low Gain Dynode: Ungated; Energy (arb.)", this->h2dsettings.at(2101));
	hismanager->RegisterPlot<TH2F>("PSPMT_2102_U", "High Gain Anodes vs High Gain Dynode: Ungated; Energy (arb.)", this->h2dsettings.at(2102));

	hismanager->RegisterPlot<TH2F>("PSPMT_2201", "Inividual Low Gain Anode; Energy (arb.); Position (arb.)", this->h2dsettings.at(2201));
	hismanager->RegisterPlot<TH2F>("PSPMT_2202", "Inividual High Gain Anode; Energy (arb.); Position (arb.)", this->h2dsettings.at(2201));

	// Babel Plots
	hismanager->RegisterPlot<TH2F>("PSPMT_2203", "Low Gain Dynode vs Low Gain X position; Position (arb.); Energy (arb.)", this->h2dsettings.at(2203));
	hismanager->RegisterPlot<TH2F>("PSPMT_2204", "Low Gain Dynode vs Low Gain Y position; Position (arb.); Energy (arb.)", this->h2dsettings.at(2204));
	hismanager->RegisterPlot<TH2F>("PSPMT_2205", "High Gain Dynode vs High Gain X position; Position (arb.); Energy (arb.)", this->h2dsettings.at(2205));
	hismanager->RegisterPlot<TH2F>("PSPMT_2206", "High Gain Dynode vs High Gain Y position; Position (arb.); Energy (arb.)", this->h2dsettings.at(2206));

	hismanager->RegisterPlot<TH2F>("PSPMT_2203_U", "Low Gain Dynode vs Low Gain X position: Ungated; Position (arb.); Energy (arb.)", this->h2dsettings.at(2203));
	hismanager->RegisterPlot<TH2F>("PSPMT_2204_U", "Low Gain Dynode vs Low Gain Y position: Ungated; Position (arb.); Energy (arb.)", this->h2dsettings.at(2204));
	hismanager->RegisterPlot<TH2F>("PSPMT_2205_U", "High Gain Dynode vs High Gain X position: Ungated; Position (arb.); Energy (arb.)", this->h2dsettings.at(2205));
	hismanager->RegisterPlot<TH2F>("PSPMT_2206_U", "High Gain Dynode vs High Gain Y position: Ungated; Position (arb.); Energy (arb.)", this->h2dsettings.at(2206));


	hismanager->RegisterPlot<TH2F>("PSPMT_2211", "Low Gain Num Anodes vs AnodeSum; Energy (arb.); Mult. (arb.)", this->h2dsettings.at(2211));
	hismanager->RegisterPlot<TH2F>("PSPMT_2212", "High Gain Num Anodes vs AnodeSum; Energy (arb.); Mult. (arb.)", this->h2dsettings.at(2212));

	hismanager->RegisterPlot<TH2F>("PSPMT_2301", "Low Gain Anodesum vs Low Gain Dynode; Energy (arb.)", this->h2dsettings.at(2301));
	hismanager->RegisterPlot<TH2F>("PSPMT_2302", "High Gain Anodesum vs High Gain Dynode; Energy (arb.)", this->h2dsettings.at(2302));
	hismanager->RegisterPlot<TH2F>("PSPMT_2303", "Low Gain QDC::Anodesum vs Low Gain QDC::Dynode; Energy (arb.)", this->h2dsettings.at(2303));
	hismanager->RegisterPlot<TH2F>("PSPMT_2304", "High Gain QDC::Anodesum vs High Gain QDC::Dynode; Energy (arb.)", this->h2dsettings.at(2304));
	
	hismanager->RegisterPlot<TH2F>("PSPMT_2301_U", "Low Gain Anodesum vs Low Gain Dynode: Ungated; Energy (arb.)", this->h2dsettings.at(2301));
	hismanager->RegisterPlot<TH2F>("PSPMT_2302_U", "High Gain Anodesum vs High Gain Dynode: Ungated; Energy (arb.)", this->h2dsettings.at(2302));
	hismanager->RegisterPlot<TH2F>("PSPMT_2303_U", "Low Gain QDC::Anodesum vs Low Gain QDC::Dynode: Ungated; Energy (arb.)", this->h2dsettings.at(2303));
	hismanager->RegisterPlot<TH2F>("PSPMT_2304_U", "High Gain QDC::Anodesum vs High Gain QDC::Dynode: Ungated; Energy (arb.)", this->h2dsettings.at(2304));

	this->console->info("Finished Declaring Plots");
}

void PSPMTProcessor::RegisterTree([[maybe_unused]] std::unordered_map<std::string, TTree*>& outputtrees) {
	this->OutputTree = new TTree("Pspmt", "PSPMT Processor Output");
	this->OutputTree->Branch("highgain", &(this->highgain));
	this->OutputTree->Branch("lowgain", &(this->lowgain));
	outputtrees[this->ProcessorName] = this->OutputTree;
}

void PSPMTProcessor::CleanupTree() {
	this->highgain = ProcessorStruct::DEFAULT_PSPMT_STRUCT;
	this->lowgain = ProcessorStruct::DEFAULT_PSPMT_STRUCT;
}

void PSPMTProcessor::Reset() {
	this->hgImage.ResetDynode();
	this->hgImage.ResetAnode();
	this->hgImage.ResetPosition(-10.0, -10.0);
	this->hgImage.ResetCorners();
	this->hgImage.ResetAnodeSum();

	this->lgImage.ResetDynode();
	this->lgImage.ResetAnode();
	this->lgImage.ResetPosition(-10.0, -10.0);
	this->lgImage.ResetCorners();
	this->lgImage.ResetAnodeSum();

	this->hgImageQdc.ResetDynode();
	this->hgImageQdc.ResetAnode();
	this->hgImageQdc.ResetPosition(-10.0, -10.0);
	this->hgImageQdc.ResetCorners();
	this->hgImageQdc.ResetAnodeSum();

	this->lgImageQdc.ResetDynode();
	this->lgImageQdc.ResetAnode();
	this->lgImageQdc.ResetPosition(-10.0, -10.0);
	this->lgImageQdc.ResetCorners();
	this->lgImageQdc.ResetAnodeSum();

	this->DynodeHighHits = 0;
	this->DynodeLowHits = 0;
	for (int ii = 0; ii < 4; ++ii) {
		this->AnodeHighHits[ii] = 0;
		this->AnodeLowHits[ii] = 0;
	}
}

void PSPMTProcessor::CalculatePosition(PSPMT::Image& img, double rotation, double xcenter, double ycenter, bool xflip, PSPMTProcessor::IMAGEMETHOD& method) {
	double x = 0.0;
	double y = 0.0;

	if (method == PSPMTProcessor::IMAGEMETHOD::CORNERS) {
		if (xflip) {
			x = (img.ya.energy + img.xb.energy) / img.anodesum;
			y = (img.xa.energy + img.xb.energy) / img.anodesum;
		} else {
			x = (img.yb.energy + img.xa.energy) / img.anodesum;
			y = (img.xa.energy + img.xb.energy) / img.anodesum;
		}

		x -= xcenter;
		y -= ycenter;
	} else if (method == PSPMTProcessor::IMAGEMETHOD::SIDES) {
		if (xflip) {
			x = (img.xa.energy - img.xb.energy) / (img.xa.energy + img.xb.energy);
			y = (img.ya.energy - img.yb.energy) / (img.ya.energy + img.yb.energy);
		} else {
			x = (img.xb.energy - img.xa.energy) / (img.xa.energy + img.xb.energy);
			y = (img.yb.energy - img.ya.energy) / (img.ya.energy + img.yb.energy);
		}
	}

	img.position = {x * std::cos(rotation) + xcenter - y * std::sin(rotation) + ycenter, x * std::sin(rotation) + xcenter + y * std::cos(rotation) + ycenter};
}

void PSPMTProcessor::FillRootStruct(ProcessorStruct::PSPMT& Rstruct, const PSPMT::Image& enImage, const PSPMT::Image& qdcImage) {
	Rstruct.xpos = enImage.position.first;
	Rstruct.ypos = enImage.position.second;
	Rstruct.xposqdc = qdcImage.position.first;
	Rstruct.yposqdc = qdcImage.position.second;
	Rstruct.anodesumen = enImage.anodesum;
	Rstruct.anodesumqdc = qdcImage.anodesum;
	Rstruct.dynode.energy = enImage.dynode.energy;
	Rstruct.dynode.qdc = qdcImage.dynode.energy;
	Rstruct.dynodets = enImage.DynodeTimeStamp;
	Rstruct.dynode.saturation = enImage.dynode.saturation;
	Rstruct.dynode.pileup = enImage.dynode.pileup;
	Rstruct.xa.energy = enImage.xa.energy;
	Rstruct.xb.energy = enImage.xb.energy;
	Rstruct.ya.energy = enImage.ya.energy;
	Rstruct.yb.energy = enImage.yb.energy;
	Rstruct.xa.saturation = enImage.xa.saturation;
	Rstruct.xb.saturation = enImage.xb.saturation;
	Rstruct.ya.saturation = enImage.ya.saturation;
	Rstruct.yb.saturation = enImage.yb.saturation;
	Rstruct.xa.pileup = enImage.xa.pileup;
	Rstruct.xb.pileup = enImage.xb.pileup;
	Rstruct.ya.pileup = enImage.ya.pileup;
	Rstruct.yb.pileup = enImage.yb.pileup;
	
}

const PSPMT::Image& PSPMTProcessor::GetLowGainImage() const {
	return this->lgImage;
}

const PSPMT::Image& PSPMTProcessor::GetHighGainImage() const {
	return this->hgImage;
}
const PSPMT::Image& PSPMTProcessor::GetLowGainImageQdc() const {
	return this->lgImageQdc;
}

const PSPMT::Image& PSPMTProcessor::GetHighGainImageQdc() const {
	return this->hgImageQdc;
}
