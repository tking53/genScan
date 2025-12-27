#ifndef __PEAK_FITTER_HPP__
#define __PEAK_FITTER_HPP__

#include <Rtypes.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>
#include <map>

#include "TF1.h"
#include "TF2.h"
#include "TH1.h"
#include "TH2.h"
#include "TList.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TLine.h"
#include "TCutG.h"
#include "TArrow.h"
#include <TMath.h>
#include <TPad.h>
#include <TMatrixDSym.h>
#include <TMatrixDSymEigen.h>
#include <TVectorD.h>

#include "CommonFitFunctions.hpp"
#include "PeakFitFunctions.hpp"
#include "PulseFitFunctions.hpp"

#include "StringManipFunctions.hpp"

#include <boost/describe.hpp>

namespace FitTypes{
	enum OneDim : int {
		GaussNLinBkgFit = 1,
		GaussNErfBkgFit = 2,
		SingleTailingGaussNFit = 10,
		SingleTailingGaussNLinBkgFit = 11,
		DoubleTailingGaussNFit = 20,
		ErfFit = 30,
		NGaussNFit = 40,
		NGaussNLinBkgFit = 41,
		NGaussNErfBkgFit = 42,
		SinglePlasticTrace = 200,
		DoublePlasticTrace = 210,
		SimpleImplantationCurveFit = 500,
		SingleDaughterImplantationCurveFit = 510
	};
	BOOST_DESCRIBE_ENUM(OneDim,
			GaussNLinBkgFit,GaussNErfBkgFit,
			SingleTailingGaussNFit,SingleTailingGaussNLinBkgFit,
			DoubleTailingGaussNFit,
			ErfFit,
			NGaussNFit,
			NGaussNLinBkgFit,
			NGaussNErfBkgFit,
			SinglePlasticTrace,
			DoublePlasticTrace,
			SimpleImplantationCurveFit,
			SingleDaughterImplantationCurveFit
			)
	enum TwoDim : int {
		BiGauss = 1000,
		BiGaussFlatBkg = 1001,
		BiGaussLinXYBkg = 1002,
	};
	BOOST_DESCRIBE_ENUM(TwoDim,
			BiGauss, BiGaussFlatBkg, BiGaussLinXYBkg
			)
};

struct PeakFitter{
	bool loglikelihood;
	bool debug;
	std::map<std::string,double> fvalues;
	std::map<std::string,std::pair<double,double>> bvalues;
	std::map<std::string,std::pair<int,double>> keys;
	std::map<std::string,double> Results;
	std::map<std::string,double> Errors;

	PeakFitter(bool chi2,bool dbg,const std::map<std::string,double>& fixedvalues,const std::map<std::string,std::pair<double,double>>& boundedvalues) 
		: loglikelihood(!chi2),debug(dbg),fvalues(fixedvalues),bvalues(boundedvalues)
	{
	}
	
	virtual ~PeakFitter() = default;

	PeakFitter(const PeakFitter&) = default;
	PeakFitter(PeakFitter&&) = default;
	PeakFitter& operator=(const PeakFitter&) = default;
	PeakFitter& operator=(PeakFitter&&) = default;

	virtual const std::map<std::string,std::pair<int,double>>& GetKeys() const final{
		return this->keys;
	}

	virtual const std::map<std::string,double>& GetFValues() const final{
		return this->fvalues;
	}

	virtual const std::map<std::string,std::pair<double,double>>& GetBValues() const final{
		return this->bvalues;
	}

	virtual void FixAndBoundParameters(){
	}
	
	virtual void VerifyBoundedValues() final{
		for( const auto& kv : this->bvalues ){
			if( this->keys.find(kv.first) == this->keys.end() ){
				throw std::runtime_error("Unknown parameter name "+kv.first);
			}
			if( kv.second.second < kv.second.first ){
				throw std::runtime_error("Bounded parameter has lowerbound higher than upperbound");
			}
		}
	}

	virtual void VerifyFixedValues() final{
		for( const auto& kv : this->fvalues ){
			if( this->keys.find(kv.first) == this->keys.end() ){
				throw std::runtime_error("Unknown parameter name "+kv.first);
			}
		}
	}

	virtual void AssignFitFuncParams(){
	}

	virtual std::pair<double,double> operator [](const std::string& key) const final{
		return {this->Results.at(key),this->Errors.at(key)};
	}
	
	template<typename OStream>
	friend OStream& operator<<(OStream& os, const PeakFitter& fitinfo) {
		for( const auto& kv : fitinfo.keys ){
			os << kv.first << " : " << fitinfo.Results.at(kv.first) << " +- " << fitinfo.Errors.at(kv.first) << " \t ";
		}
		os << "Chi2/NDF : " << fitinfo.Results.at("Chi2") << "/" << fitinfo.Errors.at("NDF");
		return os;
	}

};

struct PeakFitter2D : public PeakFitter{
	std::pair<double,double> XFitRange;
	std::pair<double,double> YFitRange;
	TH2* fithist;
	TF2* fitfunc;
	std::vector<TF2*> components;
	static constexpr auto twod = describe_enumerators_as_array<FitTypes::TwoDim>();
	std::string fitname;

	const std::string GetHisName() const{
		return std::string(this->fithist->GetName());
	}

	PeakFitter2D(double xl,double xu,double yl,double yu,bool chi2,bool dbg,int mode,TH2* hist,
			const std::map<std::string,double>& fixedvalues,const std::map<std::string,std::pair<double,double>>& boundedvalues,double ellipse,int npts) 
		: PeakFitter(chi2,dbg,fixedvalues,boundedvalues), XFitRange(xl,xu), YFitRange(yl,yu), fithist(hist){
		for( const auto& kv : fvalues ){
			if( bvalues.find(kv.first) != bvalues.end() ){
				throw std::runtime_error("Parameter is both fixed and bounded");
			}
		}

		for( const auto& x : twod ){
			if( x.value == mode ){
				this->fitname = std::string(x.name);
			}
		}

		if( mode == FitTypes::TwoDim::BiGauss ){
			this->InitBiGaussFit(ellipse,npts);
		} else if ( mode == FitTypes::TwoDim::BiGaussFlatBkg ){
			this->InitBiGaussFlatBkgFit(ellipse,npts);
		} else if ( mode == FitTypes::TwoDim::BiGaussLinXYBkg ){
			this->InitBiGaussLinXYBkgFit(ellipse,npts);
		}else{
			throw std::runtime_error("Unknown peak fitting mode");
		}
	}

