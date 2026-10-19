#ifndef __GROVER_STRUCT_HPP__
#define __GROVER_STRUCT_HPP__

#include <TString.h>

namespace ProcessorStruct {
	struct GroverLeaf {
		double timestamp = -999.0;
		double energy = -999.0;
		bool saturate = false;
		bool pileup = false;
		int leafid = -1;
	};
	static const GroverLeaf DEFAULT_GROVER_LEAF_STRUCT;
} // namespace ProcessorStruct

#endif
