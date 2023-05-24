#include <cmath>

#include "CubicExpoCalibration.hpp"

CubicExpoCalibration::CubicExpoCalibration(std::vector<double>& p) : Calibration("CubicExpo",p){
}

double CubicExpoCalibration::Calibrate(double& erg){
	double sum = Params.at(0) + Params.at(1)*erg + Params.at(2)*erg*erg + Params.at(3)*erg*erg*erg;
	return std::exp(sum);
}
