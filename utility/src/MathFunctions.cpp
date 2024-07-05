#include "MathFunctions.hpp"

namespace Math{
	//calculate angle between two points relative to a common center
	double CalcAngleBetweenTwoPoints(double x1, double y1, double x2, double y2){
		return std::acos(-1.0*(y1*y2 + x1*x2)/(std::sqrt(x1*x1+y1*y1)*std::sqrt(x2*x2+y2*y2)));
	}
}
