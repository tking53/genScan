#ifndef __CALIBRATION_HPP__
#define __CALIBRATION_HPP__

#include <string>
#include <vector>

class Calibration{
	public:
		Calibration();
		Calibration(std::string);
		Calibration(std::string,std::vector<double>&);
		virtual double Calibrate(double&);
	protected:
		std::string CalibrationName;
		std::vector<double> Params;
};

#endif
