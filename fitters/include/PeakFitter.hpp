#ifndef __PEAK_FITTER_HPP__
#define __PEAK_FITTER_HPP__

#include <stdexcept>
#include <vector>
#include <utility>
#include <map>
#include <set>

#include "Rtypes.h"
#include "TF1.h"
#include "TH1.h"
#include "TList.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TLine.h"

#include "PulseFitFunctions.hpp"

struct PeakFitter{
	std::pair<double,double> FitRange;
	bool loglikelihood;
	TH1* fithist;
	TF1* fitfunc;
	std::vector<TF1*> components;
	std::set<std::string> keys;
	std::map<std::string,double> Results;
	std::map<std::string,double> Errors;


	PeakFitter(double l,double u,bool chi2,int mode,TH1* hist): FitRange(l,u), loglikelihood(!chi2),fithist(hist){
		if( mode == 0){
			this->InitGaussNLinBkgFit();
		}else{
			throw std::runtime_error("Unknown peak fitting mode");
		}
	}

	void InitGaussNLinBkgFit(){
		this->fithist->SetLineColor(kBlack);

		this->fitfunc = new TF1("GaussNLinBkg",&PulseFit::GaussNLinBkg,FitRange.first,FitRange.second,5);
		this->fitfunc->SetLineColor(kRed);
		this->components = {
			new TF1("GaussN",&PulseFit::GaussN,FitRange.first,FitRange.second,3),
			new TF1("LinBkg",&PulseFit::Linear,FitRange.first,FitRange.second,2)
		};
		this->components.at(0)->SetLineColor(kMagenta);
		this->components.at(1)->SetLineColor(kGreen);

		this->fitfunc->SetParName(0,"Area");
		this->fitfunc->SetParName(1,"Mean");
		this->fitfunc->SetParName(2,"Sigma");
		this->fitfunc->SetParName(3,"BkgOffset");
		this->fitfunc->SetParName(4,"BkgSlope");

		this->keys = { "Area","Mean","Sigma","BkgSlope","BkgOffset"};

		double bkg_offset = 0;
		double bkg_slope = 0;
		double width = this->FitRange.second - this->FitRange.first;
		double offset = (this->FitRange.second + this->FitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));

		auto minbin = this->fithist->FindBin(this->FitRange.first);
		auto maxbin = this->fithist->FindBin(this->FitRange.second);
		auto integral = this->fithist->Integral(minbin,maxbin);

		this->fitfunc->SetParameters(area,offset,width,bkg_offset,bkg_slope);
		this->fitfunc->SetParLimits(1,this->FitRange.first,this->FitRange.second);

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",FitRange.first,FitRange.second);

			if( not fitresult->IsEmpty() ){
				this->Results["Area"] = fitresult->Parameter(0);
				this->Results["Mean"] = fitresult->Parameter(1);
				this->Results["Sigma"] = fitresult->Parameter(2);
				this->Results["BkgOffset"] = fitresult->Parameter(3);
				this->Results["BkgSlope"] = fitresult->Parameter(4);
				this->Results["Chi2"] = fitresult->Chi2();
				
				this->Errors["Area"] = fitresult->ParError(0);
				this->Errors["Mean"] = fitresult->ParError(1);
				this->Errors["Sigma"] = fitresult->ParError(2);
				this->Errors["BkgOffset"] = fitresult->ParError(3);
				this->Errors["BkgSlope"] = fitresult->ParError(4);
				this->Errors["NDF"] = fitresult->Ndf();

				this->fitfunc->SetParameters(this->Results["Area"],this->Results["Mean"],this->Results["Sigma"]
						,this->Results["BkgOffset"],this->Results["BkgSlope"]);

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

	void WriteHistogram(){
		this->fithist->Write(0,2,0);
		//auto ressingle = new TRatioPlot(this->fithist,"errfunc");
		//ressingle->SetSeparationMargin(0.0);
		//ressingle->SetH1DrawOpt("E1");
		//ressingle->SetGraphDrawOpt("P X0");
		//ressingle->Draw("nogrid noconfint");
		//ressingle->Write(0,2,0);
	}

	template<typename OStream>
	friend OStream& operator<<(OStream& os, const PeakFitter& fitinfo) {
		for( const auto& k : fitinfo.keys ){
			os << k << " : " << fitinfo.Results.at(k) << " +- " << fitinfo.Errors.at(k) << " \t ";
		}
		os << "Chi2/NDF : " << fitinfo.Results.at("Chi2") << "/" << fitinfo.Errors.at("NDF");
		return os;
	}

	std::pair<double,double> operator [](const std::string& key) const{
		return {this->Results.at(key),this->Errors.at(key)};
	}
	
	virtual ~PeakFitter() = default;

	PeakFitter(const PeakFitter&) = default;
	PeakFitter(PeakFitter&&) = default;
	PeakFitter& operator=(const PeakFitter&) = default;
	PeakFitter& operator=(PeakFitter&&) = default;
};

#endif
