#ifndef __ROOTDEV_PROCESSOR_HPP__
#define __ROOTDEV_PROCESSOR_HPP__

#include "RootDevStruct.hpp"
#include "Processor.hpp"

class RootDevProcessor : public Processor{
	public:
		RootDevProcessor(const std::string&);
		virtual ~RootDevProcessor() = default;
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
		void InsertAdditionalTypes(const std::string&);

		std::vector<ProcessorStruct::RootDev> DataVec;
		ProcessorStruct::RootDev CurrData;
};

#endif
