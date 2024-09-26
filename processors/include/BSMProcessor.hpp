#ifndef __BSM_PROCESSOR_HPP__
#define __BSM_PROCESSOR_HPP__

#include "Processor.hpp"
#include "PulseFitFunctions.hpp"

#include "Correction.hpp"

namespace PulseFit{
	double BSMSingleTraceFit(double*,double*);
	double BSMDoubleTraceFit(double*,double*);
}

class BSMProcessor : public Processor{
	public:
		BSMProcessor(const std::string&);
		virtual ~BSMProcessor() = default;
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

		void FillGSPileupTracePlots(PLOTS::PlotRegistry*) const;
		void FillPositionPlots(PLOTS::PlotRegistry*) const;

		struct EventInfo{
			double TotalEnergy;
			std::vector<double> SumFrontBackEnergy;
			std::vector<double> Position;
			std::vector<double> TDiff;
			std::vector<double> CorrectedBSM;
			double UnCorrectedTotalEnergy;
			std::vector<double> UnCorrectedSumFrontBackEnergy;
			std::vector<double> UnCorrectedBSM;
			double FirstTime;
			double LastTime;
			int NumValidSegments;
			bool Saturate;
			bool Pileup;
			bool RealEvt;

			EventInfo(){
				TotalEnergy = 0.0;
				SumFrontBackEnergy = std::vector<double>(6,0.0);
				Position = std::vector<double>(6,0.0);
				TDiff = std::vector<double>(6,-1.0e9);
				CorrectedBSM = std::vector<double>(12,0.0);
				UnCorrectedTotalEnergy = 0.0;
				UnCorrectedSumFrontBackEnergy = std::vector<double>(6,0.0);
				UnCorrectedBSM = std::vector<double>(12,0.0);
				FirstTime = -1.0;
				LastTime = -1.0;
				NumValidSegments = 0;
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

		struct TraceAnalysis{
			float integralthreshold;
			std::string cutid;

			TraceAnalysis(){
				integralthreshold = 0.0;
				cutid = "";
			}
		};

	private:

		double CalcPosition(double,double);

		void Reset();

		EventInfo CurrEvt;
		EventInfo PrevEvt;
		EventInfo NewEvt;

		std::vector<PhysicsData*> Pairs;

		std::vector<double> RawBSM;
		std::vector<int> BSMHits;
		std::vector<int> TotalMult;
		std::vector<std::unique_ptr<TraceAnalysis>> TraceSettings;

		std::vector<double> TimeStamps;
		std::vector<double> HitTimeStamps;
		std::vector<std::vector<uint16_t>> Traces;

		std::vector<std::unique_ptr<Correction::ExpoPosCorrection>> PosCorrectionMap;

		std::string fronttag;
		std::string backtag;

		bool foundfirstevt;
		double globalfirsttime;
		double currevttime;

		int NumPairs;
		int NumPMTs;

		bool PlotAllTraces;
};

#endif
