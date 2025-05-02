#ifndef __E21027_PROCESSOR_HPP__
#define __E21027_PROCESSOR_HPP__

#include "Processor.hpp"
#include "MtasProcessor.hpp"
#include "MtasImplantProcessor.hpp"
#include "PidProcessor.hpp"

class e21027Processor : public Processor{
	public:
		e21027Processor(const std::string&);
		virtual ~e21027Processor() = default;
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
		bool HasSIPM;
		bool HasPID;

		double ImplantThreshold;

		std::string implant;
		std::string beta;
		std::string gamma;
		std::string unknown;

		std::shared_ptr<MtasProcessor> MtasProc;
		std::shared_ptr<MtasImplantProcessor> ImplantProc;
		std::shared_ptr<PidProcessor> PidProc;
};

#endif
