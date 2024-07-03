#ifndef __GENERIC_ANALYZER_HPP__
#define __GENERIC_ANALYZER_HPP__

#include "Analyzer.hpp"

class GenericAnalyzer : public Analyzer{
	public:
		GenericAnalyzer(const std::string&);
		virtual ~GenericAnalyzer() = default;
		[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool Process([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;

		virtual void Init(const YAML::Node&);
		virtual void Init(const Json::Value&);
		virtual void Init(const pugi::xml_node&);

		virtual void Finalize() final;

		virtual void DeclarePlots(PLOTS::PlotRegistry*) const;
};

#endif
