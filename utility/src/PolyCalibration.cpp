#include <cmath>

#include "PolyCalibration.hpp"

PolyCalibration::PolyCalibration(std::vector<double>& p) : Calibration("Poly",p){
}

double PolyCalibration::Calibrate(double& erg){
	double sum =  Params.at(0) + Params.at(1)*erg + Params.at(2)*erg*erg + Params.at(3)*erg*erg*erg;
	for( size_t ii = 4; ii < Params.size(); ++ii ){
		sum += Params.at(ii)*std::pow(erg,ii);
	}	
	return sum;
}
