#include "QuadraticCalibration.hpp"

QuadraticCalibration::QuadraticCalibration(std::vector<double>& p) : Calibration("Quadratic",p){
}

double QuadraticCalibration::Calibrate(double& erg){
	return Params.at(0) + Params.at(1)*erg + Params.at(2)*erg*erg;
}
