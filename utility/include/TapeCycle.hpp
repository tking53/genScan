#ifndef  __TAPE_CYCLE_HPP__
#define __TAPE_CYCLE_HPP__

namespace TAPE{
	enum CycleState{
		UNKNOWN,
		TAPEMOVE,
		MEASURE,
		BACKGROUND,
		LIGHTPULSER,
		IRRADIATION
	};
}

#endif
