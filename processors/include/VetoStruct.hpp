#ifndef __VETO_STRUCT_HPP__
#define __VETO_STRUCT_HPP__

namespace ProcessorStruct {
	struct Veto {
		double timestamp = -999.0;
		double energy = -999.0;
		bool saturate = false;
		bool pileup = false;
		double head = -999.0;
		double tail = -999.0;
	};
	static const Veto DEFAULT_VETO_STRUCT;
} // namespace ProcessorStruct

#endif