	void InitBiGaussFit(double ellipse,int npts){
		this->fitfunc = new TF2("BiGauss",&PeakFit::BiGauss,XFitRange.first,XFitRange.second,YFitRange.first,YFitRange.second,6);
		this->components = {
		};

		double xwidth = this->XFitRange.second - this->XFitRange.first;
		double ywidth = this->YFitRange.second - this->YFitRange.first;
		double xoffset = (this->XFitRange.second + this->XFitRange.first)/2.0;
		double yoffset = (this->YFitRange.second + this->YFitRange.first)/2.0;
		double xlow = this->fithist->GetXaxis()->FindBin(this->XFitRange.first);
		double xhigh = this->fithist->GetXaxis()->FindBin(this->XFitRange.second);
		double ylow = this->fithist->GetYaxis()->FindBin(this->YFitRange.first);
		double yhigh = this->fithist->GetYaxis()->FindBin(this->YFitRange.second);
		double area = this->fithist->Integral(xlow,xhigh,ylow,yhigh);
		double corr = 0.0;

		this->keys = { {"Area",{0,area}},{"XMean",{1,xoffset}},{"XSigma",{2,xwidth}},{"YMean",{3,yoffset}},{"YSigma",{4,ywidth}},{"Correlation",{5,corr}}};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		if( this->fvalues.find("XMean") == this->fvalues.end() and this->bvalues.find("XMean") == this->bvalues.end() ){
			this->bvalues["XMean"] = this->XFitRange; 
		}
		if( this->fvalues.find("YMean") == this->fvalues.end() and this->bvalues.find("YMean") == this->bvalues.end() ){
			this->bvalues["YMean"] = this->YFitRange; 
		}
		if( this->fvalues.find("Correlation") == this->fvalues.end() and this->bvalues.find("Correlation") == this->bvalues.end() ){
			this->bvalues["Correlation"] = {-1.0,1.0}; 
		}

		FixAndBoundParameters();

		if( area > 0.0 ){
			std::string option = "R0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			//need to include the range
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str());

			if( not fitresult->IsEmpty() ){
				AssignFitValuesErrors(fitresult);
				AssignFitFuncParams();
				this->fithist->GetListOfFunctions()->Add(this->fitfunc);

				this->AddUncertaintyEllipse(ellipse,npts);
			}
		}
	}

	void InitBiGaussFlatBkgFit(double ellipse,int npts){
		this->fitfunc = new TF2("BiGaussFlatBkg",&PeakFit::BiGaussFlatBkg,XFitRange.first,XFitRange.second,YFitRange.first,YFitRange.second,7);
		this->components = {
			new TF2("BiGauss",&PeakFit::BiGauss,XFitRange.first,XFitRange.second,YFitRange.first,YFitRange.second,6),
			new TF2("FlatBkg",&CommonFit::Constant,XFitRange.first,XFitRange.second,YFitRange.first,YFitRange.second,1)
		};

		double xwidth = this->XFitRange.second - this->XFitRange.first;
		double ywidth = this->YFitRange.second - this->YFitRange.first;
		double xoffset = (this->XFitRange.second + this->XFitRange.first)/2.0;
		double yoffset = (this->YFitRange.second + this->YFitRange.first)/2.0;
		double xlow = this->fithist->GetXaxis()->FindBin(this->XFitRange.first);
		double xhigh = this->fithist->GetXaxis()->FindBin(this->XFitRange.second);
		double ylow = this->fithist->GetYaxis()->FindBin(this->YFitRange.first);
		double yhigh = this->fithist->GetYaxis()->FindBin(this->YFitRange.second);
		double area = this->fithist->Integral(xlow,xhigh,ylow,yhigh);
		double bkg = 0.0;
		double corr = 0.0;

		this->keys = { {"Area",{0,area}},
			{"XMean",{1,xoffset}},{"XSigma",{2,xwidth}},
			{"YMean",{3,yoffset}},{"YSigma",{4,ywidth}},
			{"Correlation",{5,corr}},
			{"Constant",{6,bkg}}
		};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		if( this->fvalues.find("XMean") == this->fvalues.end() and this->bvalues.find("XMean") == this->bvalues.end() ){
			this->bvalues["XMean"] = this->XFitRange; 
		}
		if( this->fvalues.find("YMean") == this->fvalues.end() and this->bvalues.find("YMean") == this->bvalues.end() ){
			this->bvalues["YMean"] = this->YFitRange; 
		}
		if( this->fvalues.find("Correlation") == this->fvalues.end() and this->bvalues.find("Correlation") == this->bvalues.end() ){
			this->bvalues["Correlation"] = {-1.0,1.0}; 
		}

		FixAndBoundParameters();

		if( area > 0.0 ){
			std::string option = "R0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			//need to include the range
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str());

			if( not fitresult->IsEmpty() ){
				AssignFitValuesErrors(fitresult);
				AssignFitFuncParams();
				this->fithist->GetListOfFunctions()->Add(this->fitfunc);

				this->AddUncertaintyEllipse(ellipse,npts);
			}
		}
	}

	void InitBiGaussLinXYBkgFit(double ellipse,int npts){
		this->fitfunc = new TF2("BiGaussLinXYBkg",&PeakFit::BiGaussLinXYBkg,XFitRange.first,XFitRange.second,YFitRange.first,YFitRange.second,10);
		this->components = {
			new TF2("BiGauss",&PeakFit::BiGauss,XFitRange.first,XFitRange.second,YFitRange.first,YFitRange.second,6),
			new TF2("LinXYBkg",&CommonFit::LinXY,XFitRange.first,XFitRange.second,YFitRange.first,YFitRange.second,4)
		};

		double xwidth = this->XFitRange.second - this->XFitRange.first;
		double ywidth = this->YFitRange.second - this->YFitRange.first;
		double xoffset = (this->XFitRange.second + this->XFitRange.first)/2.0;
		double yoffset = (this->YFitRange.second + this->YFitRange.first)/2.0;
		double xlow = this->fithist->GetXaxis()->FindBin(this->XFitRange.first);
		double xhigh = this->fithist->GetXaxis()->FindBin(this->XFitRange.second);
		double ylow = this->fithist->GetYaxis()->FindBin(this->YFitRange.first);
		double yhigh = this->fithist->GetYaxis()->FindBin(this->YFitRange.second);
		double area = this->fithist->Integral(xlow,xhigh,ylow,yhigh);
		double xlin = 0.0;
		double xcon = 0.0;
		double ylin = 0.0;
		double ycon = 0.0;
		double corr = 0.0;

		this->keys = { {"Area",{0,area}},
			{"XMean",{1,xoffset}},{"XSigma",{2,xwidth}},
			{"YMean",{3,yoffset}},{"YSigma",{4,ywidth}},
			{"Correlation",{5,corr}},
			{"XConstant",{6,xcon}},
			{"XLinear",{7,xlin}},
			{"YConstant",{8,ycon}},
			{"YLinear",{9,ylin}}
		};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		if( this->fvalues.find("XMean") == this->fvalues.end() and this->bvalues.find("XMean") == this->bvalues.end() ){
			this->bvalues["XMean"] = this->XFitRange; 
		}
		if( this->fvalues.find("YMean") == this->fvalues.end() and this->bvalues.find("YMean") == this->bvalues.end() ){
			this->bvalues["YMean"] = this->YFitRange; 
		}
		if( this->fvalues.find("Correlation") == this->fvalues.end() and this->bvalues.find("Correlation") == this->bvalues.end() ){
			this->bvalues["Correlation"] = {-1.0,1.0}; 
		}

		FixAndBoundParameters();

		if( area > 0.0 ){
			std::string option = "R0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			//need to include the range
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str());

			if( not fitresult->IsEmpty() ){
				AssignFitValuesErrors(fitresult);
				AssignFitFuncParams();
				this->fithist->GetListOfFunctions()->Add(this->fitfunc);

				this->AddUncertaintyEllipse(ellipse,npts);
			}
		}
	}

	//need to make these parameters that get passed in from command line
	//we only need a 15-sided polygon to have a decent approximation of the ellipse
	void AddUncertaintyEllipse(double sigma = 3,int npoints=15){
		//need to generate the ellipses using the 2x2 matrix and eigen vectors and values
		TMatrixDSym* errormat = new TMatrixDSym(2);
		(*errormat)(0,0) = (this->Results["XSigma"]*this->Results["XSigma"]);
		(*errormat)(1,1) = (this->Results["YSigma"]*this->Results["YSigma"]);
		(*errormat)(0,1) = (this->Results["Correlation"]*this->Results["XSigma"]*this->Results["YSigma"]);
		(*errormat)(1,0) = (this->Results["Correlation"]*this->Results["XSigma"]*this->Results["YSigma"]);
		TVectorD eigenvals;
		TMatrixD eigenvec = errormat->EigenVectors(eigenvals);

		//TArrow* MajorAxis = new TArrow(this->Results["XMean"],this->Results["YMean"],this->Results["XMean"] + eigenvec[0][0]*TMath::Sqrt(eigenvals[0]),this->Results["YMean"] + eigenvec[1][0]*TMath::Sqrt(eigenvals[0]),0.015,"|->");
		//MajorAxis->SetLineColor(kBlack);
		//this->fithist->GetListOfFunctions()->Add(MajorAxis);

		////this one is the x-axis because the time is "compressed" compared to the energy axis
		//TArrow* MinorAxis = new TArrow(this->Results["XMean"],this->Results["YMean"],this->Results["XMean"] + eigenvec[0][1]*TMath::Sqrt(eigenvals[1]),this->Results["YMean"] + eigenvec[1][1]*TMath::Sqrt(eigenvals[1]),0.015,"|->");
		//MinorAxis->SetLineColor(kBlack);
		//this->fithist->GetListOfFunctions()->Add(MinorAxis);

		std::string name = "Sigma";
		TCutG* unc = new TCutG(name.c_str(),npoints);
		for( int ii = 0; ii < npoints; ++ii ){
			auto t = TMath::TwoPi()*static_cast<double>(ii)/static_cast<double>(npoints-1);
			//this performs the affine transformation from a circle onto an arbitrary ellipse
			auto x = this->Results["XMean"] + sigma*(eigenvec[0][0]*TMath::Sqrt(eigenvals[0]))*TMath::Cos(t) + sigma*(eigenvec[0][1]*TMath::Sqrt(eigenvals[1]))*TMath::Sin(t);
			auto y = this->Results["YMean"] + sigma*(eigenvec[1][0]*TMath::Sqrt(eigenvals[0]))*TMath::Cos(t) + sigma*(eigenvec[1][1]*TMath::Sqrt(eigenvals[1]))*TMath::Sin(t);
			unc->SetPoint(ii,x,y);
		}
		unc->SetLineWidth(4);
		unc->SetLineColor(kBlack);
		this->fithist->GetListOfFunctions()->Add(unc);
	}

	virtual void FixAndBoundParameters() final{
		for( const auto& kv : this->keys ){
			auto fres = this->fvalues.find(kv.first);
			auto bres = this->bvalues.find(kv.first);
			if( fres != this->fvalues.end() ){
				this->fitfunc->SetParameter(kv.second.first,fres->second);
				this->fitfunc->FixParameter(kv.second.first,fres->second);
			}else if( bres != this->bvalues.end() ){
				auto middle = (bres->second.first+bres->second.second)/2.0;
				this->fitfunc->SetParameter(kv.second.first,middle);
				this->fitfunc->SetParLimits(kv.second.first,bres->second.first,bres->second.second);
			}else{
				this->fitfunc->SetParameter(kv.second.first,kv.second.second);
			}
		}
	}

	virtual void AssignFitFuncParams() final{
		for( const auto& kv : this->keys ){
			this->fitfunc->SetParameter(kv.second.first,this->Results[kv.first]);
		}
		this->fithist->GetListOfFunctions()->Clear();
	}

	void AssignFitValuesErrors(const TFitResultPtr& fitresult){
		for( const auto& kv : this->keys ){
			this->Results[kv.first] = fitresult->Parameter(kv.second.first);
			this->Errors[kv.first] = fitresult->ParError(kv.second.first);
		}
		this->Results["Chi2"] = fitresult->Chi2();
		this->Errors["NDF"] = fitresult->Ndf();
	}

	virtual void AssignFitParNames() final{
		for( const auto& kv : this->keys ){
			this->fitfunc->SetParName(kv.second.first,kv.first.c_str());
		}
	}

	virtual void WriteHistogram(bool storechi2) final{
		this->fithist->Write(0,2,0);
		if( storechi2 ){
			auto name = std::string(this->fithist->GetName());
			name += "_chi2";
			TH2* residual = dynamic_cast<TH2*>(this->fithist->Clone(name.c_str()));
			residual->Reset("ICEMS");
			residual->SetZTitle("#sigma");
			
			name += "_distribution";
			TH1* chi2dist = new TH1F(name.c_str(),"Chi2 Distribution; #sigma; counts",1000,-10,10);
			chi2dist->GetXaxis()->CenterTitle();
			chi2dist->GetYaxis()->CenterTitle();

			int minxbin = this->fithist->GetXaxis()->FindBin(this->XFitRange.first);
			int maxxbin = this->fithist->GetXaxis()->FindBin(this->XFitRange.second);
			int minybin = this->fithist->GetYaxis()->FindBin(this->YFitRange.first);
			int maxybin = this->fithist->GetYaxis()->FindBin(this->YFitRange.second);
			for( int ii = minxbin; ii <= maxxbin; ++ii ){
				for( int jj = minybin; jj < maxybin; ++jj ){
					double xcentroid = this->fithist->GetXaxis()->GetBinCenter(ii);
					double ycentroid = this->fithist->GetYaxis()->GetBinCenter(jj);
					auto fitval = this->fitfunc->Eval(xcentroid,ycentroid);
					auto hisval = this->fithist->GetBinContent(ii,jj);
					auto hiserr = this->fithist->GetBinError(ii,jj);
					auto uncert = hiserr;
					auto binchi2 = (fitval - hisval)*(fitval - hisval)/uncert/uncert;
					if( hisval > 0.0 ){
						if( (fitval - hisval) > 0.0 ){
							residual->SetBinContent(ii,jj,TMath::Sqrt(binchi2));
							chi2dist->Fill(TMath::Sqrt(binchi2));
						}else{
							residual->SetBinContent(ii,jj,-TMath::Sqrt(binchi2));
							chi2dist->Fill(-TMath::Sqrt(binchi2));
						}
						residual->SetBinError(ii,jj,0);
					}
				}
			}
			residual->Write(0,2,0);
			chi2dist->Write(0,2,0);
		}
	}


};

