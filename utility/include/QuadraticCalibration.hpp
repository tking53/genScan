#ifndef __QUADRATIC_CALIBRATION_HPP__
#define __QUADRATIC_CALIBRATION_HPP__

#include <vector>

#include "Calibration.hpp"

class QuadraticCalibration : public Calibration{
	public:
		QuadraticCalibration(std::vector<double>&);
		double Calibrate(double&) final;
};

#endif
