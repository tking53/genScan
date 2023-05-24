#ifndef __QUADRATICEXPO_CALIBRATION_HPP__
#define __QUADRATICEXPO_CALIBRATION_HPP__

#include <vector>

#include "Calibration.hpp"

class QuadraticExpoCalibration : public Calibration{
	public:
		QuadraticExpoCalibration(std::vector<double>&);
		double Calibrate(double&) final;
};

#endif
