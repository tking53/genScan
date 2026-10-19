#ifndef __SUPER3HEN_STRUCT_HPP__
#define __SUPER3HEN_STRUCT_HPP__

#include <TString.h>

namespace ProcessorStruct {
    struct S3H {
		TString holeLabel = "";
		TString tubeClass = "";
		TString preamp = "";
		double energy = 0.0;
		double timeStamp = 0.0;
		bool pileup = false;
		bool saturation = false;
    };
    static const S3H DEFAULT_S3H_STRUCT;
} // namespace ProcessorStruct

#endif // __SUPER3HEN_STRUCT_HPP__
    