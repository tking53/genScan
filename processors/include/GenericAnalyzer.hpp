#ifndef __GENERIC_ANALYZER_HPP__
#define __GENERIC_ANALYZER_HPP__

#include "Analyzer.hpp"

class GenericAnalyzer : public Analyzer{
	public:
		GenericAnalyzer(const std::string&);
		[[maybe_unused]] bool PreProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*) final;
		[[maybe_unused]] bool Process([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*) final;
		[[maybe_unused]] bool PostProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*) final;

		void Init(const YAML::Node&);
		void Init(const Json::Value&);
		void Init(const pugi::xml_node&);

		virtual void Finalize() final;

		void DeclarePlots(PLOTS::PlotRegistry*) const;
};

#endif
