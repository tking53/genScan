#include "Geometry.hpp"

#include <cmath>

namespace Geometry{
	hexagon::hexagon(double xc,double yc,double sz,double pd){
		this->center = {xc,yc};
		this->dim = {sz,pd};
		for( size_t ii = 0; ii < 6; ++ii ){
			auto corner = this->GetFlatHexagonCorner(xc,yc,sz,pd,ii);
			xcoords[ii] = corner.first;
			ycoords[ii] = corner.second;	
		}
	}

	std::pair<double,double> hexagon::GetFlatHexagonCorner(double cx,double cy,double sz,double pd,int ii){
		double angle_rad = (4.0*std::atan(1.0))*(ii*60.0/180.0);
		return std::pair<double,double>(cx + sz*pd*std::cos(angle_rad),cy + sz*pd*std::sin(angle_rad));
	}


}
