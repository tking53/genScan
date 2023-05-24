#include "CubicCalibration.hpp"

CubicCalibration::CubicCalibration(std::vector<double>& p) : Calibration("Cubic",p){
}

double CubicCalibration::Calibrate(double& erg){
	return Params.at(0) + Params.at(1)*erg + Params.at(2)*erg*erg + Params.at(3)*erg*erg*erg;
}
