#ifndef __IMAGE_MANIPULATION_HPP__
#define __IMAGE_MANIPULATION_HPP__

#include <utility>

namespace PSPMT {
	
	struct corner {
		double energy;
		bool saturation;
		bool pileup;
	};
	struct Image {
		double anodesum;
		int numanodes;
		std::pair<double, double> position;
		double DynodeTimeStamp;
		corner xa, xb, ya, yb, dynode;
		
		void ResetDynode(double val = 0.0, double ts = 0.0) {
			dynode.energy = val;
			dynode.saturation = false;
			dynode.pileup = false;
			DynodeTimeStamp = ts;
		}
		
		void ResetAnode(double val = 0.0, int n = 0) {
			anodesum = val;
			numanodes = n;
		}
		
		void ResetPosition(double x = 0.0, double y = 0.0) {
			position.first = x;
			position.second = y;
		}
		
		void ResetCorners(double xap = 0.0, double xbp = 0.0, double yap = 0.0, double ybp = 0.0) {
			xa.energy = xap;
			xb.energy = xbp;
			ya.energy = yap;
			yb.energy = ybp;
			xa.saturation = false;
			xb.saturation = false;
			ya.saturation = false;
			yb.saturation = false;
			xa.pileup = false;
			xb.pileup = false;
			ya.pileup = false;
			yb.pileup = false;
		}
		void ResetAnodeSum(double sum=0.0) {
			anodesum = sum;
		}
	};
} // namespace PSPMT

namespace SIPMIMP {
	struct Image {
		double dynode;
		double DynodeTimeStamp;
		std::pair<int, int> lowResPosition;
		std::pair<int, int> secondarylowResPosition;
		std::pair<double, double> highResPosition;
		std::pair<double, double> highResStdDev;
		double anodesum;
		int numanodes;
		
		void ResetDynode(double val = 0.0, double ts = 0.0) {
			dynode = val;
			DynodeTimeStamp = ts;
		}
		
		void ResetAnode(double val = 0.0, int nanodes = 0) {
			anodesum = val;
			numanodes = nanodes;
		}
		
		void ResetHighResPosition(double x = 0.0, double y = 0.0) {
			highResPosition.first = x;
			highResPosition.second = y;
		}
		void ResetLowResPosition(int x = -1, int y = -1) {
			lowResPosition.first = x;
			lowResPosition.second = y;
		}
		void ResetSecondaryLowResPosition(int x = -1, int y = -1) {
			secondarylowResPosition.first = x;
			secondarylowResPosition.second = y;
		}
		void ResetHighResStdDev(double x = 0.0, double y = 0.0) {
			highResStdDev.first = x;
			highResStdDev.second = y;
		}
	};
	
} // namespace SIPMIMP
#endif
