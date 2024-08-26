#ifndef __MTASTAPE_PROCESSOR_HPP__
#define __MTASTAPE_PROCESSOR_HPP__

#include "Processor.hpp"

class MtasTapeProcessor : public Processor{
	public:
		MtasTapeProcessor(const std::string&);
		virtual ~MtasTapeProcessor() = default;
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

		enum TapeCycleState{
			UNKNOWN,
			TAPEMOVE,
			MEASURE,
			BACKGROUND,
			LIGHTPULSER,
			IRRADIATION
		};

		unsigned int GetCurrentCycleNumber() const;
		TapeCycleState GetCurrentCycleState() const;
		void IncrementCycleNumber();

	private:

		void Reset();

		unsigned int CycleCount;
		TapeCycleState CurrState;
};

#endif
