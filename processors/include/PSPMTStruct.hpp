#ifndef __PSPMT_STRUCT_HPP__
#define __PSPMT_STRUCT_HPP__

#include <TString.h>

namespace ProcessorStruct {
	struct PSPMT {
		double xpos = -999.0;
		double ypos = -999.0;
		int xposqdc = -999.0;
		int yposqdc = -999.0;
		double dynodeen = 0.0;
		double dynodeqdc = 0.0;
		double dynodets = -1.0;
	};
	static const PSPMT DEFAULT_PSPMT_STRUCT;
} // namespace ProcessorStruct

#endif
