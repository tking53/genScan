#ifndef __E23064_PROCESSOR_HPP__
#define __E23064_PROCESSOR_HPP__

#include "CloverProcessor.hpp"
#include "DSSDProcessor.hpp"
#include "PidProcessor.hpp"
#include "VetoProcessor.hpp"
#include "Processor.hpp"

#include "e23064Struct.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class e23064Processor : public Processor {
public:
	e23064Processor(const std::string&);
	virtual ~e23064Processor() = default;

	[[maybe_unused]] virtual bool PreProcess(EventHistoryManager*, PLOTS::PlotRegistry*, CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool Process(EventHistoryManager*, PLOTS::PlotRegistry*, CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool PostProcess(EventHistoryManager*, PLOTS::PlotRegistry*, CUTS::CutRegistry*) final;

	virtual void Finalize() final;
	virtual void Init(const pugi::xml_node&);
	virtual void DeclarePlots(PLOTS::PlotRegistry*);
	virtual void RegisterTree(std::unordered_map<std::string, TTree*>&) final;
	virtual void CleanupTree() final;
	virtual void RegisterCuts(CUTS::CutRegistry*) final;

private:
	void Reset();
	bool IsWithinPidGate(const EventSummary*, const std::string&, CUTS::CutRegistry*) const;
	double GetDSSDLowGainForIndexFromSummary(const EventSummary*, int) const;
	double ComputePinEnergySum(bool& valid) const;

	ProcessorStruct::EXP EXPData;

	bool ConfigHasDSSD;
	bool ConfigHasPid;
	bool ConfigHasClover;
    bool ConfigHasVeto;

	bool EventHasDSSD;
	bool EventHasPid;
	bool EventHasClover;
	bool EventHasVeto;
    
	int PIDPlot;
	int ImplantDSSDIndex;
	double DSSDThreshold;
	double PinThreshold;
	bool RequireAllPins;

	std::vector<std::pair<std::string, std::string>> IsotopeCuts;
	std::vector<std::string> IsotopeTags;

	std::shared_ptr<DSSDProcessor> DSSDProc;
	std::shared_ptr<PidProcessor> PidProc;
	std::shared_ptr<CloverProcessor> CloverProc;
	std::shared_ptr<VetoProcessor> VetoProc;
};

#endif
