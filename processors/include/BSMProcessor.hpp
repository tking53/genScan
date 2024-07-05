#ifndef __BSM_PROCESSOR_HPP__
#define __BSM_PROCESSOR_HPP__

#include "Processor.hpp"

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

		struct EventInfo{
			double TotalEnergy;
			std::vector<double> SumFrontBackEnergy;
			std::vector<double> Position;
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

		struct PosCorrection{
			double constant;
			double slope;
			double mean;

			double Correct(double erg,double pos){
				double val = mean/std::exp(constant + slope*pos);
				return val*erg;
			}

			PosCorrection() = default;
			~PosCorrection() = default;
			PosCorrection(const PosCorrection&) = default;
			PosCorrection(PosCorrection&&) = default;
			PosCorrection& operator=(const PosCorrection&) = default;
			PosCorrection& operator=(PosCorrection&&) = default;
		};

		struct TraceAnalysis{
			int lowerbound;
			int upperbound;
			float integralthreshold;
			float lowthresh;
			float highthresh;
			std::pair<float,float> baseline;
			std::pair<float,float> integrals;
			std::pair<size_t,float> peakmax;
			float psd;

			TraceAnalysis(){
				lowerbound = -1;
				upperbound = -1;
				integralthreshold = 0.0;
				lowthresh = 0.0;
				highthresh = 0.0;
				baseline = {0.0,0.0};
				integrals = {0.0,0.0};
				peakmax = {0,0.0};
				psd = 0.0;
			}

			~TraceAnalysis() = default;
			TraceAnalysis(const TraceAnalysis&) = default;
			TraceAnalysis(TraceAnalysis&&) = default;
			TraceAnalysis& operator=(const TraceAnalysis&) = default;
			TraceAnalysis& operator=(TraceAnalysis&&) = default;

			void Reset(){
				baseline = {0.0,0.0};
				integrals = {0.0,0.0};
				peakmax = {0,0.0};
				psd = 0.0;
			}

			bool ValidateSelf(){
				return (lowerbound < upperbound) and (lowerbound >= 0) and (upperbound >= 0) and (lowthresh <= highthresh);
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
		std::vector<TraceAnalysis*> TraceSettings;

		std::vector<double> TimeStamps;

		std::map<int,PosCorrection> PosCorrectionMap;

		std::string fronttag;
		std::string backtag;

		bool foundfirstevt;
		double globalfirsttime;

		bool PlotAllTraces;
};

#endif
