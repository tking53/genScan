#ifndef __ROOT_DEV_STRUCT_HPP__
#define __ROOT_DEV_STRUCT_HPP__

#include <TString.h>

namespace ProcessorStruct {
	struct RootDev {
		double energy = -999;
		double rawEnergy = -999;
		double timeSansCfd = -999;
		double time = -999;
		bool cfdForcedBit = false;
		double cfdFraction = -999;
		int cfdSourceBit = -999;
		int crateNum = -999;
		int detNum = -999; // the instance number of RD in the xml Map
		int modNum = -999; // the physical module number
		int chanNum = -999; // the physical channel number
		int gchanid = -999;
		int gmodid = -999;
		TString type = "";
		TString subtype = "";
		TString group = "";
		TString tag = "";
		bool pileup = false; // Did pixie detect pileup in the event
		bool saturation = false; // Did the trace go out of the ADC range
		std::vector<unsigned int> trace = {}; // The trace if present
		int tracelength = 0;
		std::vector<unsigned int> qdcSums = {}; // output the onboard qdc sums if present
		int qdclength = 0;
		double tmax = 0.0;
	};
	static const RootDev DEFAULT_RD_STRUCT;
} // namespace ProcessorStruct

#endif
