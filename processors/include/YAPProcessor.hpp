#ifndef __YAP_PROCESSOR_HPP__
#define __YAP_PROCESSOR_HPP__

#include "PuckProcessor.hpp"
#include "MtasProcessor.hpp"
#include "Processor.hpp"

class YAPProcessor : public Processor{
	public:
		YAPProcessor(const std::string&);
		virtual ~YAPProcessor() = default;
		[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool Process(EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;

		virtual void Finalize() final;

		virtual void Init(const pugi::xml_node&);

		virtual void DeclarePlots(PLOTS::PlotRegistry*);
		virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>&) final;
		virtual void CleanupTree() final;
	private:

		PuckProcessor::EventInfo CurrPuck;

		bool HasMTAS;
		bool HasPuck;

		double BetaThreshold;

		std::unique_ptr<PuckProcessor> PuckProc;
		std::unique_ptr<MtasProcessor> MtasProc;

		double MTASTotal;
		bool MTASPileup;
		bool MTASSaturate;
		double YAPFront;
		double YAPBack;
		bool YAPPileup;
		bool YAPSaturate;
};

#endif
