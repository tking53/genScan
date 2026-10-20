#ifndef __DEUTERATED_LIQUID_SCINTILLATOR_PROCESSOR_HPP__
#define __DEUTERATED_LIQUID_SCINTILLATOR_PROCESSOR_HPP__

#include "Processor.hpp"

class DeuteratedLiquidScintillatorProcessor : public Processor {
public:
	DeuteratedLiquidScintillatorProcessor(const std::string&);
	virtual ~DeuteratedLiquidScintillatorProcessor() = default;
	[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool Process(EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;

	virtual void Finalize() final;

	virtual void Init(const pugi::xml_node&);

	virtual void DeclarePlots(PLOTS::PlotRegistry*);
	virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string, TTree*>&) final;
	virtual void CleanupTree() final;

	double GetEnergy(int) const;
	double GetPSD(int) const;
	double GetScintillatorFireTime(int) const;
	double GetTraceMax(int) const;
	bool DidScintillatorPileup(int) const;
	bool DidScintillatorSaturate(int) const;
	int GetNumScintillators() const;

private:
	struct DLSData {
		double psd = -999.0;
		double energy = 0.0;
		double ts = -999.0;
		double tmax = -1.0;
		bool saturate = false;
		bool pileup = false;
	};

	void Reset();

	std::vector<DLSData> data;
	int NumScintillator;
	size_t psd_idx;
};

#endif
