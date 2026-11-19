#ifndef __BSM_STRUCT_HPP__
#define __BSM_STRUCT_HPP__

#include <vector>

namespace ProcessorStruct {
	struct BSMTraceFit {
		double constant = -999.0;
		double sinamp = -999.0;
		double sinphase = -999.0;
		double sinfreq = -999.0;
		double pulseamp = -999.0;
		double pulsedelay = -999.0;
		double pulserise = -999.0;
		double pulsedecay = -999.0;
		double chi2 = -999.0;
		double ndf = -999.0;
	};
	static const BSMTraceFit DEFAULT_BSM_TRACE_FIT_STRUCT;

	struct BSMSingle {
		int pmtid = -1;
		double rawEnergy = -999;
		double energy = -999;
		double time = -999;
		bool pileup = false;
		bool saturation = false;
		std::vector<unsigned int> trace = {};
	};
	static const BSMSingle DEFAULT_BSM_SINGLE_STRUCT;

	struct BSMSegment {
		double frontenergy = -999.0;
		double fronttimestamp = -999.0;

		double backenergy = -999.0;
		double backtimestamp = -999.0;

		double sumenergy = -999.0;
		double avgtimestamp = -999.0;
	};
	static const BSMSegment DEFAULT_BSM_SEGMENT_STRUCT;

	struct BSMTotal {
		double timestamp = -999.0;
		double sumenergy = -999.0;
		int numfire = 0;
		bool saturate = false;
		bool pileup = false;
	};
	static const BSMTotal DEFAULT_BSM_TOTAL_STRUCT;
} // namespace ProcessorStruct

#endif
