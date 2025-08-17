#ifndef __PULSE_FIT_FUNCTIONS_HPP__
#define __PULSE_FIT_FUNCTIONS_HPP__ 

namespace PulseFit{

	//root
	double Sin(double*,double*);
	double SingleTraceFit(double*,double*);
	double BSMSingleTraceFit(double*,double*);
	double BSMDoubleTraceFit(double*,double*);

	//technically peaks, need to move out
	double GaussN(double*,double*);
	double Constant(double*,double*);
	double Linear(double*,double*);
	double Quad(double*,double*);
	double Pulse(double*,double*);
	double GaussNLinBkg(double*,double*);
	double GaussErf(double*,double*);
	double GaussNErfBkg(double*,double*);
	double Erf(double*,double*);
	double TailingGaussN(double*,double*);
	double SingleTailingGaussN(double*,double*);
	double DoubleTailingGaussN(double*,double*);
	double SingleTailingGaussNLinBkg(double*,double*);
	double SimpleHalfLife(double*,double*);

	//eigen
	double TraceFunc(double,double,double,double,double);
	double Sin(double,double,double,double);
	double sintracefunc(double,double,double,double,double,double,double,double,double);
	double tracefunc(double,double,double,double,double,double);


}

#endif
