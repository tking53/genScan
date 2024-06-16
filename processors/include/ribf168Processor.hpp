#ifndef __RIBF168_PROCESSOR_HPP__
#define __RIBF168_PROCESSOR_HPP__

#include <memory>
#include <string>

#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "HagridProcessor.hpp"
#include "IonizationChamberProcessor.hpp"
#include "PidProcessor.hpp"
#include "PSPMTProcessor.hpp"
#include "Processor.hpp"

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
	private:
		bool HasHagrid;
		bool HasIonChamber;
		bool HasPid;
		bool HasPSPMT;


		std::unique_ptr<HagridProcessor> HagridProc;
		std::unique_ptr<IonizationChamberProcessor> IonizationChamberProc;
		std::unique_ptr<PidProcessor> PidProc;
		std::unique_ptr<PSPMTProcessor> PSPMTProc;
		
};

#endif
