#include "CommonFitFunctions.hpp"

#include <TMath.h>
#include <cmath>

namespace CommonFit{
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

	double Sin(double* x,double* par){
		double amp = par[0];
		double phase = par[1];
		double freq = par[2];
		return amp*TMath::Sin(freq*(x[0]+phase));
	}

	//root 2d
	double LinXY(double* x,double* par){
		return Linear(x,par) + Linear(x+1,par+2);
	}

	//eigen
	double Sin(double t,double a,double p,double f){
		return a*std::sin(f*(t+p));
	}


}
