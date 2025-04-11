#ifndef __MTAS_PROCESSOR_HPP__
#define __MTAS_PROCESSOR_HPP__

#include "Correction.hpp"
#include "Geometry.hpp"
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

		const double& GetSumFrontBackEnergy(const int&) const;
		bool DidIndividualPMTSaturate(const int&) const;
		bool DidIndividualPMTPileup(const int&) const;

		const double& GetFirstFireTime() const;
		const double& GetLastFireTime() const;

		void FillBetaPlots(PLOTS::PlotRegistry*);
		void FillNonBetaPlots(PLOTS::PlotRegistry*);

	private:
		double CalcPosition(double,double);
		void Reset();
		void GenerateHexagonShapes();

		void DeclareBetaPlots(PLOTS::PlotRegistry*);
		void DeclareAntiBetaPlots(PLOTS::PlotRegistry*);

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
		std::vector<double> SumFrontBackEnergy;

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

		double hexagonsize;
		double hexagonpad;
		std::vector<Geometry::hexagon> HexagonShapes;

		TH2Poly* MTAS_2500;
		TH2Poly* MTAS_2501;
		TH2Poly* MTAS_2502;
		TH2Poly* MTAS_2503;

		TH1* MTAS_3100;
		TH2* MTAS_3101;
		TH1* MTAS_3110;
		TH1* MTAS_3115;
		TH1* MTAS_3120;
		TH1* MTAS_3125;
		TH1* MTAS_3130;
		TH1* MTAS_3135;
		TH1* MTAS_3140;
		TH1* MTAS_3145;
		
		TH2* MTAS_3150;
		TH2* MTAS_3151;
		TH2* MTAS_3152;
		TH2* MTAS_3153;
		TH2* MTAS_3154;
		
		TH2* MTAS_31508;
		TH2* MTAS_31518;
		TH2* MTAS_31528;
		TH2* MTAS_31538;
		TH2* MTAS_31548;

		TH2* MTAS_4100;
		TH2* MTAS_4101;
		TH2* MTAS_4102;
		TH2* MTAS_4103;
		TH2* MTAS_4104;

		TH1* MTAS_3200;
		TH2* MTAS_3201;
		TH1* MTAS_3210;
		TH1* MTAS_3215;
		TH1* MTAS_3220;
		TH1* MTAS_3225;
		TH1* MTAS_3230;
		TH1* MTAS_3235;
		TH1* MTAS_3240;
		TH1* MTAS_3245;
		
		TH2* MTAS_3250;
		TH2* MTAS_3251;
		TH2* MTAS_3252;
		TH2* MTAS_3253;
		TH2* MTAS_3254;
		
		TH2* MTAS_32508;
		TH2* MTAS_32518;
		TH2* MTAS_32528;
		TH2* MTAS_32538;
		TH2* MTAS_32548;

		TH2* MTAS_4200;
		TH2* MTAS_4201;
		TH2* MTAS_4202;
		TH2* MTAS_4203;
		TH2* MTAS_4204;

		TH1* MTAS_3300;
		TH2* MTAS_3301;
		TH1* MTAS_3310;
		TH1* MTAS_3315;
		TH1* MTAS_3320;
		TH1* MTAS_3325;
		TH1* MTAS_3330;
		TH1* MTAS_3335;
		TH1* MTAS_3340;
		TH1* MTAS_3345;
		
		TH2* MTAS_3350;
		TH2* MTAS_3351;
		TH2* MTAS_3352;
		TH2* MTAS_3353;
		TH2* MTAS_3354;
		
		TH2* MTAS_33508;
		TH2* MTAS_33518;
		TH2* MTAS_33528;
		TH2* MTAS_33538;
		TH2* MTAS_33548;

		TH2* MTAS_4300;
		TH2* MTAS_4301;
		TH2* MTAS_4302;
		TH2* MTAS_4303;
		TH2* MTAS_4304;

};

#endif
