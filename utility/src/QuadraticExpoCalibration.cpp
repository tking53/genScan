#include <cmath>

#include "QuadraticExpoCalibration.hpp"

QuadraticExpoCalibration::QuadraticExpoCalibration(std::vector<double>& p) : Calibration("QuadraticExpo",p){
}

double QuadraticExpoCalibration::Calibrate(double& erg){
	double sum = Params.at(0) + Params.at(1)*erg + Params.at(2)*erg*erg;
	return std::exp(sum);
}
