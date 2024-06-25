#ifndef __BSM_PROCESSOR_HPP__
#define __BSM_PROCESSOR_HPP__

#include <string>
#include <unordered_map>

#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "Processor.hpp"

class BSMProcessor : public Processor{
	public:
		BSMProcessor(const std::string&);
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
			double TotalEnergy;
			std::vector<double> SumFrontBackEnergy;
			std::vector<double> Position;
			double UnCorrectedTotalEnergy;
			std::vector<double> UnCorrectedSumFrontBackEnergy;
			double FirstTime;
			double LastTime;
			int NumValidSegments;
			bool Saturate;
			bool Pileup;
			bool RealEvt;
		};

		EventInfo& GetCurrEvt();
		EventInfo& GetPrevEvt();

		struct PosCorrection{
			double constant;
			double slope;
			double mean;

			double Correct(double erg,double pos){
				double val = mean/std::exp(constant + slope*pos);
				return val;
			}
		};


	private:

		double CalcPosition(double,double);

		void Reset();

		EventInfo CurrEvt;
		EventInfo PrevEvt;
		EventInfo NewEvt;

		std::vector<double> UnCorrectedBSM;
		std::vector<int> BSMHits;

		std::map<int,PosCorrection> PosCorrectionMap;

		std::string fronttag;
		std::string backtag;

		bool foundfirstevt;
		double globalfirsttime;
};

#endif