struct PeakFitter1D : public PeakFitter{
	std::pair<double,double> XFitRange;
	TH1* fithist;
	TF1* fitfunc;
	std::vector<TF1*> components;
	static constexpr auto oned = describe_enumerators_as_array<FitTypes::OneDim>();
	std::string fitname;

	const std::string GetHisName() const{
		return std::string(this->fithist->GetName());
	}

	PeakFitter1D(double l,double u,bool chi2,bool dbg,int mode,TH1* hist,
			const std::map<std::string,double>& fixedvalues,const std::map<std::string,std::pair<double,double>>& boundedvalues) 
		: PeakFitter(chi2,dbg,fixedvalues,boundedvalues),XFitRange(l,u),fithist(hist){
		for( const auto& kv : fvalues ){
			if( bvalues.find(kv.first) != bvalues.end() ){
				throw std::runtime_error("Parameter is both fixed and bounded");
			}
		}
		
		for( const auto& x : oned ){
			if( x.value == mode ){
				this->fitname = std::string(x.name);
			}
		}

		this->fithist->SetLineColor(kBlack);
		if( mode == FitTypes::OneDim::GaussNLinBkgFit ){
			this->fitfunc = new TF1("GaussNLinBkg",
					&PeakFit::GaussNLinBkg,
					XFitRange.first,XFitRange.second,5);
			this->InitGaussNLinBkgFit();
		}else if( mode == FitTypes::OneDim::GaussNErfBkgFit ){
			this->fitfunc = new TF1("GaussNErfBkg",
					&PeakFit::GaussNErfBkg,
					XFitRange.first,XFitRange.second,6);
			this->InitGaussNErfBkgFit();
		}else if( mode == FitTypes::OneDim::SingleTailingGaussNFit ){
			this->fitfunc = new TF1("SingleTailingGaussN",
					&PeakFit::SingleTailingGaussN,
					XFitRange.first,XFitRange.second,4);
			this->InitSingleTailingGaussNFit();
		}else if( mode == FitTypes::OneDim::DoubleTailingGaussNFit ){
			this->fitfunc = new TF1("DoubleTailingGaussN",
					&PeakFit::DoubleTailingGaussN,
					XFitRange.first,XFitRange.second,8);
			this->InitDoubleTailingGaussNFit();
		}else if( mode == FitTypes::OneDim::SingleTailingGaussNLinBkgFit ){
			this->fitfunc = new TF1("SingleTailingGaussNLinBkg",
					&PeakFit::SingleTailingGaussNLinBkg,
					XFitRange.first,XFitRange.second,6);
			this->InitSingleTailingGaussNLinBkgFit();
		}else if( mode == FitTypes::OneDim::ErfFit ){
			this->fitfunc = new TF1("Erf",
					&PeakFit::Erf,
					XFitRange.first,XFitRange.second,3);
			this->InitErfFit();
		}else if( mode == FitTypes::OneDim::NGaussNFit ){
			this->InitNGaussNFit();
		}else if( mode == FitTypes::OneDim::NGaussNLinBkgFit ){
			this->InitNGaussNLinBkgFit();
		}else if( mode == FitTypes::OneDim::NGaussNErfBkgFit ){
			this->InitNGaussNErfBkgFit();
		}else if( mode == FitTypes::OneDim::SimpleImplantationCurveFit ){
			this->fitfunc = new TF1("SimpleImplantationCurve",
					&PeakFit::SimpleImplantationCurve,
					XFitRange.first,XFitRange.second,3);
			this->InitSimpleImplantationCurveFit();
		}else if( mode == FitTypes::OneDim::SinglePlasticTrace ){
			this->fitfunc = new TF1("SinglePlasticTrace",
					&PulseFit::SingleTraceFit,
					XFitRange.first,XFitRange.second,5);
			this->InitSinglePlasticTraceFit();
		}else if( mode == FitTypes::OneDim::DoublePlasticTrace ){
			this->fitfunc = new TF1("DoublePlasticTrace",
					&PulseFit::DoubleTraceFit,
					XFitRange.first,XFitRange.second,9);
			this->InitDoublePlasticTraceFit();
		}else if( mode == FitTypes::SingleDaughterImplantationCurveFit ){
			this->fitfunc = new TF1("SingleDaughterPairImplantationCurve",
					&PeakFit::SingleDaughterPairImplantationCurve,
					XFitRange.first,XFitRange.second,6);
			this->InitSingleDaughterImplantationCurveFit();
		}else{
			throw std::runtime_error("Unknown peak fitting mode");
		}
	}

