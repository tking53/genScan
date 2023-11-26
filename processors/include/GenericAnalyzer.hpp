#ifndef __GENERIC_ANALYZER_HPP__
#define __GENERIC_ANALYZER_HPP__

#include "Analyzer.hpp"

class GenericAnalyzer : public Analyzer{
	public:
		GenericAnalyzer(const std::string&);
		bool PreProcess() final;
		bool Process() final;
		bool PostProcess() final;

		void Init(const YAML::Node&);
		void Init(const Json::Value&);
		void Init(const pugi::xml_node&);

		void DeclarePlots(PLOTS::PlotRegistry*) const;
};

#endif
