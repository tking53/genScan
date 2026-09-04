#ifndef __DSSDSTRUCT_HPP__
#define __DSSDSTRUCT_HPP__

namespace ProcessorStruct {
	struct STRIP {
		double energy = 0.0;
		double time = 0.0;
		int stripnum = -1;

	};
	static const STRIP DEFAULT_STRIP_STRUCT;
    
    struct DSSD {
        STRIP front = DEFAULT_STRIP_STRUCT;
        STRIP back = DEFAULT_STRIP_STRUCT;
    };
    static const DSSD DEFAULT_DSSD_STRUCT;

} // namespace ProcessorStruct
#endif // !__DSSDSTRUCT_HPP__
