#ifndef __BSM_PROCESSOR_HPP__
#define __BSM_PROCESSOR_HPP__

#include <string>
#include <unordered_map>

#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "Processor.hpp"

class BSMProcessor : public Processor{
	public:
		BSMProcessor(const std::string&);
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
};

#endif