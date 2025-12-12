#ifndef __PULSE_FIT_FUNCTIONS_HPP__
#define __PULSE_FIT_FUNCTIONS_HPP__ 

namespace PulseFit{

	//root
	double Pulse(double*,double*);
	double SingleTraceFit(double*,double*);
	double DoubleTraceFit(double*,double*);
	double BSMSingleTraceFit(double*,double*);
	double BSMDoubleTraceFit(double*,double*);

	//eigen
	double TraceFunc(double,double,double,double,double);
	double sintracefunc(double,double,double,double,double,double,double,double,double);
	double tracefunc(double,double,double,double,double,double);
}

#endif
