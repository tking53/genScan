#ifndef __RIBF168_PROCESSOR_HPP__
#define __RIBF168_PROCESSOR_HPP__

#include <memory>
#include <string>
#include <map>

#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "HagridProcessor.hpp"
#include "RIKENIonizationChamberProcessor.hpp"
#include "RIKENPidProcessor.hpp"
#include "PSPMTProcessor.hpp"
#include "Processor.hpp"
#include "VetoProcessor.hpp"
#include "boost/container/devector.hpp"

class ribf168Processor : public Processor{
	public:
		ribf168Processor(const std::string&);
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

		struct PID{
			double Z;
			double AoQ;
			double TimeStamp;
		};

		//struct Implant{
		//	double TimeStamp;
		//	PSPMTProcessor::EventInfo PSPMT;
		//	HagridProcessor::EventInfo Hagrid;
		//};

	private:
		bool HasHagrid;
		bool HasRIKENIonChamber;
		bool HasRIKENPid;
		bool HasPSPMT;
		bool HasVeto;

		double PreImplantTimeNs;
		double PostImplantTimeNs;
		double TotalImplantTimeNs;

		RIKENIonizationChamberProcessor::EventInfo CurrIonChamber;
		PSPMTProcessor::EventInfo CurrPSPMT;
		HagridProcessor::EventInfo CurrHagrid;
		RIKENPidProcessor::EventInfo CurrPid;
		VetoProcessor::EventInfo CurrVeto;

		boost::container::devector<PID> Ions;

		std::unique_ptr<HagridProcessor> HagridProc;
		std::unique_ptr<RIKENIonizationChamberProcessor> RIKENIonizationChamberProc;
		std::unique_ptr<RIKENPidProcessor> RIKENPidProc;
		std::unique_ptr<PSPMTProcessor> PSPMTProc;
		std::unique_ptr<VetoProcessor> VetoProc;
};

#endif
