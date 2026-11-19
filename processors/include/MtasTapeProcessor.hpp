#ifndef __MTASTAPE_PROCESSOR_HPP__
#define __MTASTAPE_PROCESSOR_HPP__

#include "TapeCycle.hpp"
#include "Processor.hpp"

class MtasTapeProcessor : public Processor {
public:
	MtasTapeProcessor(const std::string&);
	virtual ~MtasTapeProcessor() = default;
	[[maybe_unused]] virtual bool PreProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool Process(EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;
	[[maybe_unused]] virtual bool PostProcess([[maybe_unused]] EventHistoryManager*, [[maybe_unused]] PLOTS::PlotRegistry*, [[maybe_unused]] CUTS::CutRegistry*) final;

	virtual void Finalize() final;

	virtual void Init(const pugi::xml_node&);

	virtual void DeclarePlots(PLOTS::PlotRegistry*);
	virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string, TTree*>&) final;
	virtual void CleanupTree() final;

	unsigned int GetCurrentCycleNumber() const;
	TAPE::CycleState GetCurrentCycleState() const;
	void IncrementCycleNumber();
	double GetCycleTimeInSeconds() const;

private:
	void Reset();

	double CycleStartTime;

	unsigned int CycleCount;

	TAPE::CycleState PrevState;
	TAPE::CycleState CurrState;

	bool isTriggerOn;
	bool isIrradOn;
	bool isIrradOff;
	bool isLightPulseOn;
	bool isLightPulseOff;
	bool isTapeMoveOn;
	bool isTapeMoveOff;
	bool isBkgOn;
	bool isBkgOff;
	bool isMeasureOn;
	bool isMeasureOff;

	int logicSignalValue;

	int CycleRoll;

	std::string tapemove;
	std::string measure;
	std::string background;
	std::string irradiation;
	std::string lightpulser;
	std::string unknown;
};

#endif
