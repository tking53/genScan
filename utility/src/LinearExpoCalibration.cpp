#include <cmath>

#include "LinearExpoCalibration.hpp"

LinearExpoCalibration::LinearExpoCalibration(std::vector<double>& p) : Calibration("LinearExpo",p){
}

double LinearExpoCalibration::Calibrate(double& erg){
	double sum =  Params.at(0) + Params.at(1)*erg;
	return std::exp(sum);
}
