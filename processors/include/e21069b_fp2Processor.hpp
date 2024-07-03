#ifndef __E21069B_PROCESSOR_HPP__
#define __E21069B_PROCESSOR_HPP__

#include "MtasImplantProcessor.hpp"
#include "MtasProcessor.hpp"
#include "PidProcessor.hpp"
#include "Processor.hpp"

class e21069b_fp2Processor : public Processor{
	public:
		e21069b_fp2Processor(const std::string&);
		virtual ~e21069b_fp2Processor() = default;
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

		bool HasMtas;
		bool HasPid;
		bool HasImplant;

		std::unique_ptr<MtasProcessor> MtasProc;
		std::unique_ptr<PidProcessor> PidProc;
		std::unique_ptr<MtasImplantProcessor> MtasImplantProc;
		
};

#endif
