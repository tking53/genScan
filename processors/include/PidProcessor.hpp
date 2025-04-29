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


	void FillStruct(PhysicsData* data, ProcessorStruct::PidDet &det);

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
		PIN1,
		PIN2,
		PIN3,
		PIN4,
		RF
	};

	DETPOSITION currDETPOSITION;
	DETSUBTYPE currDETSUBTYPE;
	BOXID currBOXID;

	double d3p0aF1X1TD ;
	double d3p0aF1X2TD ;
	double d3p1aF1X1TD ;
	double d3p1aF1X2TD ;
	double d3SLF1X1TD  ;
	double d3SLF1X2TD  ;
	double d3SRF1X1TD  ;
	double d3SRF1X2TD  ;
	;
	double d3p0aF2X1TD ;
	double d3p0aF2X2TD ;
	double d3p1aF2X1TD ;
	double d3p1aF2X2TD ;
	double d3SLF2X1TD  ;
	double d3SLF2X2TD  ;
	double d3SRF2X1TD  ;
	double d3SRF2X2TD  ;


};

#endif
