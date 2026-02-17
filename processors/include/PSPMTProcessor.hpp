#ifndef __PSPMT_PROCESSOR_HPP__
#define __PSPMT_PROCESSOR_HPP__

#include <vector>

#include "Processor.hpp"

#include "ImageManipulation.hpp"
#include "PSPMTStruct.hpp"

class PSPMTProcessor : public Processor {
public:
	PSPMTProcessor(const std::string&);
	virtual ~PSPMTProcessor() = default;
	[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool Process(EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;

	virtual void Finalize() final;

	virtual void Init(const pugi::xml_node&);

	virtual void DeclarePlots(PLOTS::PlotRegistry*);
	virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string, TTree*>&) final;
	virtual void CleanupTree() final;

	const PSPMT::Image& GetLowGainImage() const;
	const PSPMT::Image& GetHighGainImage() const;

	const PSPMT::Image& GetLowGainImageQdc() const;
	const PSPMT::Image& GetHighGainImageQdc() const;

private:
	enum IMAGEMETHOD {
		CORNERS,
		SIDES
	};

	void Reset();
	void CalculatePosition(PSPMT::Image&, double, double, double, bool, PSPMTProcessor::IMAGEMETHOD&);
	void FillRootStruct(ProcessorStruct::PSPMT&, const PSPMT::Image&, const PSPMT::Image&);

	int NumRequiredHighAnodes;
	int NumRequiredLowAnodes;

	int AmpDynodeHits;
	int DynodeLowHits;
	std::vector<int> AnodeLowHits;

	int DynodeHighHits;
	std::vector<int> AnodeHighHits;

	std::string highgaintag;
	std::string lowgaintag;

	PSPMT::Image hgImage;
	PSPMT::Image lgImage;
	PSPMT::Image hgImageQdc;
	PSPMT::Image lgImageQdc;
	PSPMTProcessor::IMAGEMETHOD CurrMethod;
	double ampdynode;

	ProcessorStruct::PSPMT highgain;
	ProcessorStruct::PSPMT lowgain;
};

#endif
