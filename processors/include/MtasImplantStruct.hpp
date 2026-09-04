#ifndef __MTAS_IMPLANT_STRUCT_HPP__
#define __MTAS_IMPLANT_STRUCT_HPP__

namespace ProcessorStruct {
	struct MtasImplant {
		double highresx = -999.0;
		double highresy = -999.0;
		int lowresx = -999;
		int lowresy = -999;
		double dynodeerg = 0.0;
		double dynodets = -1.0;
		double anodesum = 0.0;
		int numanodes = 0;
	};
	static const MtasImplant DEFAULT_MTAS_IMPLANT_STRUCT;
} // namespace ProcessorStruct

#endif
