#ifndef __PID_PROCESSOR_HPP__
#define __PID_PROCESSOR_HPP__

#include "Processor.hpp"
#include "PidStruct.hpp"

class PidProcessor : public Processor{
	public:
		PidProcessor(const std::string&);
		virtual ~PidProcessor() = default;
		[[maybe_unused]] bool PreProcess([[maybe_unused]] EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] bool Process(EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;
		[[maybe_unused]] bool PostProcess([[maybe_unused]] EventHistoryManager*,[[maybe_unused]] PLOTS::PlotRegistry*,[[maybe_unused]] CUTS::CutRegistry*) final;

		virtual void Finalize() final;

		void Init(const pugi::xml_node&);

		void DeclarePlots(PLOTS::PlotRegistry*);
		virtual void RegisterTree([[maybe_unused]] std::unordered_map<std::string,TTree*>&) final;
		virtual void CleanupTree() final;
	private:

	ProcessorStruct::DBOX db3;
	ProcessorStruct::DBOX db4;
	ProcessorStruct::DBOX db5;	
	ProcessorStruct::FP fp1;
	ProcessorStruct::FP fp2;
	double currRF;
	

	enum DETPOSITION{
		UP,
		DOWN,
		LEFT,
		RIGHT,
		ANODE,
		
	};

	enum BOXID{
		DB3,
		DB4,
		DB5,
		FP1,
		FP2,
		
	};

	enum DETSUBTYPE	{
		SCINT,
		PPAC0,
		PPAC1,
		PIN,
		RF
	};

	DETPOSITION currDETPOSITION;
	DETSUBTYPE currDETSUBTYPE;
	BOXID currBOXID;

	int numPins;
};

#endif
