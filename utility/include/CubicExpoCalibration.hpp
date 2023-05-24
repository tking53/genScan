#ifndef __CUBICEXPO_CALIBRATION_HPP__
#define __CUBICEXPO_CALIBRATION_HPP__

#include <vector>

#include "Calibration.hpp"

class CubicExpoCalibration : public Calibration{
	public:
		CubicExpoCalibration(std::vector<double>&);
		double Calibrate(double&) final;
};

#endif
