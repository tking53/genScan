#ifndef __PULSE_FIT_FUNCTIONS_HPP__
#define __PULSE_FIT_FUNCTIONS_HPP__ 

namespace PulseFit{

	//root
	double Constant(double*,double*);
	double Linear(double*,double*);
	double Pulse(double*,double*);
	double Sin(double*,double*);
	double SingleTraceFit(double*,double*);
	double BSMSingleTraceFit(double*,double*);
	double BSMDoubleTraceFit(double*,double*);

	//eigen
	double TraceFunc(double,double,double,double,double);
	double Sin(double,double,double,double);
	double sintracefunc(double,double,double,double,double,double,double,double,double);
	double tracefunc(double,double,double,double,double,double);


}

#endif
