#include "Correction.hpp"

#include <cmath>

namespace Correction{
	ExpoPosCorrection::ExpoPosCorrection(){
		constant = 0.0;
		slope = 0.0;
		mean = 1.0;
	}

	double ExpoPosCorrection::Correct(const double& erg,const double& pos){
		double val = mean/std::exp(constant + slope*pos);
		return val*erg;
	}
}
