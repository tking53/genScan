#include "CommonFitFunctions.hpp"
#include "PeakFitFunctions.hpp"

#include <TMath.h>

namespace PeakFit{
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
		double fitval = GaussN(x,par) + CommonFit::Linear(x,par+3);
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
		auto bkg = CommonFit::Linear(x,par+4);
		return gaussn + ce + bkg;
	}
	
	double Erf(double* x,double* par){
		//have to calc by hand because we can't transform the params to work without causing memory issues
		double arg = 0.0;
		double norm = par[3];
		if( par[2] != 0 ){
			arg = (x[0] - par[1])/par[2];
			norm = par[3]/(par[2]*TMath::Sqrt(TMath::Pi()));
		}
		double ce = norm*(TMath::Erfc(arg));
		return ce;
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

	double SingleTailingGaussNLinBkg(double* x,double* par){
		return SingleTailingGaussN(x,par) + CommonFit::Linear(x,par+4);
	}

	double DoubleTailingGaussN(double* x,double* par){
		return SingleTailingGaussN(x,par) + SingleTailingGaussN(x,par+4);
	}

	double SimpleHalfLife(double* x,double* par){
		double arg = 0.0;
		if( TMath::Abs(par[1]) > 0.0 ){
			arg = -(TMath::Log(2.0)/par[1])*x[0];
		}
		return (par[0]/par[1])*TMath::Exp(arg);
	}

	double SimpleImplantationCurve(double* x, double* par){
		if( x[0] < 0 ){
			return par[0];
		}else{
			return par[0] + SimpleHalfLife(x,par+1);
		}
	}

	double ImplantationBatemanStep(double* x,double* par){
		if( x[0] < 0 ){
			return par[0];
		}else{
			double lambda1 = TMath::Log(2.0)/par[2];
			double lambda2 = TMath::Log(2.0)/par[3];
			double halfliferatio = lambda1/(lambda2 - lambda1);
			return par[0] + halfliferatio*par[1]*(TMath::Exp(-lambda1*x[0]) - TMath::Exp(-lambda2*x[0]));
		}
	}
	
	double SingleDaughterPairImplantationCurve(double* x,double* par){
		//par[0] -> Constant
		//par[1,2] -> SimpleHalfLife
		//par[3,4,5] -> Pn, Beta-daughter-HalfLife, Beta-n-daughter-HalfLife 
		auto bkg = par[0];

		auto primary = SimpleHalfLife(x,par+1);

		double beta_par[] = {0.0,(1.0-par[3])*par[1],par[2],par[4]};
		double beta_daughter = ImplantationBatemanStep(x,beta_par);

		double beta_n_par[] = {0.0,par[3]*par[1],par[2],par[5]};
		double beta_n_daugther = ImplantationBatemanStep(x,beta_n_par);

		if( x[0] < 0 ){
			return bkg;
		}else{
			return bkg + primary + beta_daughter + beta_n_daugther;
		}
	}

	double BiGauss(double* x,double* par){
		double xval = x[0];
		double yval = x[1];

		double Amp = par[0];
		double XCentroid = par[1];
		double XSigma = par[2];
		double YCentroid = par[3];
		double YSigma = par[4];
		double Corr = par[5];

		double u = (xval - XCentroid)/XSigma;
		double v = (yval - YCentroid)/YSigma;
		double c = 1.0 - Corr*Corr;
		double z = u*u - 2.0*Corr*u*v + v*v;
		return Amp*TMath::Exp(-z/(2.0*c));
	}
}
