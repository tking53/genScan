#ifndef __CUBIC_CALIBRATION_HPP__
#define __CUBIC_CALIBRATION_HPP__

#include <vector>

#include "Calibration.hpp"

class CubicCalibration : public Calibration{
	public:
		CubicCalibration(std::vector<double>&);
		double Calibrate(double&) final;
};

#endif
