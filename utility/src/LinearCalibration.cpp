#include "LinearCalibration.hpp"

LinearCalibration::LinearCalibration(std::vector<double>& p) : Calibration("Linear",p){
}

double LinearCalibration::Calibrate(double& erg){
	return Params.at(0) + Params.at(1)*erg;
}
