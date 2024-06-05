#ifndef __E21069B_PROCESSOR_HPP__
#define __E21069B_PROCESSOR_HPP__

#include <memory>
#include <string>

#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "MtasImplantProcessor.hpp"
#include "MtasProcessor.hpp"
#include "PidProcessor.hpp"
#include "Processor.hpp"

class e21069bProcessor : public Processor{
	public:
		e21069bProcessor(const std::string&);
		[[maybe_unused]] bool PreProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] bool Process(EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] bool PostProcess([[maybe_unused]] EventSummary&,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;

		virtual void Finalize() final;

		void Init(const YAML::Node&);
		void Init(const Json::Value&);
		void Init(const pugi::xml_node&);

		void DeclarePlots(PLOTS::PlotRegistry*) const;
		virtual TTree* RegisterTree() final;
		virtual void CleanupTree() final;
	private:

		std::unique_ptr<MtasProcessor> MtasProc;
		std::unique_ptr<PidProcessor> PidProc;
		std::unique_ptr<MtasImplantProcessor> MtasImplantProc;
		
};

#endif