	void InitSimpleImplantationCurveFit(){
		this->fitfunc->SetNpx(this->fithist->GetNbinsX()*10);
		this->fitfunc->SetLineColor(kRed);
		this->components = {
			new TF1("Constant",&CommonFit::Constant,XFitRange.first,XFitRange.second,1),
			new TF1("SimpleHalfLife",&PeakFit::SimpleHalfLife,0.0,XFitRange.second,2)
		};
		this->components.at(0)->SetLineColor(kMagenta);
		this->components.at(1)->SetLineColor(kGreen);
		this->components.at(0)->SetNpx(this->fithist->GetNbinsX()*10);
		this->components.at(1)->SetNpx(this->fithist->GetNbinsX()*10);

		auto leftbin = this->fithist->FindBin(XFitRange.first);
		auto zerobin = this->fithist->FindBin(0.0);
		auto rightbin = this->fithist->FindBin(XFitRange.second);
		double bkg = this->fithist->Integral(leftbin,zerobin)/(zerobin-leftbin);
		double l = this->fithist->GetBinContent(zerobin+1) - bkg;
		double h = l/2.0;
		double half_life = XFitRange.second;
		for( int jj = zerobin; jj < rightbin; ++jj ){
			if( (this->fithist->GetBinContent(jj) - bkg) < h ){
				half_life = this->fithist->GetBinCenter(jj);
				break;
			}
		}
		double amp = this->fithist->GetBinContent(zerobin+1);
		this->keys = { {"Constant",{0,bkg}}, {"Amplitude",{1,amp}}, {"HalfLife",{2,half_life}}};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();

		auto minbin = this->fithist->FindBin(this->XFitRange.first);
		auto maxbin = this->fithist->FindBin(this->XFitRange.second);
		auto integral = this->fithist->Integral(minbin,maxbin);
		
		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",XFitRange.first,XFitRange.second);

			if( not fitresult->IsEmpty() ){
				AssignFitValuesErrors(fitresult);
				AssignFitFuncParams();
				this->fithist->GetListOfFunctions()->Add(this->fitfunc);

				this->components.at(0)->SetParameters(this->Results["Constant"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(0));
				this->components.at(1)->SetParameters(this->Results["Amplitude"],this->Results["HalfLife"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(1));
			}
		}

	}
	
	void InitSingleDaughterImplantationCurveFit(){
		this->fitfunc->SetNpx(this->fithist->GetNbinsX()*10);
		this->fitfunc->SetLineColor(kRed);
		//there are 4 components, bkg, parent decay, beta-daughter, beta-n-daughter
		//need to make correct functions for this though
		this->components = {
			new TF1("Constant",&CommonFit::Constant,XFitRange.first,XFitRange.second,1),
			new TF1("Parent",&PeakFit::SimpleImplantationCurve,0.0,XFitRange.second,3),
			new TF1("BetaDaughter",&PeakFit::ImplantationBatemanStep,0.0,XFitRange.second,4),
			new TF1("BetaNDaughter",&PeakFit::ImplantationBatemanStep,0.0,XFitRange.second,4)
		};
		this->components.at(0)->SetLineColor(kMagenta);
		this->components.at(1)->SetLineColor(kGreen);
		this->components.at(2)->SetLineColor(kAzure);
		this->components.at(3)->SetLineColor(kOrange-3);
		this->components.at(0)->SetNpx(this->fithist->GetNbinsX()*10);
		this->components.at(1)->SetNpx(this->fithist->GetNbinsX()*10);
		this->components.at(2)->SetNpx(this->fithist->GetNbinsX()*10);
		this->components.at(3)->SetNpx(this->fithist->GetNbinsX()*10);

		auto leftbin = this->fithist->FindBin(XFitRange.first);
		auto zerobin = this->fithist->FindBin(0.0);
		auto rightbin = this->fithist->FindBin(XFitRange.second);
		double bkg = this->fithist->Integral(leftbin,zerobin)/(zerobin-leftbin);
		double l = this->fithist->GetBinContent(zerobin+1) - bkg;
		double h = l/2.0;
		double half_life = XFitRange.second;
		for( int jj = zerobin; jj < rightbin; ++jj ){
			if( (this->fithist->GetBinContent(jj) - bkg) < h ){
				half_life = this->fithist->GetBinCenter(jj);
				break;
			}
		}
		double amp = this->fithist->GetBinContent(zerobin+1);
		double pn = 0.0;
		this->keys = { {"Constant",{0,bkg}}, {"Amplitude",{1,amp}}, {"HalfLife",{2,half_life}}, 
			{"Pn",{3,pn}}, {"BetaHalfLife",{4,half_life}}, {"BetaNHalfLife",{5,half_life}} };
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();

		auto minbin = this->fithist->FindBin(this->XFitRange.first);
		auto maxbin = this->fithist->FindBin(this->XFitRange.second);
		auto integral = this->fithist->Integral(minbin,maxbin);
	
		if( this->fvalues.find("Pn") == this->fvalues.end() and this->bvalues.find("Pn") == this->bvalues.end() ){
			this->bvalues["Pn"] = {0.0,1.0}; 
		}
	
		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",XFitRange.first,XFitRange.second);

			if( not fitresult->IsEmpty() ){
				AssignFitValuesErrors(fitresult);
				AssignFitFuncParams();
				this->fithist->GetListOfFunctions()->Add(this->fitfunc);

				this->components.at(0)->SetParameters(this->Results["Constant"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(0));
				this->components.at(1)->SetParameters(this->Results["Constant"],this->Results["Amplitude"],this->Results["HalfLife"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(1));
				this->components.at(2)->SetParameters(this->Results["Constant"],this->Results["Amplitude"]*(1.0-this->Results["Pn"]),this->Results["HalfLife"],this->Results["BetaHalfLife"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(2));
				this->components.at(3)->SetParameters(this->Results["Constant"],this->Results["Amplitude"]*this->Results["Pn"],this->Results["HalfLife"],this->Results["BetaNHalfLife"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(3));
			}
		}

	}
	
	//this one is weird and must live here
	void InitNGaussNFit(){
		this->fithist->SetLineColor(kBlack);
		int npeaks = -1;
		if( this->fvalues.find("NPeaks") == this->fvalues.end() ){
			throw std::runtime_error("NGaussN fit requires a fixed value named NPeaks to generate fit");
		}else{
			npeaks = this->fvalues["NPeaks"];
		}

		double width = this->XFitRange.second - this->XFitRange.first;
		double offset = (this->XFitRange.second + this->XFitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));

		this->fitfunc = new TF1("NGaussN",&PeakFit::NGaussN,XFitRange.first,XFitRange.second,3*npeaks+1);
		this->fitfunc->SetLineColor(kRed);
		this->keys = { {"NPeaks",{0,npeaks}} };
		for( int ii = 0; ii < npeaks; ++ii ){
			this->components.push_back(new TF1("GaussN",&PeakFit::GaussN,XFitRange.first,XFitRange.second,3));
			this->components.back()->SetLineColor(kMagenta+(ii%6)-4);
			this->keys.insert({"Area"+std::to_string(ii),{3*ii+1,area}});
			this->keys.insert({"Mean"+std::to_string(ii),{3*ii+2,offset}});
			this->keys.insert({"Sigma"+std::to_string(ii),{3*ii+3,width}});
		}

		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		for( int ii = 0; ii < npeaks; ++ii ){
			if( this->fvalues.find("Mean"+std::to_string(ii)) == this->fvalues.end() 
					and this->bvalues.find("Mean"+std::to_string(ii)) == this->bvalues.end() ){
				this->bvalues["Mean"+std::to_string(ii)] = this->XFitRange; 
			}
		}

		auto minbin = this->fithist->FindBin(this->XFitRange.first);
		auto maxbin = this->fithist->FindBin(this->XFitRange.second);
		auto integral = this->fithist->Integral(minbin,maxbin);

		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",XFitRange.first,XFitRange.second);

			if( not fitresult->IsEmpty() ){
				AssignFitValuesErrors(fitresult);
				AssignFitFuncParams();
				this->fithist->GetListOfFunctions()->Add(this->fitfunc);

				std::vector<TLine*> centroids;
				for( int ii = 0; ii < npeaks; ++ii ){
					this->components.at(ii)->SetParameters(
							this->Results["Area"+std::to_string(ii)],
							this->Results["Mean"+std::to_string(ii)],
							this->Results["Sigma"+std::to_string(ii)]
							);
					this->fithist->GetListOfFunctions()->Add(this->components.at(ii));
					centroids.push_back(new TLine(
							this->Results["Mean"+std::to_string(ii)],
							0,
							this->Results["Mean"+std::to_string(ii)],
							0.75*(this->fithist->GetBinContent(this->fithist->FindBin(this->Results["Mean"+std::to_string(ii)]))))
							);
					centroids.back()->SetLineColor(kAzure+(ii%6)-4);
					this->fithist->GetListOfFunctions()->Add(centroids.at(ii));
				}
			}
		}
	}
	
	//same for here
	void InitNGaussNLinBkgFit(){
		this->fithist->SetLineColor(kBlack);
		int npeaks = -1;
		if( this->fvalues.find("NPeaks") == this->fvalues.end() ){
			throw std::runtime_error("NGaussN fit requires a fixed value named NPeaks to generate fit");
		}else{
			npeaks = this->fvalues["NPeaks"];
		}

		double bkg_offset = 0;
		double bkg_slope = 0;
		double width = this->XFitRange.second - this->XFitRange.first;
		double offset = (this->XFitRange.second + this->XFitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));

		this->fitfunc = new TF1("NGaussNLinBkg",&PeakFit::NGaussNLinBkg,XFitRange.first,XFitRange.second,3*npeaks+3);
		this->fitfunc->SetLineColor(kRed);
		this->keys = { {"NPeaks",{0,npeaks}} };
		for( int ii = 0; ii < npeaks; ++ii ){
			this->components.push_back(new TF1("GaussN",&PeakFit::GaussN,XFitRange.first,XFitRange.second,3));
			this->components.back()->SetLineColor(kMagenta+(ii%6)-4);
			this->keys.insert({"Area"+std::to_string(ii),{3*ii+1,area}});
			this->keys.insert({"Mean"+std::to_string(ii),{3*ii+2,offset}});
			this->keys.insert({"Sigma"+std::to_string(ii),{3*ii+3,width}});
		}
		this->components.push_back(new TF1("LinBkg",&CommonFit::Linear,XFitRange.first,XFitRange.second,2));
		this->keys.insert({"BkgOffset",{3*npeaks+1,bkg_offset}});
		this->keys.insert({"BkgSlope",{3*npeaks+2,bkg_slope}});
		this->components.at(npeaks)->SetLineColor(kGreen);

		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		for( int ii = 0; ii < npeaks; ++ii ){
			if( this->fvalues.find("Mean"+std::to_string(ii)) == this->fvalues.end() 
					and this->bvalues.find("Mean"+std::to_string(ii)) == this->bvalues.end() ){
				this->bvalues["Mean"+std::to_string(ii)] = this->XFitRange; 
			}
		}

		auto minbin = this->fithist->FindBin(this->XFitRange.first);
		auto maxbin = this->fithist->FindBin(this->XFitRange.second);
		auto integral = this->fithist->Integral(minbin,maxbin);

		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",XFitRange.first,XFitRange.second);

			if( not fitresult->IsEmpty() ){
				AssignFitValuesErrors(fitresult);
				AssignFitFuncParams();
				this->fithist->GetListOfFunctions()->Add(this->fitfunc);

				std::vector<TLine*> centroids;
				for( int ii = 0; ii < npeaks; ++ii ){
					this->components.at(ii)->SetParameters(
							this->Results["Area"+std::to_string(ii)],
							this->Results["Mean"+std::to_string(ii)],
							this->Results["Sigma"+std::to_string(ii)]
							);
					this->fithist->GetListOfFunctions()->Add(this->components.at(ii));
					centroids.push_back(new TLine(
							this->Results["Mean"+std::to_string(ii)],
							0,
							this->Results["Mean"+std::to_string(ii)],
							0.75*(this->fithist->GetBinContent(this->fithist->FindBin(this->Results["Mean"+std::to_string(ii)]))))
							);
					centroids.back()->SetLineColor(kAzure+(ii%6)-4);
					this->fithist->GetListOfFunctions()->Add(centroids.at(ii));
				}
				this->components.at(npeaks)->SetParameters(this->Results["BkgOffset"],this->Results["BkgSlope"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(npeaks));
			}
		}
	}

	//must also exist as defined here until I come up with a better way -tjr
	void InitNGaussNErfBkgFit(){
		this->fithist->SetLineColor(kBlack);
		int npeaks = -1;
		if( this->fvalues.find("NPeaks") == this->fvalues.end() ){
			throw std::runtime_error("NGaussN fit requires a fixed value named NPeaks to generate fit");
		}else{
			npeaks = this->fvalues["NPeaks"];
		}

		double bkg_offset = 0;
		double bkg_slope = 0;
		double width = this->XFitRange.second - this->XFitRange.first;
		double offset = (this->XFitRange.second + this->XFitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));

		this->fitfunc = new TF1("NGaussNErfBkg",&PeakFit::NGaussNErfBkg,XFitRange.first,XFitRange.second,4*npeaks+3);
		this->fitfunc->SetLineColor(kRed);
		this->keys = { {"NPeaks",{0,npeaks}} };
		for( int ii = 0; ii < npeaks; ++ii ){
			this->components.push_back(new TF1("GaussN",&PeakFit::GaussN,XFitRange.first,XFitRange.second,3));
			this->components.back()->SetLineColor(kMagenta+(ii%6)-4);
			this->keys.insert({"Area"+std::to_string(ii),{4*ii+1,area}});
			this->keys.insert({"Mean"+std::to_string(ii),{4*ii+2,offset}});
			this->keys.insert({"Sigma"+std::to_string(ii),{4*ii+3,width}});
			
			//add in the erf for each as well
			this->components.push_back(new TF1("ErfBkg",&PeakFit::GaussErf,XFitRange.first,XFitRange.second,3));
			this->components.back()->SetLineColor(kGreen+(ii%6)-4);
			this->keys.insert({"ComptonArea"+std::to_string(ii),{4*ii+4,area}});
		}
		this->components.push_back(new TF1("LinBkg",&CommonFit::Linear,XFitRange.first,XFitRange.second,2));
		this->keys.insert({"BkgOffset",{4*npeaks+1,bkg_offset}});
		this->keys.insert({"BkgSlope",{4*npeaks+2,bkg_slope}});
		this->components.back()->SetLineColor(kViolet);

		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		for( int ii = 0; ii < npeaks; ++ii ){
			if( this->fvalues.find("Mean"+std::to_string(ii)) == this->fvalues.end() 
					and this->bvalues.find("Mean"+std::to_string(ii)) == this->bvalues.end() ){
				this->bvalues["Mean"+std::to_string(ii)] = this->XFitRange; 
			}
		}

		auto minbin = this->fithist->FindBin(this->XFitRange.first);
		auto maxbin = this->fithist->FindBin(this->XFitRange.second);
		auto integral = this->fithist->Integral(minbin,maxbin);

		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",XFitRange.first,XFitRange.second);

			if( not fitresult->IsEmpty() ){
				AssignFitValuesErrors(fitresult);
				AssignFitFuncParams();
				this->fithist->GetListOfFunctions()->Add(this->fitfunc);

				std::vector<TLine*> centroids;
				for( int ii = 0; ii < npeaks; ++ii ){
					this->components.at(2*ii)->SetParameters(
							this->Results["Area"+std::to_string(ii)],
							this->Results["Mean"+std::to_string(ii)],
							this->Results["Sigma"+std::to_string(ii)]
							);
					this->fithist->GetListOfFunctions()->Add(this->components.at(2*ii));

					this->components.at(2*ii+1)->SetParameters(
							this->Results["ComptonArea"+std::to_string(ii)],
							this->Results["Mean"+std::to_string(ii)],
							this->Results["Sigma"+std::to_string(ii)]
							);
					this->fithist->GetListOfFunctions()->Add(this->components.at(2*ii+1));

					centroids.push_back(new TLine(
							this->Results["Mean"+std::to_string(ii)],
							0,
							this->Results["Mean"+std::to_string(ii)],
							0.75*(this->fithist->GetBinContent(this->fithist->FindBin(this->Results["Mean"+std::to_string(ii)]))))
							);
					centroids.back()->SetLineColor(kAzure+(ii%6)-4);
					this->fithist->GetListOfFunctions()->Add(centroids.at(ii));
				}
				this->components.at(2*npeaks)->SetParameters(this->Results["BkgOffset"],this->Results["BkgSlope"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(2*npeaks));
			}
		}
	}

