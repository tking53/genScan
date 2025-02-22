#ifndef __GEOMETRY_HPP__
#define __GEOMETRY_HPP__

#include <utility>
namespace Geometry{
	struct hexagon{
		hexagon(double,double,double,double);
		std::pair<double,double> center;
		std::pair<double,double> dim;
		double xcoords[6];
		double ycoords[6];
		std::pair<double,double> GetFlatHexagonCorner(double,double,double,double,int);
	};
}

#endif
