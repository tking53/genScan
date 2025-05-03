#ifndef __MTAS_PROCESSOR_HPP__
#define __MTAS_PROCESSOR_HPP__

#include "Correction.hpp"
#include "Geometry.hpp"
#include "HistogramManager.hpp"
#include "MtasStruct.hpp"
#include "Processor.hpp"

class MtasProcessor : public Processor{
	public:
		MtasProcessor(const std::string&);
		virtual ~MtasProcessor() = default;
		[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool Process(EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;

		virtual void Finalize() final;

		virtual void Init(const pugi::xml_node&);

		virtual void DeclarePlots(PLOTS::PlotRegistry*);
		virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>&) final;
		virtual void CleanupTree() final;

		const double& GetTotalEnergy(const int&) const;

		const int& GetNumPairsFire() const;
		const int& GetNumCenterPairsFire() const;
		const int& GetNumInnerPairsFire() const;
		const int& GetNumMiddlePairsFire() const;
		const int& GetNumOuterPairsFire() const;

		const bool& DidAnyFire() const;
		const bool& DidAnyCenterFire() const;
		const bool& DidAnyInnerFire() const;
		const bool& DidAnyMiddleFire() const;
		const bool& DidAnyOuterFire() const;

		const bool& DidAnySaturate() const;
		const bool& DidAnyCenterSaturate() const;
		const bool& DidAnyInnerSaturate() const;
		const bool& DidAnyMiddleSaturate() const;
		const bool& DidAnyOuterSaturate() const;

		const bool& DidAnyPileup() const;
		const bool& DidAnyCenterPileup() const;
		const bool& DidAnyInnerPileup() const;
		const bool& DidAnyMiddlePileup() const;
		const bool& DidAnyOuterPileup() const;

		const double& GetCrystalEnergy(const int&) const;
		bool DidIndividualPMTSaturate(const int&) const;
		bool DidIndividualPMTPileup(const int&) const;

		const double& GetFirstFireTime() const;
		const double& GetLastFireTime() const;

		void FillBetaPlots(PLOTS::PlotRegistry*);
		void FillNonBetaPlots(PLOTS::PlotRegistry*);
		void FillNoLogicBetaPlots(PLOTS::PlotRegistry*);
		void FillNoLogicNonBetaPlots(PLOTS::PlotRegistry*);

	private:
		double CalcPosition(double,double);
		void Reset();
		void GenerateHexagonShapes();

		void DeclareBetaPlots(PLOTS::PlotRegistry*);
		void DeclareAntiBetaPlots(PLOTS::PlotRegistry*);

		bool UseOldCenter;
		void NewCenterCalculation();
		void OldCenterCalculation();

		std::vector<double> Position;

		std::vector<double> Center;
		std::vector<double> Inner;
		std::vector<double> Middle;
		std::vector<double> Outer;

		std::vector<double> RawCenter;
		std::vector<double> RawInner;
		std::vector<double> RawMiddle;
		std::vector<double> RawOuter;

		std::vector<double> CalCenter;
		std::vector<double> CalInner;
		std::vector<double> CalMiddle;
		std::vector<double> CalOuter;

		std::vector<int> CenterHits;
		std::vector<int> InnerHits;
		std::vector<int> MiddleHits;
		std::vector<int> OuterHits;

		std::vector<bool> IndividualPMTSaturate;
		bool CenterSaturate;
		bool InnerSaturate;
		bool MiddleSaturate;
		bool OuterSaturate;
		bool AnySaturate;

		std::vector<bool> IndividualPMTPileup;
		bool CenterPileup;
		bool InnerPileup;
		bool MiddlePileup;
		bool OuterPileup;
		bool AnyPileup;
			
		std::vector<int> NumFire;
		bool CenterFire;
		bool InnerFire;
		bool MiddleFire;
		bool OuterFire;
		bool AnyFire;
			
		std::vector<double> TotalEnergy;
		std::vector<double> CrystalEnergy;

		std::vector<double> TimeStamps;
			
		double FirstTime;
		double LastTime;
		
		std::vector<ProcessorStruct::MtasSegment> SegmentDataVec;
		std::vector<ProcessorStruct::MtasTotal> TotalDataVec;

		enum SUBTYPE{
			CENTER,
			INNER,
			MIDDLE,
			OUTER,
			UNKNOWN
		};

		std::string fronttag;
		std::string backtag;

		SUBTYPE currsubtype;

		bool foundfirstevt;
		double globalfirsttime;
		double currevttime;

		bool diagnosticplots;

		std::vector<std::unique_ptr<Correction::ExpoPosCorrection>> PosCorrectionMap;

		std::string gamma;
		std::string muon;

		double hexagonsize;
		double hexagonpad;
		std::vector<Geometry::hexagon> HexagonShapes;

		//anti-beta hit map
		TH2Poly* MTAS_2400;
		TH2Poly* MTAS_2401;
		TH2Poly* MTAS_2402;
		TH2Poly* MTAS_2403;

		//general hit map
		TH2Poly* MTAS_2500;
		TH2Poly* MTAS_2501;
		TH2Poly* MTAS_2502;
		TH2Poly* MTAS_2503;

		//beta hit map
		TH2Poly* MTAS_2600;
		TH2Poly* MTAS_2601;
		TH2Poly* MTAS_2602;
		TH2Poly* MTAS_2603;

};

#endif
