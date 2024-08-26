#ifndef __MTASSSD_PROCESSOR_HPP__
#define __MTASSSD_PROCESSOR_HPP__

#include "PhysicsData.hpp"
#include "Processor.hpp"

class MtasSSDProcessor : public Processor{
	public:
		MtasSSDProcessor(const std::string&);
		virtual ~MtasSSDProcessor() = default;
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

		void Reset();

		PhysicsData* MaxEvent;
};

#endif
