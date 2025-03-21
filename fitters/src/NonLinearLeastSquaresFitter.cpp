#include "NonLinearLeastSquaresFitter.hpp"

#include <cmath>

double Sin(double t,double a,double p,double f){
	return a*std::sin(f*(t+p));
}

double TraceFunc(double t,double a,double d,double r,double f){
	return a*((1.0/(std::exp(-(t-d)/r)+1.0))*(1.0/(std::exp((t-d)/f)+1.0)));
}

double sintracefunc(double t,double c,double sa,double sp,double sf,double pa,double pd,double pr,double pf){
	double SinVal = Sin(t,sa,sp,sf);
	double PulseVal = TraceFunc(t,pa,pd,pr,pf);
	return c + SinVal + PulseVal;
}

double tracefunc(double t,double c,double pa,double pd,double pr,double pf){
	double PulseVal = TraceFunc(t,pa,pd,pr,pf);
	return c + PulseVal;
}
