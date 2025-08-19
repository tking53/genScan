#include "CommonFitFunctions.hpp"
#include "PulseFitFunctions.hpp"

#include <TMath.h>
#include <cmath>

namespace PulseFit{
	//root
	double Pulse(double* x,double* par){
		double amp = par[0];
		double t0 = par[1];
		double tr = par[2];
		double tf = par[3];
		return amp*(1.0/(TMath::Exp(-(x[0]-t0)/tr) + 1.0))*(1.0/(TMath::Exp((x[0]-t0)/tf)+1.0));
	}

	double SingleTraceFit(double* x,double* par){
		double c = CommonFit::Constant(x,par);
		double pulse = PulseFit::Pulse(x,par+1);
		return c + pulse;
	}

	double BSMSingleTraceFit(double* x,double* par){
		double c = CommonFit::Constant(x,par);
		double pulse = PulseFit::Pulse(x,par+1);
		double sine = CommonFit::Sin(x,par+5);
		return c + sine + pulse;
	}

	double BSMDoubleTraceFit(double* x,double* par){
		return CommonFit::Constant(x,par)+CommonFit::Sin(x,par+1)+PulseFit::Pulse(x,par+3)+PulseFit::Pulse(x,par+7);
	}

	//eigen
	double TraceFunc(double t,double a,double d,double r,double f){
		return a*((1.0/(std::exp(-(t-d)/r)+1.0))*(1.0/(std::exp((t-d)/f)+1.0)));
	}

	double sintracefunc(double t,double c,double pa,double pd,double pr,double pf,double sa,double sp,double sf){
		double SinVal = CommonFit::Sin(t,sa,sp,sf);
		double PulseVal = TraceFunc(t,pa,pd,pr,pf);
		return c + SinVal + PulseVal;
	}

	double tracefunc(double t,double c,double pa,double pd,double pr,double pf){
		double PulseVal = TraceFunc(t,pa,pd,pr,pf);
		return c + PulseVal;
	}
}
