#ifndef __CLOVER_PROCESSOR_HPP__
#define __CLOVER_PROCESSOR_HPP__

#include "CloverStruct.hpp"
#include "Processor.hpp"

class CloverProcessor : public Processor {
public:
	CloverProcessor(const std::string&);
	virtual ~CloverProcessor() = default;
	[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool Process(EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;

	virtual void Finalize() final;

	virtual void Init(const pugi::xml_node&);

	virtual void DeclarePlots(PLOTS::PlotRegistry*);
	virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string, TTree*>&) final;
	virtual void CleanupTree() final;

	int GetNumCrystals() const;

private:
	void Reset();
	void FillCloverLeafStruc(PhysicsData* evt, ProcessorStruct::Leaf& leaf, const int& cloveridx);
	std::pair<double, double> GetLeafAngles(const int& cloveridx, const std::string& leafname);
	double GetCloverLength(const int& cloveridx);

	std::vector<ProcessorStruct::Clover> CloverDataVec;
	int NumClover;
	;
	int LeavesPerClover = 4;
};

#endif
