#ifndef __SIMPLE_HPGE_PROCESSOR_HPP__
#define __SIMPLE_HPGE_PROCESSOR_HPP__

#include "Processor.hpp"

class SimpleHPGeProcessor : public Processor {
public:
	SimpleHPGeProcessor(const std::string&);
	virtual ~SimpleHPGeProcessor() = default;
	[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool Process(EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;

	virtual void Finalize() final;

	virtual void Init(const pugi::xml_node&);

	virtual void DeclarePlots(PLOTS::PlotRegistry*);
	virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string, TTree*>&) final;
	virtual void CleanupTree() final;

	double GetEnergy(int) const;
	double GetCrystalFireTime(int) const;
	bool DidCrystalPileup(int) const;
	bool DidCrystalSaturate(int) const;
	int GetNumCrystals() const;

private:
	void Reset();

	std::vector<double> Energies;
	std::vector<double> TS;
	std::vector<bool> Saturate;
	std::vector<bool> Pileup;
	int NumHPGe;
};

#endif
