#ifndef __IONIZATION_CHAMBER_PROCESSOR_HPP__
#define __IONIZATION_CHAMBER_PROCESSOR_HPP__

#include <string>

#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "Processor.hpp"

class IonizationChamberProcessor : public Processor{
	public:
		IonizationChamberProcessor(const std::string&);
		[[maybe_unused]] bool PreProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] bool Process(EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] bool PostProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;

		virtual void Finalize() final;

		void Init(const YAML::Node&);
		void Init(const Json::Value&);
		void Init(const pugi::xml_node&);

		void DeclarePlots(PLOTS::PlotRegistry*) const;
		virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>&) final;
		virtual void CleanupTree() final;

		struct EventInfo{
			std::vector<double> AnodeEnergy;
			std::vector<double> AnodeTimeStamp;
			double TotalAnodeEnergy;
			double MaxAnodeEnergy;
			std::vector<double> CathodeEnergy;
			std::vector<double> CathodeTimeStamp;
			double TotalCathodeEnergy;
			double MaxCathodeEnergy;
			double FirstPSD;
			double TotalPSD;
			double MaxPSD;
			double FirstTimeStamp;
			double FinalTimeStamp;
			bool Saturate;
			bool Pileup;
			bool RealEvent;
		};

		EventInfo& GetCurrEvt();
		EventInfo& GetPrevEvt();

	private:
		enum SUBTYPE{
			ANODE,
			CATHODE,
			UNKNOWN
		};

		void Reset();
		void InitHelpers();

		EventInfo NewEvt;
		EventInfo CurrEvt;
		EventInfo PrevEvt;

		SUBTYPE currsubtype;

		std::vector<int> AnodeHits;
		std::vector<int> CathodeHits;

		bool firstevt;

		int NumAnode;
		int NumCathode;
};

#endif
