#ifndef __MTASIMPLANT_PROCESSOR_HPP__
#define __MTASIMPLANT_PROCESSOR_HPP__

#include "Processor.hpp"
#include "MtasImplantStruct.hpp"

class MtasImplantProcessor : public Processor{
	public:
		MtasImplantProcessor(const std::string&);
		virtual ~MtasImplantProcessor() = default;
		[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool Process(EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;

		virtual void Finalize() final;

		virtual void Init(const pugi::xml_node&);

		virtual void DeclarePlots(PLOTS::PlotRegistry*);
		virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>&) final;
		virtual void CleanupTree() final;

		void Reset();
	private:
		std::pair<unsigned int,unsigned int> CalcXY(const unsigned int&) const;
		void CalcPosition(const std::vector<double>&,std::pair<double,double>&,std::pair<unsigned int,unsigned int>&);

		enum IMPLANTSIPMTYPE{
			HIGHGAINANODE,
			HIGHGAINDYNODE,
			LOWGAINANODE,
			LOWGAINDYNODE,
			UNKNOWN
		};

		IMPLANTSIPMTYPE currsipmtype;

		std::string highgaintag;
		std::string lowgaintag;

		double YSOHGThreshold;
		double YSOLGThreshold;

		std::pair<double,double> IsBetaThresh;
		std::pair<double,double> IsIonThresh;

		std::vector<short> HighGainAnodeHitMap;
		short HighGainDynodeHits;
		short HighGainAnodeHits;
		double HighGainDynode;
		double HighGainDynodeOQDC;
		double HighGainDynodeTS;
		std::vector<double> HighGainAnodes;
		std::vector<double> HighGainAnodesOQDC;
		std::pair<double,double> HighResHighGainPosition;
		std::pair<unsigned int,unsigned int> LowResHighGainPosition;
		ProcessorStruct::MtasImplant HighGain;

		std::vector<short> LowGainAnodeHitMap;
		short LowGainDynodeHits;
		short LowGainAnodeHits;
		double LowGainDynode;
		double LowGainDynodeOQDC;
		double LowGainDynodeTS;
		std::vector<double> LowGainAnodes;
		std::vector<double> LowGainAnodesOQDC;
		std::pair<double,double> HighResLowGainPosition;
		std::pair<unsigned int,unsigned int> LowResLowGainPosition;
		ProcessorStruct::MtasImplant LowGain;
};

#endif
