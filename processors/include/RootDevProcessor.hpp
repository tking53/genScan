#ifndef __ROOTDEV_PROCESSOR_HPP__
#define __ROOTDEV_PROCESSOR_HPP__

#include <string>

#include "PhysicsData.hpp"
#include "RootDevStruct.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "Processor.hpp"

class RootDevProcessor : public Processor{
	public:
		RootDevProcessor(const std::string&);
		[[maybe_unused]] bool PreProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*) final;
		[[maybe_unused]] bool Process([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*) final;
		[[maybe_unused]] bool PostProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*) final;

		virtual void Finalize() final;

		void Init(const YAML::Node&);
		void Init(const Json::Value&);
		void Init(const pugi::xml_node&);

		void DeclarePlots(PLOTS::PlotRegistry*) const;
		virtual TTree* RegisterTree() final;
		virtual void CleanupTree() final;
	private:
		std::vector<ProcessorStruct::RootDev> DataVec;
		ProcessorStruct::RootDev CurrData;
		std::vector<PhysicsData*> SummaryData;
};

#endif
