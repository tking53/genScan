#ifndef __MTAS_STRUCT_HPP__
#define __MTAS_STRUCT_HPP__

#include <vector>

#include <TString.h>

namespace ProcessorStruct{
	struct MtasSegment{
		double frontenergy = -999.0;
		double fronttimestamp = -999.0;

		double backenergy = -999.0;
		double backtimestamp = -999.0;

		double sumenergy = -999.0;
		double avgtimestamp = -999.0;
	};
	static const MtasSegment DEFAULT_MTAS_SEGMENT_STRUCT;

	struct MtasTotal{
		double timestamp = -999.0;
		double sumenergy = -999.0;
		int numfire = 0;
		bool saturate = false;
		bool pileup = false;
	};
	static const MtasTotal DEFAULT_MTAS_TOTAL_STRUCT; 
}

#endif
