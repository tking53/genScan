#ifndef __GENERIC_PROCESSOR_HPP__
#define __GENERIC_PROCESSOR_HPP__

#include <string>

#include "HistogramManager.hpp"
#include "Processor.hpp"

class GenericProcessor : public Processor{
	public:
		GenericProcessor(const std::string&);
		bool PreProcess() final;
		bool Process() final;
		bool PostProcess() final;

		void Init(const YAML::Node&);
		void Init(const Json::Value&);
		void Init(const pugi::xml_node&);

		void DeclarePlots(PLOTS::PlotRegistry*) const;
};

#endif
