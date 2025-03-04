#ifndef __BSM_TRACE_FIT_STRUCT_HPP__
#define __BSM_TRACE_FIT_STRUCT_HPP__

namespace ProcessorStruct{
	struct BSMTraceFit{
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
		double energy = -999.0;
		double timestamp = -999.0;
	};
	static const BSMTraceFit DEFAULT_BSM_TRACE_FIT_STRUCT;
}

#endif
