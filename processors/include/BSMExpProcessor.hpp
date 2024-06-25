#ifndef __BSMEXP_PROCESSOR_HPP__
#define __BSMEXP_PROCESSOR_HPP__

#include <memory>
#include <string>

#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "BSMProcessor.hpp"
#include "MtasProcessor.hpp"
#include "Processor.hpp"

class BSMExpProcessor : public Processor{
	public:
		BSMExpProcessor(const std::string&);
		[[maybe_unused]] bool PreProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] bool Process(EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] bool PostProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;

		virtual void Finalize() final;

		void Init(const YAML::Node&);
		void Init(const Json::Value&);
		void Init(const pugi::xml_node&);

		void DeclarePlots(PLOTS::PlotRegistry*) const;
		virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>&) final;
		virtual void CleanupTree() final;
	private:

		MtasProcessor::EventInfo CurrMTAS;
		BSMProcessor::EventInfo CurrBSM;

		bool HasMTAS;
		bool HasBSM;

		double BetaThreshold;

		std::unique_ptr<BSMProcessor> BSMProc;
		std::unique_ptr<MtasProcessor> MtasProc;
};

#endif
