#ifndef __EBSS_PROCESSOR_HPP__
#define __EBSS_PROCESSOR_HPP__

#include "EBSSStruct.hpp"
#include "Processor.hpp"
#include <vector>

class EBSSProcessor : public Processor {
public:
	EBSSProcessor(const std::string&);
	virtual ~EBSSProcessor() = default;

	[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool Process([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;

	virtual void Finalize() final;
	virtual void Init(const pugi::xml_node&);
	virtual void DeclarePlots(PLOTS::PlotRegistry*);
	virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string, TTree*>&) final;
	virtual void CleanupTree() final;

private:
	void Reset();

	const int NumPaddles = 16;
	const int firstPaddleGroup = 3; // cables 0-2 are at ANL for the BSM, so we start from 3 to match the real labels on cables and the tubes

	double TotalEventEnergy = 0.0;
	double TotalEventEnergyDirty = 0.0;

	std::vector<ProcessorStruct::EBSSPaddle> PaddleData;
	ProcessorStruct::EBSStotal TotalData;

	TTree* OutputTree;
};

#endif
