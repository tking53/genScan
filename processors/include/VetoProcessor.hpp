#ifndef __VETO_PROCESSOR_HPP__
#define __VETO_PROCESSOR_HPP__

#include "Gates.hpp"
#include "Processor.hpp"
#include "VetoStruct.hpp"
#include <vector>

class VetoProcessor : public Processor {
public:
	VetoProcessor(const std::string&);
	virtual ~VetoProcessor() = default;
	[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool Process(EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;

	virtual void Finalize() final;

	virtual void Init(const pugi::xml_node&);

	virtual void DeclarePlots(PLOTS::PlotRegistry*);
	virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string, TTree*>&) final;
	virtual void CleanupTree() final;

	const double& GetRIT() const;
	const double& GetFIT() const;

private:
	void Reset();

	enum SUBTYPE {
		FIT,
		RIT,
		UNKNOWN
	};

	SUBTYPE currsubtype;
	double rit;
	double rit_psd;
	double fit;
	double fit_psd;

	std::vector<Gate<double>> FitReject;
	std::vector<Gate<double>> RitReject;

	ProcessorStruct::Veto fit_root;
	ProcessorStruct::Veto rit_root;
};

#endif
