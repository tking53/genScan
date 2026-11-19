#ifndef __KCL_COMPTON_PROCESSOR_HPP__
#define __KCL_COMPTON_PROCESSOR_HPP__

#include "BSMProcessor.hpp"
#include "SimpleHPGeProcessor.hpp"
#include "Processor.hpp"

class KClComptonProcessor : public Processor {
public:
	KClComptonProcessor(const std::string&);
	virtual ~KClComptonProcessor() = default;
	[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool Process(EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;

	virtual void Finalize() final;

	virtual void Init(const pugi::xml_node&);

	virtual void DeclarePlots(PLOTS::PlotRegistry*);
	virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string, TTree*>&) final;
	virtual void CleanupTree() final;

private:
	bool HasHPGe;
	bool HasBSM;

	std::unique_ptr<BSMProcessor> BSMProc;
	std::unique_ptr<SimpleHPGeProcessor> HPGeProc;
};

#endif
