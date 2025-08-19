#ifndef __PEAK_FIT_FUNCTIONS_HPP__
#define __PEAK_FIT_FUNCTIONS_HPP__

namespace PeakFit{
	
	//technically peaks, need to move out
	double GaussN(double*,double*);
	double GaussNLinBkg(double*,double*);
	double GaussErf(double*,double*);
	double GaussNErfBkg(double*,double*);
	double Erf(double*,double*);
	double TailingGaussN(double*,double*);
	double SingleTailingGaussN(double*,double*);
	double DoubleTailingGaussN(double*,double*);
	double SingleTailingGaussNLinBkg(double*,double*);
	double SimpleHalfLife(double*,double*);


}

#endif
