#ifndef __SINGLE_GROVER_PROCESSOR_HPP__
#define __SINGLE_GROVER_PROCESSOR_HPP__

#include "Processor.hpp"
#include "GroverStruct.hpp"

class SingleGroverProcessor : public Processor{
	public:
		SingleGroverProcessor(const std::string&);
		virtual ~SingleGroverProcessor() = default;
		[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool Process(EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;

		virtual void Finalize() final;

		virtual void Init(const pugi::xml_node&);

		virtual void DeclarePlots(PLOTS::PlotRegistry*);
		virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>&) final;
		virtual void CleanupTree() final;

		double GetEnergy(int,bool) const;
		double GetCrystalFireTime(int,bool) const;
		bool DidCrystalPileup(int,bool) const;
		bool DidCrystalSaturate(int,bool) const;

	private:
		struct GroverInfo {
			double energy;
			double timestamp;
			int hits;
			bool saturate;
			bool pileup;
		};

		void Reset();
		void ResetInfo(GroverInfo&);

		std::string highgaintag;
		std::string lowgaintag;

		std::vector<GroverInfo> HighGain;
		std::vector<ProcessorStruct::GroverLeaf> HG;
		std::vector<GroverInfo> LowGain;
		std::vector<ProcessorStruct::GroverLeaf> LG;
};

#endif
