#ifndef __BSMEXP_PROCESSOR_HPP__
#define __BSMEXP_PROCESSOR_HPP__

#include "BSMProcessor.hpp"
#include "MtasProcessor.hpp"
#include "Processor.hpp"

class BSMExpProcessor : public Processor{
	public:
		BSMExpProcessor(const std::string&);
		virtual ~BSMExpProcessor() = default;
		[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool Process(EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;

		virtual void Finalize() final;

		virtual void Init(const YAML::Node&);
		virtual void Init(const Json::Value&);
		virtual void Init(const pugi::xml_node&);

		virtual void DeclarePlots(PLOTS::PlotRegistry*);
		virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>&) final;
		virtual void CleanupTree() final;
	private:
		bool HasMTAS;
		bool HasBSM;

		double BetaThreshold;
		double QBeta;
		std::pair<double,double> BSMPosBounds;

		bool PPCutExists;

		std::unique_ptr<BSMProcessor> BSMProc;
		std::unique_ptr<MtasProcessor> MtasProc;

		TH1* BSMEXP_2000;

		TH1* BSMEXP_3600;
		TH1* BSMEXP_3601;
		TH1* BSMEXP_3602;
		TH1* BSMEXP_3603;
		TH1* BSMEXP_3604;
		TH1* BSMEXP_3605;
		
		TH1* BSMEXP_3610;
		TH1* BSMEXP_3611;
		TH1* BSMEXP_3612;

		TH2* BSMEXP_3650;
		TH2* BSMEXP_36508;
		TH2* BSMEXP_3651;
		TH2* BSMEXP_36518;
		TH2* BSMEXP_3652;
		TH2* BSMEXP_36528;
		TH2* BSMEXP_3653;
		TH2* BSMEXP_36538;
		TH2* BSMEXP_3654;
		TH2* BSMEXP_36548;
		TH2* BSMEXP_3655;
		TH2* BSMEXP_36558;
		TH2* BSMEXP_3656;
		TH2* BSMEXP_36568;
		TH2* BSMEXP_3657;
		TH2* BSMEXP_36578;
		TH2* BSMEXP_3658;
		TH2* BSMEXP_36588;

		TH2* BSMEXP_3660;
		TH2* BSMEXP_36608;
		TH2* BSMEXP_3661;
		TH2* BSMEXP_36618;

		TH1* BSMEXP_2000_PP;
		TH1* BSMEXP_3600_PP;

		TH1* BSMEXP_3300_PILEUP;
};

#endif
