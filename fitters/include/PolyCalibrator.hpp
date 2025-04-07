#ifndef __POLY_CALIBRATOR_HPP__
#define __POLY_CALIBRATOR_HPP__

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <set>

#include "Rtypes.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TList.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"

#include "TMath.h"

struct calibrationpoint{
	std::pair<double,double> energy;
	std::pair<double,double> channel;
};

struct PolyCalibrator{
	bool loglikelihood;
	TGraphErrors* fithist;
	TF1* fitfunc;
	std::map<std::string,double> Results;
	std::map<std::string,double> Errors;
	std::set<std::string> keys;
	double chi2;
	double ndf;
	std::string FitName;

	PolyCalibrator(const std::vector<calibrationpoint>& cp,bool fixcontstant,int order,const std::string& gchid){
		if( cp.size() == 0 ){
			throw std::runtime_error("Not enough points to calibrate with");
		}
		FitName = gchid;

		this->fithist = new TGraphErrors(cp.size());
		int idx = 0;
		auto minx = cp.begin()->channel.first;
		auto maxx = cp.begin()->channel.first;
		for( const auto& p : cp ){
			this->fithist->SetPoint(idx,p.channel.first,p.energy.first);
			this->fithist->SetPointError(idx,p.channel.second,p.energy.second);
			if( p.channel.first < minx ){
				minx = p.channel.first;
			}
			if( p.channel.first > maxx ){
				maxx = p.channel.first;
			}
			++idx;
		}

		this->fithist->SetLineColor(kBlack);
		this->fitfunc = new TF1("ArbPoly",
				[&order](double* x,double* par){
					double sum = 0.0;
					for( int ii = 0; ii < order+1; ++ii ){
						sum += TMath::Power(x[0],ii)*par[ii];
					}
					return sum;
				},
				minx-1.0,maxx+1.0,order+1);
		this->fitfunc->SetLineColor(kRed);

		if( fixcontstant ){
			this->fitfunc->FixParameter(0,0.0);
		}


		std::string option = "0SQB";
		TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",minx-1.0,maxx+1.0);

		if( not fitresult->IsEmpty() ){
			this->fithist->GetListOfFunctions()->Add(this->fitfunc);

			this->chi2 = fitresult->Chi2();
			this->ndf = fitresult->Ndf();
			for( int ii = 0; ii < order+1; ++ii ){
				auto key = "p"+std::to_string(ii);
				this->keys.insert(key);
				this->Results[key] = fitresult->Parameter(ii);
				this->Errors[key] = fitresult->ParError(ii);
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
	friend OStream& operator<<(OStream& os, const PolyCalibrator& fitinfo) {
		for( const auto& k : fitinfo.keys ){
			os << k << " : " << fitinfo.Results.at(k) << " +- " << fitinfo.Errors.at(k) << " \t ";
		}
		os << "Chi2/NDF : " << fitinfo.chi2 << "/" << fitinfo.ndf;
		return os;
	}

	std::pair<double,double> operator [](const std::string& key) const{
		return {this->Results.at(key),this->Errors.at(key)};
	}
	virtual ~PolyCalibrator() = default;

	PolyCalibrator(const PolyCalibrator&) = default;
	PolyCalibrator(PolyCalibrator&&) = default;
	PolyCalibrator& operator=(const PolyCalibrator&) = default;
	PolyCalibrator& operator=(PolyCalibrator&&) = default;
};


#endif
