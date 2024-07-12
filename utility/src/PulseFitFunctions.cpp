#include <TMath.h>

#include "PulseFitFunctions.hpp"

namespace PulseFit{
	double Constant(double* x,double* par){
		return par[0];
	}

	double Linear(double* x,double* par){
		return par[0] + par[1]*x[0];
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
		return amp*TMath::Sin(x[0]+phase);
	}
}
