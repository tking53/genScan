#ifndef __DLS_STRUCT_HPP__
#define __DLS_STRUCT_HPP__

#include <TString.h>

namespace ProcessorStruct {
	struct DLS {
		double psd = -999.0;
		double energy = 0.0;
		double ts = -999.0;
		double tmax = -1.0;
		bool saturate = false;
		bool pileup = false;
		TString subtype = "";
		TString group = "";
		TString tag = "";
	};
	static const DLS DEFAULT_DLS_STRUCT;


} // namespace ProcessorStruct

#endif
