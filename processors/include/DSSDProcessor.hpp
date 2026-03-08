#ifndef __DSSD_PROCESSOR_HPP__
#define __DSSD_PROCESSOR_HPP__

#include <array>

#include "Processor.hpp"
#include "DSSDStruct.hpp"

class DSSDProcessor : public Processor {
public:
	DSSDProcessor(const std::string&);
	virtual ~DSSDProcessor() = default;
	[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool Process([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;

	virtual void Finalize() final;
	virtual void Init(const pugi::xml_node&);
	virtual void DeclarePlots(PLOTS::PlotRegistry*);
	virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string, TTree*>&) final;
	virtual void CleanupTree() final;

private:
	void Reset();

    int numstrips;
    int numDSSDs;
	int LL_Branch_Energy_Split;

	int currStrip;
	int currDSSD;

	std::vector<ProcessorStruct::DSSD> ION_DSSDData_vec;
	std::vector<ProcessorStruct::DSSD> BETA_DSSDData_vec;

	std::vector<std::pair<PhysicsData*, PhysicsData*>> LL_maxEnEvt;
	std::vector<PhysicsData*> LG_maxEnEvt;
	std::vector<PhysicsData*> HG_maxEnEvt;
	
	std::vector<int> LL_Mult;
	std::vector<int> HG_Mult;
	std::vector<int> LG_Mult;

};

#endif