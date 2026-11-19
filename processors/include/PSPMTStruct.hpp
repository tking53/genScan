#ifndef __PSPMT_STRUCT_HPP__
#define __PSPMT_STRUCT_HPP__

#include <TString.h>

namespace ProcessorStruct {
		
	struct CORNER {
		double energy = -999.0;
		double qdc = -999.0;
		bool saturation = false;
		bool pileup = false;
	};
	struct PSPMT {
		double xpos = -999.0;
		double ypos = -999.0;
		double xposqdc = -999.0;
		double yposqdc = -999.0;
		double dynodets = -999.0;
		double anodesumen = -999.0;
		double anodesumqdc = -999.0;
		CORNER xa, xb, ya, yb, dynode;

	};
	static const PSPMT DEFAULT_PSPMT_STRUCT;
} // namespace ProcessorStruct

#endif
