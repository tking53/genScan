#ifndef __MTAS_ISOMER_PROCESSOR_HPP__
#define __MTAS_ISOMER_PROCESSOR_HPP__

#include "Gates.hpp"
#include "MtasProcessor.hpp"
#include "MtasSSDProcessor.hpp"
#include "PSPMTProcessor.hpp"
#include "Processor.hpp"
#include "SimpleHPGeProcessor.hpp"
#include <memory>
#include <vector>

class MtasIsomerProcessor : public Processor {
public:
	MtasIsomerProcessor(const std::string&, Processor*);
	virtual ~MtasIsomerProcessor() = default;

	void FillMtasPlots(EventHistoryManager*, PLOTS::PlotRegistry*, CUTS::CutRegistry*, MtasProcessor*, MtasSSDProcessor*);
	void FillImplantPlots(EventHistoryManager*, PLOTS::PlotRegistry*, CUTS::CutRegistry*, SimpleHPGeProcessor*, PSPMTProcessor*);
	void SetInternalTDiff(double);

	virtual void Finalize() final;

	virtual void Init(const pugi::xml_node&);

	virtual void DeclarePlots(PLOTS::PlotRegistry*);

private:
	Processor* ParentProc;

	std::vector<BoxGate<double>> ISOMER_3701_Gates;

	double SiliconThreshold;
	double ImplantThreshold;
	double HPGeThreshold;

	std::string implant;
	std::string hpge;
	std::string beta;
	std::string gamma;
	std::string muon;

	double internaltdiff;
};

#endif
