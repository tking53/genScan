#ifndef __LINEAR_CALIBRATION_HPP__
#define __LINEAR_CALIBRATION_HPP__

#include <vector>

#include "Calibration.hpp"

class LinearCalibration : public Calibration{
	public:
		LinearCalibration(std::vector<double>&);
		double Calibrate(double&) final;
};

#endif
