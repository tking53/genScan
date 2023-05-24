#ifndef __LINEAREXPO_CALIBRATION_HPP__
#define __LINEAREXPO_CALIBRATION_HPP__

#include <vector>

#include "Calibration.hpp"

class LinearExpoCalibration : public Calibration{
	public:
		LinearExpoCalibration(std::vector<double>&);
		double Calibrate(double&) final;
};

#endif
