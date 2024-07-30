#ifndef __BSMEXP_PROCESSOR_HPP__
#define __BSMEXP_PROCESSOR_HPP__

#include "BSMProcessor.hpp"
#include "MtasProcessor.hpp"
#include "Processor.hpp"

class BSMExpProcessor : public Processor{
	public:
		BSMExpProcessor(const std::string&);
		virtual ~BSMExpProcessor() = default;
		[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool Process(EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;

		virtual void Finalize() final;

		virtual void Init(const YAML::Node&);
		virtual void Init(const Json::Value&);
		virtual void Init(const pugi::xml_node&);

		virtual void DeclarePlots(PLOTS::PlotRegistry*) const;
		virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>&) final;
		virtual void CleanupTree() final;
	private:

		MtasProcessor::EventInfo CurrMTAS;
		BSMProcessor::EventInfo CurrBSM;

		bool HasMTAS;
		bool HasBSM;

		double BetaThreshold;
		double QBeta;

		bool PPCutExists;

		std::unique_ptr<BSMProcessor> BSMProc;
		std::unique_ptr<MtasProcessor> MtasProc;
};

#endif