	void InitGaussNLinBkgFit(){
		this->fitfunc->SetLineColor(kRed);
		this->components = {
			new TF1("GaussN",&PeakFit::GaussN,XFitRange.first,XFitRange.second,3),
			new TF1("LinBkg",&CommonFit::Linear,XFitRange.first,XFitRange.second,2)
		};
		this->components.at(0)->SetLineColor(kMagenta);
		this->components.at(1)->SetLineColor(kGreen);

		double bkg_offset = 0;
		double bkg_slope = 0;
		double width = this->XFitRange.second - this->XFitRange.first;
		double offset = (this->XFitRange.second + this->XFitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));

		this->keys = { {"Area",{0,area}},{"Mean",{1,offset}},{"Sigma",{2,width}},{"BkgOffset",{3,bkg_offset}},{"BkgSlope",{4,bkg_slope}}};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		if( this->fvalues.find("Mean") == this->fvalues.end() and this->bvalues.find("Mean") == this->bvalues.end() ){
			this->bvalues["Mean"] = this->XFitRange; 
		}

		auto minbin = this->fithist->FindBin(this->XFitRange.first);
		auto maxbin = this->fithist->FindBin(this->XFitRange.second);
		auto integral = this->fithist->Integral(minbin,maxbin);

		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",XFitRange.first,XFitRange.second);

