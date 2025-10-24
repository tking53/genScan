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
		const double& GetHighGainPSD() const;
		const double& GetLowGainPSD() const;
	
		void Reset();
	private:
		std::pair<int,int> CalcXY(const unsigned int&) const;
		void CalcPosition(const std::vector<double>&,SIPMIMP::Image&);
		std::vector<size_t> get_sorted_indices(const std::vector<double>&);


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
		SIPMIMP::Image hgImage;
		double hgPSD;
		std::vector<double> HighGainAnodes;
		ProcessorStruct::MtasImplant HighGain;

		std::vector<short> LowGainAnodeHitMap;
		short LowGainDynodeHits;
		short LowGainAnodeHits;
		SIPMIMP::Image lgImage;
		double lgPSD;
		std::vector<double> LowGainAnodes;
		ProcessorStruct::MtasImplant LowGain;
};

#endif
