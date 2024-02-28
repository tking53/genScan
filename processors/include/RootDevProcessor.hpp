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
		[[maybe_unused]] bool PreProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] bool Process(EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] bool PostProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;

		virtual void Finalize() final;

		void Init(const YAML::Node&);
		void Init(const Json::Value&);
		void Init(const pugi::xml_node&);

		void DeclarePlots(PLOTS::PlotRegistry*) const;
		virtual TTree* RegisterTree() final;
		virtual void CleanupTree() final;
	private:
		void InsertAdditionalTypes(const std::string&);

		std::vector<ProcessorStruct::RootDev> DataVec;
		ProcessorStruct::RootDev CurrData;
		std::vector<PhysicsData*> SummaryData;
};

#endif