			if( not fitresult->IsEmpty() ){
				AssignFitValuesErrors(fitresult);
				AssignFitFuncParams();
				this->fithist->GetListOfFunctions()->Add(this->fitfunc);

				this->components.at(0)->SetParameters(this->Results["Area"],this->Results["Mean"],this->Results["Sigma"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(0));
				this->components.at(1)->SetParameters(this->Results["BkgOffset"],this->Results["BkgSlope"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(1));

				TLine* gauss_centroid = new TLine(this->Results["Mean"],0,
						this->Results["Mean"],0.75*(this->fithist->GetBinContent(this->fithist->FindBin(this->Results["Mean"]))));
				gauss_centroid->SetLineColor(kAzure);
				this->fithist->GetListOfFunctions()->Add(gauss_centroid);
			}
		}
	}
	
	void InitGaussNErfBkgFit(){
		this->fitfunc->SetLineColor(kRed);
		this->components = {
			new TF1("GaussN",&PeakFit::GaussN,XFitRange.first,XFitRange.second,3),
			new TF1("ErfBkg",&PeakFit::GaussErf,XFitRange.first,XFitRange.second,3),
			new TF1("LinBkg",&CommonFit::Quad,XFitRange.first,XFitRange.second,3)
		};
		this->components.at(0)->SetLineColor(kMagenta);
		this->components.at(1)->SetLineColor(kGreen);
		this->components.at(2)->SetLineColor(kViolet);

		double cbkg = 0;
		double sbkg = 0;
		double width = this->XFitRange.second - this->XFitRange.first;
		double offset = (this->XFitRange.second + this->XFitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));
		double ca = area;

		this->keys = { {"Area",{0,area}},{"Mean",{1,offset}},{"Sigma",{2,width}},{"ComptonArea",{3,ca}},{"BkgOffset",{4,cbkg}},{"BkgSlope",{5,sbkg}}};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		if( this->fvalues.find("Mean") == this->fvalues.end() and this->bvalues.find("Mean") == this->bvalues.end() ){
			this->bvalues["Mean"] = this->XFitRange; 
		}

		auto minbin = this->fithist->FindBin(this->XFitRange.first);
		auto maxbin = this->fithist->FindBin(this->XFitRange.second);
		auto integral = this->fithist->Integral(minbin,maxbin);

		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",XFitRange.first,XFitRange.second);

