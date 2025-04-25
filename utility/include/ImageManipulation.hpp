#ifndef __IMAGE_MANIPULATION_HPP__
#define __IMAGE_MANIPULATION_HPP__

#include <utility>

namespace PSPMT{
		struct Image{
			double dynode;
			double xa;
			double xb;
			double ya;
			double yb;
			double anodesum;
			int numanodes;
			std::pair<double,double> position;
			double DynodeTimeStamp;

			void ResetDynode(double val = 0.0,double ts = 0.0){
				dynode = val;
				DynodeTimeStamp = ts;
			}

			void ResetAnode(double val = 0.0,int n = 0){
				anodesum = val;
				numanodes = n;
			}

			void ResetPosition(double x = 0.0,double y = 0.0){
				position.first = x;
				position.second = y;
			}

			void ResetCorners(double xap = 0.0,double xbp = 0.0,double yap = 0.0,double ybp = 0.0){
				xa = xap;
				xb = xbp;
				ya = yap;
				yb = ybp;
			}
		};
}


#endif
