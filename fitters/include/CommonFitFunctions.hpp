#ifndef __COMMON_FIT_FUNCTIONS_HPP__
#define __COMMON_FIT_FUNCTIONS_HPP__

namespace CommonFit {
	// root
	double Constant(double*, double*);
	double Linear(double*, double*);
	double Quad(double*, double*);
	double Sin(double*, double*);

	// 2d root
	double LinXY(double*, double*);

	// eigen
	double Sin(double, double, double, double);
} // namespace CommonFit

#endif
