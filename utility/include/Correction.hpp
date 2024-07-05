#ifndef __CORRECTION_HPP__
#define __CORRECTION_HPP__ 

namespace Correction{
		struct ExpoPosCorrection{
			double constant;
			double slope;
			double mean;

			double Correct(const double&,const double&);

			ExpoPosCorrection();
			~ExpoPosCorrection() = default;
			//ExpoPosCorrection(const ExpoPosCorrection&) = default;
			//ExpoPosCorrection(ExpoPosCorrection&&) = default;
			//ExpoPosCorrection& operator=(const ExpoPosCorrection&) = default;
			//ExpoPosCorrection& operator=(ExpoPosCorrection&&) = default;
		};
}

#endif