			if( not fitresult->IsEmpty() ){
				AssignFitValuesErrors(fitresult);
				AssignFitFuncParams();
				this->fithist->GetListOfFunctions()->Add(this->fitfunc);

				this->components.at(0)->SetParameters(this->Results["Area"],this->Results["Mean"],this->Results["Sigma"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(0));
				this->components.at(1)->SetParameters(this->Results["ComptonArea"],this->Results["Mean"],this->Results["Sigma"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(1));
				this->components.at(2)->SetParameters(this->Results["BkgOffset"],this->Results["BkgSlope"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(2));

				TLine* gauss_centroid = new TLine(this->Results["Mean"],0,
						this->Results["Mean"],0.75*(this->fithist->GetBinContent(this->fithist->FindBin(this->Results["Mean"]))));
				gauss_centroid->SetLineColor(kAzure);
				this->fithist->GetListOfFunctions()->Add(gauss_centroid);
			}
		}
	}

	void InitSingleTailingGaussNFit(){
		this->fitfunc->SetLineColor(kRed);
		this->components = {
		};
		
		auto minbin = this->fithist->FindBin(this->XFitRange.first);
		auto maxbin = this->fithist->FindBin(this->XFitRange.second);

		double width = this->XFitRange.second - this->XFitRange.first;
		double offset = (this->XFitRange.second + this->XFitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));
		double tau = this->fithist->GetBinContent(maxbin) - this->fithist->GetBinContent(minbin);

		this->keys = {{"Area",{0,area}},{"Mean",{1,offset}},{"Sigma",{2,width}},{"Tau",{3,tau}}};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		if( this->fvalues.find("Mean") == this->fvalues.end() and this->bvalues.find("Mean") == this->bvalues.end() ){
			this->bvalues["Mean"] = this->XFitRange; 
		}

		auto integral = this->fithist->Integral(minbin,maxbin);

		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",XFitRange.first,XFitRange.second);

			if( not fitresult->IsEmpty() ){
				AssignFitValuesErrors(fitresult);
				AssignFitFuncParams();
				this->fithist->GetListOfFunctions()->Add(this->fitfunc);

				TLine* gauss_centroid = new TLine(this->Results["Mean"],0,
						this->Results["Mean"],0.75*(this->fithist->GetBinContent(this->fithist->FindBin(this->Results["Mean"]))));
				gauss_centroid->SetLineColor(kAzure);
				this->fithist->GetListOfFunctions()->Add(gauss_centroid);
			}
		}
	}
	
	void InitDoubleTailingGaussNFit(){
		this->fitfunc->SetLineColor(kRed);
		this->components = {
			new TF1("TailingGaussN1",&PeakFit::SingleTailingGaussN,XFitRange.first,XFitRange.second,4),
			new TF1("TailingGaussN2",&PeakFit::SingleTailingGaussN,XFitRange.first,XFitRange.second,4)
		};
		this->components.at(0)->SetLineColor(kMagenta);
		this->components.at(1)->SetLineColor(kGreen);
		
		auto minbin = this->fithist->FindBin(this->XFitRange.first);
		auto maxbin = this->fithist->FindBin(this->XFitRange.second);

		double width = this->XFitRange.second - this->XFitRange.first;
		double offset = (this->XFitRange.second + this->XFitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));
		double tau = this->fithist->GetBinContent(maxbin) - this->fithist->GetBinContent(minbin);

		this->keys = {{"Area1",{0,area}},{"Mean1",{1,offset}},{"Sigma1",{2,width}},{"Tau1",{3,tau}},{"Area2",{4,area}},{"Mean2",{5,offset}},{"Sigma2",{6,width}},{"Tau2",{7,tau}}};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		if( this->fvalues.find("Mean1") == this->fvalues.end() and this->bvalues.find("Mean1") == this->bvalues.end() ){
			this->bvalues["Mean1"] = this->XFitRange; 
		}
		if( this->fvalues.find("Mean2") == this->fvalues.end() and this->bvalues.find("Mean2") == this->bvalues.end() ){
			this->bvalues["Mean2"] = this->XFitRange; 
		}

		auto integral = this->fithist->Integral(minbin,maxbin);

		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",XFitRange.first,XFitRange.second);

			if( not fitresult->IsEmpty() ){
				AssignFitValuesErrors(fitresult);
				AssignFitFuncParams();
				this->fithist->GetListOfFunctions()->Add(this->fitfunc);

				this->components.at(0)->SetParameters(this->Results["Area1"],this->Results["Mean1"],this->Results["Sigma1"],this->Results["Tau1"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(0));

				this->components.at(1)->SetParameters(this->Results["Area2"],this->Results["Mean2"],this->Results["Sigma2"],this->Results["Tau2"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(1));

				TLine* gauss1_centroid = new TLine(this->Results["Mean1"],0,
						this->Results["Mean1"],0.75*(this->fithist->GetBinContent(this->fithist->FindBin(this->Results["Mean1"]))));
				gauss1_centroid->SetLineColor(kAzure);
				this->fithist->GetListOfFunctions()->Add(gauss1_centroid);
				
				TLine* gauss2_centroid = new TLine(this->Results["Mean2"],0,
						this->Results["Mean2"],0.75*(this->fithist->GetBinContent(this->fithist->FindBin(this->Results["Mean2"]))));
				gauss2_centroid->SetLineColor(kOrange-3);
				this->fithist->GetListOfFunctions()->Add(gauss2_centroid);
			}
		}
	}
	
	void InitSingleTailingGaussNLinBkgFit(){
		this->fitfunc->SetLineColor(kRed);
		this->components = {
			new TF1("TailingGaussN1",&PeakFit::SingleTailingGaussN,XFitRange.first,XFitRange.second,4),
			new TF1("LinBkg",&CommonFit::Linear,XFitRange.first,XFitRange.second,2)
		};
		this->components.at(0)->SetLineColor(kMagenta);
		this->components.at(1)->SetLineColor(kGreen);
		
		auto minbin = this->fithist->FindBin(this->XFitRange.first);
		auto maxbin = this->fithist->FindBin(this->XFitRange.second);

		double width = this->XFitRange.second - this->XFitRange.first;
		double offset = (this->XFitRange.second + this->XFitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));
		double tau = this->fithist->GetBinContent(maxbin) - this->fithist->GetBinContent(minbin);
		double bkgoffset = 0.0;
		double bkgslope = 0.0;

		this->keys = {{"Area",{0,area}},{"Mean",{1,offset}},{"Sigma",{2,width}},{"Tau",{3,tau}},{"BkgOffset",{4,bkgoffset}},{"BkgSlope",{5,bkgslope}}};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		if( this->fvalues.find("Mean") == this->fvalues.end() and this->bvalues.find("Mean") == this->bvalues.end() ){
			this->bvalues["Mean"] = this->XFitRange; 
		}

		auto integral = this->fithist->Integral(minbin,maxbin);

		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",XFitRange.first,XFitRange.second);

			if( not fitresult->IsEmpty() ){
				AssignFitValuesErrors(fitresult);
				AssignFitFuncParams();
				this->fithist->GetListOfFunctions()->Add(this->fitfunc);
				
				this->components.at(0)->SetParameters(this->Results["Area"],this->Results["Mean"],this->Results["Sigma"],this->Results["Tau"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(0));

				this->components.at(1)->SetParameters(this->Results["BkgOffset"],this->Results["BkgSlope"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(1));

				TLine* gauss_centroid = new TLine(this->Results["Mean"],0,
						this->Results["Mean"],0.75*(this->fithist->GetBinContent(this->fithist->FindBin(this->Results["Mean"]))));
				gauss_centroid->SetLineColor(kAzure);
				this->fithist->GetListOfFunctions()->Add(gauss_centroid);
			}
		}
	}

	void InitSinglePlasticTraceFit(){
		this->fitfunc->SetNpx(this->fithist->GetNbinsX()*10);
		this->fitfunc->SetLineColor(kRed);
		this->components = {
			new TF1("Offset",&CommonFit::Constant,XFitRange.first,XFitRange.second,1),
			new TF1("Pulse",&PulseFit::Pulse,XFitRange.first,XFitRange.second,4)
		};
		this->components.at(0)->SetLineColor(kMagenta);
		this->components.at(0)->SetNpx(this->fithist->GetNbinsX()*10);
		this->components.at(1)->SetLineColor(kGreen);
		this->components.at(1)->SetNpx(this->fithist->GetNbinsX()*10);
		
		auto minbin = this->fithist->FindBin(this->XFitRange.first);
		auto maxbin = this->fithist->FindBin(this->XFitRange.second);

		double width = this->XFitRange.second - this->XFitRange.first;
		double offset = (this->XFitRange.second + this->XFitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));
		double tau = this->fithist->GetBinContent(maxbin) - this->fithist->GetBinContent(minbin);
		double bkgoffset = 0.0;

		this->keys = {{"Offset",{0,bkgoffset}},{"Amp",{1,area}},{"T0",{2,offset}},{"Rise",{3,width}},{"Fall",{4,tau}}};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		if( this->fvalues.find("T0") == this->fvalues.end() and this->bvalues.find("T0") == this->bvalues.end() ){
			this->bvalues["T0"] = this->XFitRange; 
		}

		auto integral = this->fithist->Integral(minbin,maxbin);

		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQWW";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",XFitRange.first,XFitRange.second);

			if( not fitresult->IsEmpty() ){
				AssignFitValuesErrors(fitresult);
				AssignFitFuncParams();
				this->fithist->GetListOfFunctions()->Add(this->fitfunc);
				
				this->components.at(0)->SetParameters(this->Results["Amp"],this->Results["T0"],this->Results["Rise"],this->Results["Fall"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(0));

				this->components.at(1)->SetParameters(this->Results["Offset"]);
				this->fithist->GetListOfFunctions()->Add(this->components.at(1));

				TLine* centroid = new TLine(this->Results["T0"],0,
						this->Results["T0"],0.75*(this->fithist->GetBinContent(this->fithist->FindBin(this->Results["T0"]))));
				centroid->SetLineColor(kAzure);
				this->fithist->GetListOfFunctions()->Add(centroid);
			}
		}
	}

	void InitDoublePlasticTraceFit(){
		if( this->debug) {
			this->fitfunc->SetNpx(this->fithist->GetNbinsX()*10);
			this->fitfunc->SetLineColor(kRed);
			this->components = {
				new TF1("Offset",&CommonFit::Constant,XFitRange.first,XFitRange.second,1),
				new TF1("Pulse1",&PulseFit::Pulse,XFitRange.first,XFitRange.second,4),
				new TF1("Pulse2",&PulseFit::Pulse,XFitRange.first,XFitRange.second,4)
			};
			this->components.at(0)->SetLineColor(kMagenta);
			this->components.at(0)->SetNpx(this->fithist->GetNbinsX()*10);
			this->components.at(1)->SetLineColor(kGreen);
			this->components.at(1)->SetNpx(this->fithist->GetNbinsX()*10);
			this->components.at(2)->SetLineColor(kGreen+1);
			this->components.at(2)->SetNpx(this->fithist->GetNbinsX()*10);
		}
		
		auto minbin = this->fithist->FindBin(this->XFitRange.first);
		auto maxbin = this->fithist->FindBin(this->XFitRange.second);

		double width = this->XFitRange.second - this->XFitRange.first;
		double offset = (this->XFitRange.second + this->XFitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));
		double tau = this->fithist->GetBinContent(maxbin) - this->fithist->GetBinContent(minbin);
		double bkgoffset = 0.0;

		this->keys = {{"Offset",{0,bkgoffset}},{"Amp1",{1,area}},{"T01",{2,offset}},{"Rise1",{3,width}},{"Fall1",{4,tau}},{"Amp2",{5,area}},{"T02",{6,offset}},{"Rise2",{7,width}},{"Fall2",{8,tau}}};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		if( this->fvalues.find("T01") == this->fvalues.end() and this->bvalues.find("T01") == this->bvalues.end() ){
			this->bvalues["T01"] = this->XFitRange; 
		}
		if( this->fvalues.find("T02") == this->fvalues.end() and this->bvalues.find("T02") == this->bvalues.end() ){
			this->bvalues["T02"] = this->XFitRange; 
		}

		auto integral = this->fithist->Integral(minbin,maxbin);

		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQWW";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",XFitRange.first,XFitRange.second);

			if( not fitresult->IsEmpty() ){
				AssignFitValuesErrors(fitresult);
				AssignFitFuncParams();
				if( this->debug ){
					this->fithist->GetListOfFunctions()->Add(this->fitfunc);

					this->components.at(0)->SetParameters(this->Results["Offset"]);
					this->fithist->GetListOfFunctions()->Add(this->components.at(0));

					this->components.at(1)->SetParameters(this->Results["Amp1"],this->Results["T01"],this->Results["Rise1"],this->Results["Fall1"]);
					this->fithist->GetListOfFunctions()->Add(this->components.at(1));

					this->components.at(2)->SetParameters(this->Results["Amp2"],this->Results["T02"],this->Results["Rise1"],this->Results["Fall1"]);
					this->fithist->GetListOfFunctions()->Add(this->components.at(2));

					TLine* centroid1 = new TLine(this->Results["T01"],0,
							this->Results["T01"],
							0.75*(this->fithist->GetBinContent(this->fithist->FindBin(this->Results["T01"]))));
					centroid1->SetLineColor(kAzure);
					this->fithist->GetListOfFunctions()->Add(centroid1);

					TLine* centroid2 = new TLine(this->Results["T02"],0,
							this->Results["T02"],
							0.75*(this->fithist->GetBinContent(this->fithist->FindBin(this->Results["T02"]))));
					centroid2->SetLineColor(kAzure);
					this->fithist->GetListOfFunctions()->Add(centroid2);
				}
			}
		}
	}

	void InitErfFit(){
		this->components = {
		};
		
		auto minbin = this->fithist->FindBin(this->XFitRange.first);
		auto maxbin = this->fithist->FindBin(this->XFitRange.second);

		double width = this->XFitRange.second - this->XFitRange.first;
		double offset = (this->XFitRange.second + this->XFitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));

		this->keys = {{"Area",{0,area}},{"Mean",{1,offset}},{"Sigma",{2,width}}};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		if( this->fvalues.find("Mean") == this->fvalues.end() and this->bvalues.find("Mean") == this->bvalues.end() ){
			this->bvalues["Mean"] = this->XFitRange; 
		}

		auto integral = this->fithist->Integral(minbin,maxbin);

		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",XFitRange.first,XFitRange.second);

			if( not fitresult->IsEmpty() ){
				AssignFitValuesErrors(fitresult);
				AssignFitFuncParams();
				this->fithist->GetListOfFunctions()->Add(this->fitfunc);

				TLine* gauss_centroid = new TLine(this->Results["Mean"],0,
						this->Results["Mean"],0.75*(this->fithist->GetBinContent(this->fithist->FindBin(this->Results["Mean"]))));
				gauss_centroid->SetLineColor(kAzure);
				this->fithist->GetListOfFunctions()->Add(gauss_centroid);
			}
		}
	}

	virtual void FixAndBoundParameters() final{
		for( const auto& kv : this->keys ){
			auto fres = this->fvalues.find(kv.first);
			auto bres = this->bvalues.find(kv.first);
			if( fres != this->fvalues.end() ){
				this->fitfunc->SetParameter(kv.second.first,fres->second);
				this->fitfunc->FixParameter(kv.second.first,fres->second);
			}else if( bres != this->bvalues.end() ){
				auto middle = (bres->second.first+bres->second.second)/2.0;
				this->fitfunc->SetParameter(kv.second.first,middle);
				this->fitfunc->SetParLimits(kv.second.first,bres->second.first,bres->second.second);
			}else{
				this->fitfunc->SetParameter(kv.second.first,kv.second.second);
			}
		}
	}

	virtual void AssignFitFuncParams() final{
		for( const auto& kv : this->keys ){
			this->fitfunc->SetParameter(kv.second.first,this->Results[kv.first]);
		}
		this->fithist->GetListOfFunctions()->Clear();
	}

	void AssignFitValuesErrors(const TFitResultPtr& fitresult){
		for( const auto& kv : this->keys ){
			this->Results[kv.first] = fitresult->Parameter(kv.second.first);
			this->Errors[kv.first] = fitresult->ParError(kv.second.first);
		}
		this->Results["Chi2"] = fitresult->Chi2();
		this->Errors["NDF"] = fitresult->Ndf();
	}

	virtual void AssignFitParNames() final{
		for( const auto& kv : this->keys ){
			this->fitfunc->SetParName(kv.second.first,kv.first.c_str());
		}
	}

	virtual void WriteHistogram(bool storechi2) final{
		this->fithist->Write(0,2,0);
		if( storechi2 ){
			auto name = std::string(this->fithist->GetName());
			name += "_chi2";
			TH1* residual = dynamic_cast<TH1*>(this->fithist->Clone(name.c_str()));
			residual->Reset("ICEMS");
			residual->SetYTitle("#sigma");
			residual->GetYaxis()->CenterTitle();
			
			name += "_distribution";
			TH1* chi2dist = new TH1F(name.c_str(),"Chi2 Distribution; #sigma; counts",1000,-10,10);
			chi2dist->GetXaxis()->CenterTitle();
			chi2dist->GetYaxis()->CenterTitle();

			int minbin = this->fithist->FindBin(this->XFitRange.first);
			int maxbin = this->fithist->FindBin(this->XFitRange.second);
			for( int ii = minbin; ii <= maxbin; ++ii ){
				double centroid = this->fithist->GetBinCenter(ii);
				auto fitval = this->fitfunc->Eval(centroid);
				auto hisval = this->fithist->GetBinContent(ii);
				auto hiserr = this->fithist->GetBinError(ii);
				auto uncert = hiserr;
				auto binchi2 = (fitval - hisval)*(fitval - hisval)/uncert/uncert;
				if( hisval > 0.0 ){
					if( (fitval - hisval) > 0.0 ){
						residual->SetBinContent(ii,TMath::Sqrt(binchi2));
						chi2dist->Fill(TMath::Sqrt(binchi2));
					}else{
						residual->SetBinContent(ii,-TMath::Sqrt(binchi2));
						chi2dist->Fill(-TMath::Sqrt(binchi2));
					}
					residual->SetBinError(ii,0);
				}
			}
			residual->Write(0,2,0);
			chi2dist->Write(0,2,0);
		}
	}

};

#endif
