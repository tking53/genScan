#ifndef __EBSS_STRUCT_HPP__
#define __EBSS_STRUCT_HPP__

namespace ProcessorStruct {
	struct EBSSPaddle {
		double timestamp = -999.0;
		double energy = -999.0;
		int paddle_id = -1;
		bool saturate = false;
		bool pileup = false;
	};
	static const EBSSPaddle DEFAULT_EBSS_PADDLE_STRUCT;

	struct EBSStotal {
		double sumenergy = -999.0;
		int multiplicy = -1;
	};
	static const EBSStotal DEFAULT_EBSS_TOTAL_STRUCT;
} // namespace ProcessorStruct

#endif
