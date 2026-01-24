#ifndef __MTASIMPLANT_PROCESSOR_HPP__
#define __MTASIMPLANT_PROCESSOR_HPP__

#include "Processor.hpp"
#include "MtasImplantStruct.hpp"

#include "ImageManipulation.hpp"

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

		const SIPMIMP::Image& GetLowGainImage() const;
		const SIPMIMP::Image& GetHighGainImage() const;
		const SIPMIMP::Image& GetLowGainQDCImage() const;
		const SIPMIMP::Image& GetHighGainQDCImage() const;
		const double& GetHighGainPSD() const;
		const double& GetLowGainPSD() const;
	
		void Reset();
	private:
		struct AnodeHitInfo{
			double energy;
			double timestamp;
			short hits;
		} ;

		void AssignRootStruct(ProcessorStruct::MtasImplant&,const SIPMIMP::Image&);
		void ResetImage(SIPMIMP::Image&);
		void ResetAnodes(std::vector<AnodeHitInfo>&);
		
		std::pair<double,double> CalcXY(const unsigned int&) const;
		void CalcPosition(const std::vector<AnodeHitInfo>&,SIPMIMP::Image&);
		std::vector<size_t> get_sorted_indices(const std::vector<double>&);

		std::vector<std::pair<double,double>> PositionMap;

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

		size_t qdc_baseline_idx;
		size_t qdc_energy_idx;

		short HighGainDynodeHits;
		short HighGainAnodeHits;
		SIPMIMP::Image hgImage;
		SIPMIMP::Image hgQDCImage;
		double hgPSD;
		std::vector<AnodeHitInfo> HighGainAnodes;
		std::vector<AnodeHitInfo> HighGainQDCAnodes;
		ProcessorStruct::MtasImplant HighGain;
		ProcessorStruct::MtasImplant HighGainQDC;

		short LowGainDynodeHits;
		short LowGainAnodeHits;
		SIPMIMP::Image lgImage;
		SIPMIMP::Image lgQDCImage;
		double lgPSD;
		std::vector<AnodeHitInfo> LowGainAnodes;
		std::vector<AnodeHitInfo> LowGainQDCAnodes;
		ProcessorStruct::MtasImplant LowGain;
		ProcessorStruct::MtasImplant LowGainQDC;

};

#endif
