#ifndef __PULSE_FIT_FUNCTIONS_HPP__
#define __PULSE_FIT_FUNCTIONS_HPP__ 

namespace PulseFit{

	double Constant(double*,double*);
	double Linear(double*,double*);
	double Pulse(double*,double*);
	double Sin(double*,double*);

	double SingleTraceFit(double*,double*);
	double BSMSingleTraceFit(double*,double*);
	double BSMDoubleTraceFit(double*,double*);
}

#endif
