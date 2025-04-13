#include "PulseFitFunctions.hpp"

#include <TMath.h>
#include <cmath>

namespace PulseFit{
	//root
	double Constant(double* x,double* par){
		return par[0];
	}

	double Linear(double* x,double* par){
		return par[0] + par[1]*x[0];
	}

	double Quad(double* x,double* par){
		return par[0] + par[1]*x[0] + par[2]*x[0]*x[0];
	}

	double GaussN(double* x,double* par){
		double norm = par[0];
		double arg = 0.0;
		if( par[2] != 0 ){
			arg = (x[0] - par[1])/par[2];
			norm = par[0]/(par[2]*TMath::Sqrt(TMath::Pi()));
		}

		double fitval = norm*TMath::Exp(-0.5*arg*arg);
		return fitval;
	}

	double GaussNLinBkg(double* x,double* par){
		double fitval = GaussN(x,par) + Linear(x,par+3);
		return fitval;
	}

	double GaussErf(double* x, double* par){
		double arg = 0.0;
		double norm = par[0];
		if( par[2] != 0 ){
			arg = (x[0] - par[1])/par[2];
			norm = par[0]/(par[2]*TMath::Sqrt(TMath::Pi()));
		}
		double fitval = norm*(TMath::Erfc(arg));
		return fitval;
	}

	double GaussNErfBkg(double* x,double* par){
		//have to calc by hand because we can't transform the params to work without causing memory issues
		double arg = 0.0;
		double norm = par[3];
		if( par[2] != 0 ){
			arg = (x[0] - par[1])/par[2];
			norm = par[3]/(par[2]*TMath::Sqrt(TMath::Pi()));
		}
		double ce = norm*(TMath::Erfc(arg));
		
		auto gaussn = GaussN(x,par);
		auto bkg = Linear(x,par+4);
		return gaussn + ce + bkg;
	}
	
	double SingleTailingGaussN(double* x,double* par){
		double A = par[0];
		double mu = par[1];
		double sigma = par[2];
		double tau = par[3];

		double mean = x[0] - mu;
		double c = A;
		double t1 = 1.0;
		double t2 = 1.0;
		if( tau != 0.0 ){
			c = A/(2.0*tau);
			t1 = TMath::Exp((mean/tau) + ((sigma*sigma)/(2.0*tau*tau)));
			if( sigma != 0.0 ){
				t2 = TMath::Erfc((1.0/TMath::Sqrt(2.0))*((mean/sigma) + (sigma/tau)));
			}
		}
		return c*t1*t2;
	}

	double DoubleTailingGaussN(double* x,double* par){
		return SingleTailingGaussN(x,par) + SingleTailingGaussN(x,par+4);
	}

	double Pulse(double* x,double* par){
		double amp = par[0];
		double t0 = par[1];
		double tr = par[2];
		double tf = par[3];
		return amp*(1.0/(TMath::Exp(-(x[0]-t0)/tr) + 1.0))*(1.0/(TMath::Exp((x[0]-t0)/tf)+1.0));
	}

	double Sin(double* x,double* par){
		double amp = par[0];
		double phase = par[1];
		double freq = par[2];
		return amp*TMath::Sin(freq*(x[0]+phase));
	}

	double SingleTraceFit(double* x,double* par){
		double c = PulseFit::Constant(x,par);
		double pulse = PulseFit::Pulse(x,par+1);
		return c + pulse;
	}

	double BSMSingleTraceFit(double* x,double* par){
		double c = PulseFit::Constant(x,par);
		double pulse = PulseFit::Pulse(x,par+1);
		double sine = PulseFit::Sin(x,par+5);
		return c + sine + pulse;
	}

	double BSMDoubleTraceFit(double* x,double* par){
		return PulseFit::Constant(x,par)+PulseFit::Sin(x,par+1)+PulseFit::Pulse(x,par+3)+PulseFit::Pulse(x,par+7);
	}

	//eigen
	double Sin(double t,double a,double p,double f){
		return a*std::sin(f*(t+p));
	}

	double TraceFunc(double t,double a,double d,double r,double f){
		return a*((1.0/(std::exp(-(t-d)/r)+1.0))*(1.0/(std::exp((t-d)/f)+1.0)));
	}

	double sintracefunc(double t,double c,double pa,double pd,double pr,double pf,double sa,double sp,double sf){
		double SinVal = Sin(t,sa,sp,sf);
		double PulseVal = TraceFunc(t,pa,pd,pr,pf);
		return c + SinVal + PulseVal;
	}

	double tracefunc(double t,double c,double pa,double pd,double pr,double pf){
		double PulseVal = TraceFunc(t,pa,pd,pr,pf);
		return c + PulseVal;
	}


}
