#ifndef __MUSES_PROCESSOR_HPP__
#define __MUSES_PROCESSOR_HPP__

#include "PhysicsData.hpp"
#include "Processor.hpp"
#include "MusesStruct.hpp"

class MusesProcessor : public Processor{
	public:
		MusesProcessor(const std::string&);
		virtual ~MusesProcessor() = default;
		[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool Process(EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;

		virtual void Finalize() final;

		virtual void Init(const pugi::xml_node&);

		virtual void DeclarePlots(PLOTS::PlotRegistry*);
		virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>&) final;
		virtual void CleanupTree() final;

		double GetMaxEnergy() const;
		double GetPixelEnergy(int) const;

		const double& GetFirstFireTime() const;
		const double& GetLastFireTime() const;

	private:

		void Reset();

		double MaxErg;
		double Maxidx;

		std::vector<double> TimeStamps;
		double FirstTime;
		double LastTime;

		std::vector<int> PixelHits;
		std::vector<double> Pixels;

		std::vector<ProcessorStruct::MusesPixel> Pix;
		std::vector<std::pair<double,double>> Positions;
};

#endif
