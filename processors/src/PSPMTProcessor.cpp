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

		{2201, {16384, 0, 65536, 4, 0, 4}},
		{2202, {16384, 0, 65536, 4, 0, 4}},

		{2211, {16384, 0, 65536, 16, 0, 16}},
		{2212, {16384, 0, 65536, 16, 0, 16}},

		{2301, {8192, 0, 65536, 8192, 0, 4 * 65536}},
		{2302, {8192, 0, 65536, 8192, 0, 4 * 65536}},
		{2303, {8192, 0, 65536, 8192, 0, 4 * 65536}},
		{2304, {8192, 0, 65536, 8192, 0, 4 * 65536}}};

	this->NumRequiredHighAnodes = 4;
	this->NumRequiredLowAnodes = 4;

	this->hgImage = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, {-10.0, -10.0}, 0.0};
	this->lgImage = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, {-10.0, -10.0}, 0.0};
	this->hgImageQdc = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, {-10.0, -10.0}, 0.0};
	this->lgImageQdc = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, {-10.0, -10.0}, 0.0};
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
		bool hasQDCs = !evt->GetQDCSums().empty();

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
						this->lgImage.xa += evt->GetEnergy();
						++this->lgImage.numanodes;
						if (hasQDCs) {
							this->lgImageQdc.xa += (evt->GetQDCSums()[0] - evt->GetQDCSums()[2]);
							++this->lgImageQdc.numanodes;
						}
					} else {
						++this->AnodeLowHits[0];
					}
				} else {
					if (not this->AnodeHighHits[0]) {
						++this->AnodeHighHits[0];
						this->hgImage.xa += evt->GetEnergy();
						++this->hgImage.numanodes;
						if (hasQDCs) {
							this->hgImageQdc.xa += (evt->GetQDCSums()[0] - evt->GetQDCSums()[2]);
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
						this->lgImage.xb += evt->GetEnergy();
						++this->lgImage.numanodes;
						if (hasQDCs) {
							this->lgImageQdc.xb += (evt->GetQDCSums()[0] - evt->GetQDCSums()[2]);
							++this->lgImageQdc.numanodes;
						}
					} else {
						++this->AnodeLowHits[1];
					}
				} else {
					if (not this->AnodeHighHits[1]) {
						++this->AnodeHighHits[1];
						this->hgImage.xb += evt->GetEnergy();
						++this->hgImage.numanodes;
						if (hasQDCs) {
							this->hgImageQdc.xb += (evt->GetQDCSums()[0] - evt->GetQDCSums()[2]);
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
						this->lgImage.ya += evt->GetEnergy();
						++this->lgImage.numanodes;
						if (hasQDCs) {
							this->lgImageQdc.ya += (evt->GetQDCSums()[0] - evt->GetQDCSums()[2]);
							++this->lgImageQdc.numanodes;
						}
					} else {
						++this->AnodeLowHits[2];
					}
				} else {
					if (not this->AnodeHighHits[2]) {
						++this->AnodeHighHits[2];
						this->hgImage.ya += evt->GetEnergy();
						++this->hgImage.numanodes;
						if (hasQDCs) {
							this->hgImageQdc.ya += (evt->GetQDCSums()[0] - evt->GetQDCSums()[2]);
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
						this->lgImage.yb += evt->GetEnergy();
						++this->lgImage.numanodes;
						if (hasQDCs) {
							this->lgImageQdc.yb += (evt->GetQDCSums()[0] - evt->GetQDCSums()[2]);
							++this->lgImageQdc.numanodes;
						}
					} else {
						++this->AnodeLowHits[3];
					}
				} else {
					if (not this->AnodeHighHits[3]) {
						++this->AnodeHighHits[3];
						this->hgImage.yb += evt->GetEnergy();
						++this->hgImage.numanodes;
						if (hasQDCs) {
							this->hgImageQdc.yb += (evt->GetQDCSums()[0] - evt->GetQDCSums()[2]);
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
						this->lgImage.dynode += evt->GetEnergy();
						this->lgImage.DynodeTimeStamp = evt->GetTimeStamp();
						if (hasQDCs) {
							this->lgImageQdc.dynode += (evt->GetQDCSums()[0] - evt->GetQDCSums()[2]);
							this->lgImageQdc.DynodeTimeStamp = evt->GetTimeStamp();
						}
					} else {
						++this->DynodeLowHits;
					}
				} else {
					if (not this->DynodeHighHits) {
						++this->DynodeHighHits;
						this->hgImage.dynode += evt->GetEnergy();
						this->hgImage.DynodeTimeStamp = evt->GetTimeStamp();
						if (hasQDCs) {
							this->hgImageQdc.dynode += (evt->GetQDCSums()[0] - evt->GetQDCSums()[2]);
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

	this->lgImage.anodesum = this->lgImage.xa + this->lgImage.ya + this->lgImage.xb + this->lgImage.yb;
	hismanager->Fill("PSPMT_2211", this->lgImage.anodesum, this->lgImage.numanodes);
	if (this->lgImage.numanodes == this->NumRequiredLowAnodes) {
		this->CalculatePosition(this->lgImage, 0.0, 0.0, 0.0, false, this->CurrMethod);
		this->CalculatePosition(this->lgImageQdc, 0.0, 0.0, 0.0, false, this->CurrMethod);
		hismanager->Fill("PSPMT_1901", this->lgImage.position.first, this->lgImage.position.second);

		hismanager->Fill("PSPMT_2001", this->lgImage.dynode);

		hismanager->Fill("PSPMT_2101", this->lgImage.dynode, this->lgImage.xa);
		hismanager->Fill("PSPMT_2101", this->lgImage.dynode, this->lgImage.xb);
		hismanager->Fill("PSPMT_2101", this->lgImage.dynode, this->lgImage.ya);
		hismanager->Fill("PSPMT_2101", this->lgImage.dynode, this->lgImage.yb);

		hismanager->Fill("PSPMT_2201", this->lgImage.xa, 0);
		hismanager->Fill("PSPMT_2201", this->lgImage.xb, 1);
		hismanager->Fill("PSPMT_2201", this->lgImage.ya, 2);
		hismanager->Fill("PSPMT_2201", this->lgImage.yb, 3);

		hismanager->Fill("PSPMT_2301", this->lgImage.dynode, this->lgImage.anodesum);

		FillRootStruct(this->lowgain, this->lgImage, this->lgImageQdc);
	}

	this->hgImage.anodesum = this->hgImage.xa + this->hgImage.ya + this->hgImage.xb + this->hgImage.yb;
	hismanager->Fill("PSPMT_2212", this->hgImage.anodesum, this->hgImage.numanodes);
	// this->console->info("num anodes : {}",this->hgImage.numanodes);
	if (this->hgImage.numanodes == this->NumRequiredHighAnodes) {
		this->CalculatePosition(this->hgImage, 0.0, 0.0, 0.0, false, this->CurrMethod);
		this->CalculatePosition(this->hgImageQdc, 0.0, 0.0, 0.0, false, this->CurrMethod);
		hismanager->Fill("PSPMT_1902", this->hgImage.position.first, this->hgImage.position.second);

		hismanager->Fill("PSPMT_2002", this->hgImage.dynode);

		hismanager->Fill("PSPMT_2102", this->hgImage.dynode, this->hgImage.xa);
		hismanager->Fill("PSPMT_2102", this->hgImage.dynode, this->hgImage.xb);
		hismanager->Fill("PSPMT_2102", this->hgImage.dynode, this->hgImage.ya);
		hismanager->Fill("PSPMT_2102", this->hgImage.dynode, this->hgImage.yb);

		hismanager->Fill("PSPMT_2202", this->hgImage.xa, 0);
		hismanager->Fill("PSPMT_2202", this->hgImage.xb, 1);
		hismanager->Fill("PSPMT_2202", this->hgImage.ya, 2);
		hismanager->Fill("PSPMT_2202", this->hgImage.yb, 3);

		hismanager->Fill("PSPMT_2302", this->hgImage.dynode, this->hgImage.anodesum);

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
	}

	pugi::xml_node lg = config.child("LowGain");
	if (lg) {
		this->console->info("Found LowGain tag for PSPMTProcessor");
		this->NumRequiredLowAnodes = lg.attribute("numanodes").as_int(4);
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

	hismanager->RegisterPlot<TH1F>("PSPMT_2001", "Low Gain Dynode; Energy (arb.)", this->h1dsettings.at(2001));
	hismanager->RegisterPlot<TH1F>("PSPMT_2002", "High Gain Dynode; Energy (arb.)", this->h1dsettings.at(2002));

	hismanager->RegisterPlot<TH2F>("PSPMT_2101", "Low Gain Anodes vs Low Gain Dynode; Energy (arb.)", this->h2dsettings.at(2101));
	hismanager->RegisterPlot<TH2F>("PSPMT_2102", "High Gain Anodes vs High Gain Dynode; Energy (arb.)", this->h2dsettings.at(2102));

	hismanager->RegisterPlot<TH2F>("PSPMT_2201", "Inividual Low Gain Anode; Energy (arb.); Position (arb.)", this->h2dsettings.at(2201));
	hismanager->RegisterPlot<TH2F>("PSPMT_2202", "Inividual High Gain Anode; Energy (arb.); Position (arb.)", this->h2dsettings.at(2201));

	hismanager->RegisterPlot<TH2F>("PSPMT_2211", "Low Gain Num Anodes vs AnodeSum; Energy (arb.); Mult. (arb.)", this->h2dsettings.at(2211));
	hismanager->RegisterPlot<TH2F>("PSPMT_2212", "High Gain Num Anodes vs AnodeSum; Energy (arb.); Mult. (arb.)", this->h2dsettings.at(2212));

	hismanager->RegisterPlot<TH2F>("PSPMT_2301", "Low Gain Anodesum vs Low Gain Dynode; Energy (arb.)", this->h2dsettings.at(2301));
	hismanager->RegisterPlot<TH2F>("PSPMT_2302", "High Gain Anodesum vs High Gain Dynode; Energy (arb.)", this->h2dsettings.at(2302));
	hismanager->RegisterPlot<TH2F>("PSPMT_2303", "Low Gain QDC::Anodesum vs Low Gain QDC::Dynode; Energy (arb.)", this->h2dsettings.at(2303));
	hismanager->RegisterPlot<TH2F>("PSPMT_2304", "High Gain QDC::Anodesum vs High Gain QDC::Dynode; Energy (arb.)", this->h2dsettings.at(2304));
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

	this->lgImage.ResetDynode();
	this->lgImage.ResetAnode();
	this->lgImage.ResetPosition(-10.0, -10.0);
	this->lgImage.ResetCorners();

	this->hgImageQdc.ResetDynode();
	this->hgImageQdc.ResetAnode();
	this->hgImageQdc.ResetPosition(-10.0, -10.0);
	this->hgImageQdc.ResetCorners();

	this->lgImageQdc.ResetDynode();
	this->lgImageQdc.ResetAnode();
	this->lgImageQdc.ResetPosition(-10.0, -10.0);
	this->lgImageQdc.ResetCorners();

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
			x = (img.ya + img.xb) / img.anodesum;
			y = (img.xa + img.xb) / img.anodesum;
		} else {
			x = (img.yb + img.xa) / img.anodesum;
			y = (img.xa + img.xb) / img.anodesum;
		}

		x -= xcenter;
		y -= ycenter;
	} else if (method == PSPMTProcessor::IMAGEMETHOD::SIDES) {
		if (xflip) {
			x = (img.xa - img.xb) / (img.xa + img.xb);
			y = (img.ya - img.yb) / (img.ya + img.yb);
		} else {
			x = (img.xb - img.xa) / (img.xa + img.xb);
			y = (img.yb - img.ya) / (img.ya + img.yb);
		}
	}

	img.position = {x * std::cos(rotation) + xcenter - y * std::sin(rotation) + ycenter, x * std::sin(rotation) + xcenter + y * std::cos(rotation) + ycenter};
}

void PSPMTProcessor::FillRootStruct(ProcessorStruct::PSPMT& Rstruct, const PSPMT::Image& enImage, const PSPMT::Image& qdcImage) {
	Rstruct.xpos = enImage.position.first;
	Rstruct.ypos = enImage.position.second;
	Rstruct.xposqdc = qdcImage.position.first;
	Rstruct.yposqdc = qdcImage.position.second;
	Rstruct.dynodeen = enImage.dynode;
	Rstruct.dynodeqdc = qdcImage.dynode;
	Rstruct.dynodets = enImage.DynodeTimeStamp;
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
