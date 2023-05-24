#ifndef __POLY_CALIBRATION_HPP__
#define __POLY_CALIBRATION_HPP__

#include <vector>

#include "Calibration.hpp"

class PolyCalibration : public Calibration{
	public:
		PolyCalibration(std::vector<double>&);
		double Calibrate(double&) final;
};

#endif
