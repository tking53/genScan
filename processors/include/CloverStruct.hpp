#ifndef __CLOVERSTRUCT_HPP__
#define __CLOVERSTRUCT_HPP__

#include <vector>
namespace ProcessorStruct {
	struct Leaf {
		double energy = 0.0;
		double time = 0.0;
		double theta = -999.0;
		double phi = -999.0;
		bool pileup = false;
		bool saturation = false;
	};
	static const Leaf DEFAULT_LEAF_STRUCT;

	struct Clover {
		Leaf U = DEFAULT_LEAF_STRUCT;
		Leaf B = DEFAULT_LEAF_STRUCT;
		Leaf G = DEFAULT_LEAF_STRUCT;
		Leaf R = DEFAULT_LEAF_STRUCT;

		double length = -999.0;
		double addbackEn = 0.0;
		std::pair<double, double> addbackTS = {-999.0, -999.0};
	};
	static const Clover DEFAULT_CLOVER_STRUCT;

} // namespace ProcessorStruct
#endif // !__CLOVERSTRUCT_HPP__
