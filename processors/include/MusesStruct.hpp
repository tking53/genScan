#ifndef __MUSES_STRUCT_HPP__
#define __MUSES_STRUCT_HPP__

#include <TString.h>

namespace ProcessorStruct {
	struct MusesPixel {
		double timestamp = -999.0;
		double energy = -999.0;
		bool saturate = false;
		bool pileup = false;
		int pixelid = -1;
	};
	static const MusesPixel DEFAULT_MUSES_PIXEL_STRUCT;
} // namespace ProcessorStruct

#endif
