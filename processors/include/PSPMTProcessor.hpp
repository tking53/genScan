#ifndef __PSPMT_PROCESSOR_HPP__
#define __PSPMT_PROCESSOR_HPP__

#include <vector>
#include <utility>

#include "Processor.hpp"

class PSPMTProcessor : public Processor{
	public:
		PSPMTProcessor(const std::string&);
		virtual ~PSPMTProcessor() = default;
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

		struct Image{
			double dynode;
			double xa;
			double xb;
			double ya;
			double yb;
			double anodesum;
			int numanodes;
			std::pair<double,double> position;
			double DynodeTimeStamp;
		};

		struct EventInfo{
			Image hg;
			Image lg;
			double ampdynode;
			bool Pileup;
			bool Saturate;
			bool RealEvt;
		};

		EventInfo& GetCurrEvt();
		EventInfo& GetPrevEvt();

	private:
		void Reset();
		void CalculatePosition(Image&,double,double,double,bool);
		
		EventInfo CurrEvt;
		EventInfo PrevEvt;
		EventInfo NewEvt;

		int DynodeHighHits;
		int DynodeLowHits;
		int AmpDynodeHits;
		std::vector<int> AnodeLowHits;
		std::vector<int> AnodeHighHits;

		std::string highgaintag;
		std::string lowgaintag;
};

#endif
