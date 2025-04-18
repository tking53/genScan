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
		};
}


#endif
