#include <stdexcept>

#include "Calibration.hpp"

Calibration::Calibration() : CalibrationName("Calibration"), Params({}){
}

Calibration::Calibration(std::string name) : CalibrationName(name),Params({}){
}

Calibration::Calibration(std::string name,std::vector<double>& p):CalibrationName(name),Params({}){
}

double Calibration::Calibrate(double& erg){
	throw std::runtime_error("Called Calibration::Calibrate(double& erg)");
}
