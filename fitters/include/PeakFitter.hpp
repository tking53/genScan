#ifndef __PEAK_FITTER_HPP__
#define __PEAK_FITTER_HPP__

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

struct PeakFitter{
	bool loglikelihood;
	std::map<std::string,double> fvalues;
	std::map<std::string,std::pair<double,double>> bvalues;
	std::map<std::string,std::pair<int,double>> keys;
	std::map<std::string,double> Results;
	std::map<std::string,double> Errors;

	PeakFitter(bool chi2,const std::map<std::string,double>& fixedvalues,const std::map<std::string,std::pair<double,double>>& boundedvalues) 
		: loglikelihood(!chi2),fvalues(fixedvalues),bvalues(boundedvalues)
	{
	}
	
	virtual ~PeakFitter() = default;

	PeakFitter(const PeakFitter&) = default;
	PeakFitter(PeakFitter&&) = default;
	PeakFitter& operator=(const PeakFitter&) = default;
	PeakFitter& operator=(PeakFitter&&) = default;

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

	PeakFitter2D(double xl,double xu,double yl,double yu,bool chi2,int mode,TH2* hist,
			const std::map<std::string,double>& fixedvalues,const std::map<std::string,std::pair<double,double>>& boundedvalues,double ellipse,int npts) 
		: XFitRange(xl,xu), YFitRange(yl,yu), fithist(hist), PeakFitter(chi2,fixedvalues,boundedvalues){
		for( const auto& kv : fvalues ){
			if( bvalues.find(kv.first) != bvalues.end() ){
				throw std::runtime_error("Parameter is both fixed and bounded");
			}
		}
		if( mode == 1000){
			this->InitBiGaussFit(ellipse,npts);
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

	PeakFitter1D(double l,double u,bool chi2,int mode,TH1* hist,
			const std::map<std::string,double>& fixedvalues,const std::map<std::string,std::pair<double,double>>& boundedvalues) 
		: XFitRange(l,u),fithist(hist),PeakFitter(chi2,fixedvalues,boundedvalues){
		for( const auto& kv : fvalues ){
			if( bvalues.find(kv.first) != bvalues.end() ){
				throw std::runtime_error("Parameter is both fixed and bounded");
			}
		}
		if( mode == 0){
			this->InitGaussNLinBkgFit();
		}else if( mode == 1 ){
			this->InitGaussNErfBkgFit();
		}else if( mode == 2 ){
			this->InitSingleTailingGaussNFit();
		}else if( mode == 3 ){
			this->InitDoubleTailingGaussNFit();
		}else if( mode == 4 ){
			this->InitSingleTailingGaussNLinBkgFit();
		}else if( mode == 5 ){
			this->InitErfFit();
		}else if( mode == 500 ){
			this->InitSimpleImplantationCurveFit();
		}else{
			throw std::runtime_error("Unknown peak fitting mode");
		}
	}


	void InitSimpleImplantationCurveFit(){
		this->fithist->SetLineColor(kBlack);

		this->fitfunc = new TF1("SimpleImplantationCurve",&PeakFit::SimpleImplantationCurve,XFitRange.first,XFitRange.second,3);
		this->fitfunc->SetLineColor(kRed);
		this->components = {
			new TF1("Constant",&CommonFit::Constant,XFitRange.first,XFitRange.second,1),
			new TF1("SimpleHalfLife",&PeakFit::SimpleHalfLife,0.0,XFitRange.second,2)
		};
		this->components.at(0)->SetLineColor(kMagenta);
		this->components.at(1)->SetLineColor(kGreen);

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
	
	void InitGaussNLinBkgFit(){
		this->fithist->SetLineColor(kBlack);

		this->fitfunc = new TF1("GaussNLinBkg",&PeakFit::GaussNLinBkg,XFitRange.first,XFitRange.second,5);
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
		this->fithist->SetLineColor(kBlack);

		this->fitfunc = new TF1("GaussNErfBkg",&PeakFit::GaussNErfBkg,XFitRange.first,XFitRange.second,6);
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
		this->fithist->SetLineColor(kBlack);

		this->fitfunc = new TF1("SingleTailingGaussN",&PeakFit::SingleTailingGaussN,XFitRange.first,XFitRange.second,4);
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
		this->fithist->SetLineColor(kBlack);

		this->fitfunc = new TF1("DoubleTailingGaussN",&PeakFit::DoubleTailingGaussN,XFitRange.first,XFitRange.second,8);
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
		this->fithist->SetLineColor(kBlack);

		this->fitfunc = new TF1("SingleTailingGaussNLinBkg",&PeakFit::SingleTailingGaussNLinBkg,XFitRange.first,XFitRange.second,6);
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

	void InitErfFit(){
		this->fithist->SetLineColor(kBlack);

		this->fitfunc = new TF1("Erf",&PeakFit::Erf,XFitRange.first,XFitRange.second,3);
		this->fitfunc->SetLineColor(kRed);
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
