#ifndef __BSM_PROCESSOR_HPP__
#define __BSM_PROCESSOR_HPP__

#include "Processor.hpp"
#include "BSMStruct.hpp"
#include "Correction.hpp"

class BSMProcessor : public Processor{
	public:
		BSMProcessor(const std::string&);
		virtual ~BSMProcessor() = default;
		[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool Process(EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;

		virtual void Finalize() final;

		virtual void Init(const pugi::xml_node&);

		virtual void DeclarePlots(PLOTS::PlotRegistry*);
		virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>&) final;
		virtual void CleanupTree() final;

		const double& GetAverageTotalEnergy() const;
		const double& GetSumFrontBackEnergy(const int&) const;

		const double& GetGeometricTotalEnergy() const;
		const double& GetGeometricFrontBackEnergy(const int&) const;

		const double& GetPosition(const int&) const;

		const double& GetTDiff(const int&) const;

		const double& GetFirstFireTime() const;
		const double& GetLastFireTime() const;

		bool DidIndividualPMTSaturate(const int&) const;
		const bool& DidAnySaturate() const;

		bool DidIndividualPMTPileup(const int&) const;
		const bool& DidAnyPileup() const;

		void FillGSPileupTracePlots(PLOTS::PlotRegistry*) const;
		void FillPositionPlots(PLOTS::PlotRegistry*) const;

		int GetNumPMTs() const;
		int GetNumSegments() const;

		double GetIndividualPMTEnergy(const int&) const;

		int GetBSMHits(const int&) const;

	private:
		struct TraceAnalysis{
			float integralthreshold;
			std::string cutid;

			TraceAnalysis(){
				integralthreshold = 0.0;
				cutid = "";
			}
		};


		double CalcPosition(double,double);

		void Reset();

		std::vector<PhysicsData*> Pairs;

		std::vector<double> RawBSM;
		std::vector<double> UnCorrectedBSM;
		std::vector<double> CorrectedBSM;
		std::vector<int> BSMHits;
		std::vector<int> TotalMult;
		std::vector<std::unique_ptr<TraceAnalysis>> TraceSettings;

		ProcessorStruct::BSMTraceFit fronttracefitvalues;
		ProcessorStruct::BSMTraceFit backtracefitvalues;

		std::vector<ProcessorStruct::BSMSingle> PMTDataVec;

		double AverageTotalEnergy;
		double GeometricTotalEnergy;
		std::vector<double> SumFrontBackEnergy;
		std::vector<double> GeometricFrontBackEnergy;
		
		std::vector<double> Position;
		std::vector<double> TDiff;

		double FirstTime;
		double LastTime;

		int NumValidSegments;

		std::vector<bool> IndividualPMTSaturate;
		bool AnySaturate;
		
		std::vector<bool> IndividualPMTPileup;
		bool AnyPileup;

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

		TH2Poly* BSM_2500;
		TH2Poly* BSM_2501;
		TH2Poly* BSM_2502;
		TH2Poly* BSM_2503;
};

#endif
