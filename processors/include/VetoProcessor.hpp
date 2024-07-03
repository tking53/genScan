#ifndef __VETO_PROCESSOR_HPP__
#define __VETO_PROCESSOR_HPP__

#include "Processor.hpp"

class VetoProcessor : public Processor{
	public:
		VetoProcessor(const std::string&);
		virtual ~VetoProcessor() = default;
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
			std::vector<double> FrontErg;
			std::vector<double> FrontTimeStamp;
			std::vector<double> FrontCFDTimeStamp;
			std::vector<double> RearErg;
			std::vector<double> RearTimeStamp;
			std::vector<double> RearCFDTimeStamp;
			double MaxFrontErg;
			double MaxFrontTimeStamp;
			double MaxFrontCFDTimeStamp;
			double MaxRearErg;
			double MaxRearTimeStamp;
			double MaxRearCFDTimeStamp;
			bool Pileup;
			bool Saturate;
			bool RealEvent;
		};

		EventInfo& GetCurrEvt();
		EventInfo& GetPrevEvt();

	private:
		void Reset();

		EventInfo NewEvt;
		EventInfo CurrEvt;
		EventInfo PrevEvt;

		enum SUBTYPE{
			FIT,
			RIT,
			UNKNOWN 
		};

		SUBTYPE currsubtype;
		std::vector<std::tuple<double,double,double>> HighestFit;
		std::vector<std::tuple<double,double,double>> HighestRit;
};

#endif
