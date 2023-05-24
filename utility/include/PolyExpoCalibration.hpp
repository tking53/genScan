#ifndef __POLYEXPO_CALIBRATION_HPP__
#define __POLYEXPO_CALIBRATION_HPP__

#include <vector>

#include "Calibration.hpp"

class PolyExpoCalibration : public Calibration{
	public:
		PolyExpoCalibration(std::vector<double>&);
		double Calibrate(double&) final;
};

#endif
