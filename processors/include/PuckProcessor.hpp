#ifndef __PUCK_PROCESSOR_HPP__
#define __PUCK_PROCESSOR_HPP__

#include "Processor.hpp"

class PuckProcessor : public Processor{
	public:
		PuckProcessor(const std::string&);
		virtual ~PuckProcessor() = default;
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

		struct EventInfo{
			double TotalEnergy;
			std::vector<double> Individual;
			double FirstTime;
			double LastTime;
			bool Saturate;
			bool Pileup;
			bool RealEvt;

			EventInfo(){
				TotalEnergy = 0.0;
				Individual = std::vector<double>(2,0.0);
				FirstTime = -1.0;
				LastTime = -1.0;
				Saturate = false;
				Pileup = false;
				RealEvt = false;
			}
			~EventInfo() = default;
			EventInfo(const EventInfo&) = default;
			EventInfo(EventInfo&&) = default;
			EventInfo& operator=(const EventInfo&) = default;
			EventInfo& operator=(EventInfo&&) = default;
		};

		EventInfo& GetCurrEvt();
		EventInfo& GetPrevEvt();

	private:
		void Reset();

		EventInfo CurrEvt;
		EventInfo PrevEvt;
		EventInfo NewEvt;

		std::vector<double> Puck;
		std::vector<int> PuckHits;

		std::vector<double> TimeStamps;

		std::string fronttag;
		std::string backtag;

		bool foundfirstevt;
		double globalfirsttime;
};

#endif
