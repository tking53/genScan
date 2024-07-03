#ifndef __HAGRID_PROCESSOR_HPP__
#define __HAGRID_PROCESSOR_HPP__

#include "Processor.hpp"

class HagridProcessor : public Processor{
	public:
		HagridProcessor(const std::string&);
		virtual ~HagridProcessor() = default;
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

		void SetIsBeta();
		void SetIsIon();

		struct EventInfo{
			std::vector<double> Energy;
			std::vector<double> TimeStamp;
			double TotalEnergy;
			double FirstTimeStamp;
			double FinalTimeStamp;
			bool BetaTriggered;
			bool IonTriggered;
			bool Saturate;
			bool Pileup;
			bool RealEvent;
		};

		EventInfo& GetCurrEvt();
		EventInfo& GetPrevEvt();

	private:
		void Reset();
		void InitHelpers();

		EventInfo NewEvt;
		EventInfo CurrEvt;
		EventInfo PrevEvt;

		std::vector<int> Hits;
		std::vector<int> ZeroHits;
		std::vector<double> TimeStamps;
		unsigned int NumHagrid;

		std::string upstreamtag;
		std::string downstreamtag;

		double FirstEvtTime;
		bool FoundFirstEvt;
};

#endif
