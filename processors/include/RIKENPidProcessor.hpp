#ifndef __RIKEN_PID_PROCESSOR_HPP__
#define __RIKEN_PID_PROCESSOR_HPP__

#include <string>

#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "Processor.hpp"

class RIKENPidProcessor : public Processor{
	public:
		RIKENPidProcessor(const std::string&);
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
			double F7Analog;
			double F7AnalogTimeStamp;
			double F7Logic;
			double F7LogicTimeStamp;
			std::vector<double> F11LeftRight;
			std::vector<double> F11LeftRightTimeStamp;
			std::vector<double> tdiff;
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
			F7,
			F11,
			UNKNOWN
		};

		SUBTYPE currsubtype;

		std::string analogtag;
		std::string logictag;
		std::string lefttag;
		std::string righttag;
};

#endif
