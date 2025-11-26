#ifndef __ANL2021_PROCESSOR_HPP__
#define __ANL2021_PROCESSOR_HPP__

#include "Processor.hpp"
#include "MtasProcessor.hpp"
#include "MtasSSDProcessor.hpp"
#include "MtasTapeProcessor.hpp"
#include "SimpleHPGeProcessor.hpp"
#include "PSPMTProcessor.hpp"

#include "Gates.hpp"
#include <vector>

class anl2021Processor : public Processor{
	public:
		anl2021Processor(const std::string&);
		virtual ~anl2021Processor() = default;
		[[maybe_unused]] virtual bool PreProcess(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool Process(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool PostProcess(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*) final;

		virtual void Finalize() final;

		virtual void Init(const pugi::xml_node&);

		virtual void DeclarePlots(PLOTS::PlotRegistry*);
		virtual void RegisterTree(std::unordered_map<std::string,TTree*>&) final;
		virtual void CleanupTree() final;
	private:
		void Reset();
		bool HasMTAS;
		bool HasSilicon;
		bool HasTape;
		bool HasHPGe;
		bool HasPSPMT;

		double SiliconThreshold;
		double ImplantThreshold;
		double HPGeThreshold;

		Gate<double> EarlyCycle;
		Gate<double> MidCycle;
		Gate<double> LateCycle;
		
		std::vector<BoxGate<double>> ISOMER_3701_Gates;

		std::string implant;
		std::string hpge;
		std::string beta;
		std::string gamma;
		std::string muon;
		std::string tapemove;
		std::string measure;
		std::string background;
		std::string irradiation;
		std::string lightpulser;
		std::string unknown;

		std::shared_ptr<MtasSSDProcessor> SiliconProc;
		std::shared_ptr<MtasTapeProcessor> TapeProc;
		std::shared_ptr<MtasProcessor> MtasProc;
		std::shared_ptr<SimpleHPGeProcessor> HPGeProc;
		std::shared_ptr<PSPMTProcessor> ImplantProc;
};

#endif
