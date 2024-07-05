#ifndef __MTAS_PROCESSOR_HPP__
#define __MTAS_PROCESSOR_HPP__

#include "Correction.hpp"
#include "MtasStruct.hpp"
#include "Processor.hpp"

class MtasProcessor : public Processor{
	public:
		MtasProcessor(const std::string&);
		virtual ~MtasProcessor() = default;
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
			std::vector<double> TotalEnergy;
			std::vector<double> SumFrontBackEnergy;
			double FirstTime;
			double LastTime;
			bool Saturate;
			bool Pileup;
			bool BetaTriggered;
			bool RealEvt;

			EventInfo(){
				TotalEnergy = std::vector<double>(5,0.0);
				SumFrontBackEnergy = std::vector<double>(24,0.0);
				FirstTime = -1.0;
				LastTime = -1.0;
				Saturate = false;
				Pileup = false;
				BetaTriggered = false;
				RealEvt = false;
			}
			~EventInfo() = default;
			EventInfo(const EventInfo&) = default;
			EventInfo(EventInfo&&) = default;
			EventInfo& operator=(const EventInfo&) = default;
			EventInfo& operator=(EventInfo&&) = default;
		};

		void SetIsBeta();
		EventInfo& GetCurrEvt();
		EventInfo& GetPrevEvt();

		void FillBetaPlots(PLOTS::PlotRegistry*);
		void FillNonBetaPlots(PLOTS::PlotRegistry*);

	private:
		double CalcPosition(double,double);
		void Reset();

		EventInfo CurrEvt;
		EventInfo PrevEvt;
		EventInfo NewEvt;

		std::vector<double> Position;
		std::vector<double> Center;
		std::vector<double> Inner;
		std::vector<double> Middle;
		std::vector<double> Outer;
		std::vector<int> CenterHits;
		std::vector<int> InnerHits;
		std::vector<int> MiddleHits;
		std::vector<int> OuterHits;

		std::vector<double> TimeStamps;
		
		std::vector<ProcessorStruct::MtasSegment> SegmentDataVec;
		ProcessorStruct::MtasSegment CurrSegmentData;
		std::vector<ProcessorStruct::MtasTotal> TotalDataVec;
		ProcessorStruct::MtasTotal CurrTotalData;

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

		std::vector<std::unique_ptr<Correction::ExpoPosCorrection>> PosCorrectionMap;
};

#endif
