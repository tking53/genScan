#ifndef __PEAK_FIT_FUNCTIONS_HPP__
#define __PEAK_FIT_FUNCTIONS_HPP__

namespace PeakFit{
	
	//ROOT
	//1D
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
	double SimpleImplantationCurve(double*,double*);
	double ImplantationBatemanStep(double*,double*);
	double SingleDaughterPairImplantationCurve(double*,double*);
	double NGaussN(double*,double*);

	//ROOT
	//2D
	double BiGauss(double*,double*);
	double BiGaussFlatBkg(double*,double*);
	double BiGaussLinXYBkg(double*,double*);

}

#endif
