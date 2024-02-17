#ifndef __GENERIC_PROCESSOR_HPP__
#define __GENERIC_PROCESSOR_HPP__

#include <string>

#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "Processor.hpp"

class GenericProcessor : public Processor{
	public:
		GenericProcessor(const std::string&);
		[[maybe_unused]] bool PreProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*) final;
		[[maybe_unused]] bool Process([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*) final;
		[[maybe_unused]] bool PostProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*) final;

		virtual void Finalize() final;

		void Init(const YAML::Node&);
		void Init(const Json::Value&);
		void Init(const pugi::xml_node&);

		void DeclarePlots(PLOTS::PlotRegistry*) const;
};

#endif
