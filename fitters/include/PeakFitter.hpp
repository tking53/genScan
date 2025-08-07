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
	std::map<std::string,double> fvalues;
	std::map<std::string,std::pair<double,double>> bvalues;
	TF1* fitfunc;
	std::vector<TF1*> components;
	std::map<std::string,std::pair<int,double>> keys;
	std::map<std::string,double> Results;
	std::map<std::string,double> Errors;


	PeakFitter(double l,double u,bool chi2,int mode,TH1* hist,
			const std::map<std::string,double>& fixedvalues,const std::map<std::string,std::pair<double,double>>& boundedvalues) 
		: FitRange(l,u), loglikelihood(!chi2),fithist(hist),fvalues(fixedvalues),bvalues(boundedvalues){
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

		double bkg_offset = 0;
		double bkg_slope = 0;
		double width = this->FitRange.second - this->FitRange.first;
		double offset = (this->FitRange.second + this->FitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));

		this->keys = { {"Area",{0,area}},{"Mean",{1,offset}},{"Sigma",{2,width}},{"BkgOffset",{3,bkg_offset}},{"BkgSlope",{4,bkg_slope}}};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		if( this->fvalues.find("Mean") == this->fvalues.end() and this->bvalues.find("Mean") == this->bvalues.end() ){
			this->bvalues["Mean"] = this->FitRange; 
		}

		auto minbin = this->fithist->FindBin(this->FitRange.first);
		auto maxbin = this->fithist->FindBin(this->FitRange.second);
		auto integral = this->fithist->Integral(minbin,maxbin);

		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",FitRange.first,FitRange.second);

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

		this->fitfunc = new TF1("GaussNErfBkg",&PulseFit::GaussNErfBkg,FitRange.first,FitRange.second,6);
		this->fitfunc->SetLineColor(kRed);
		this->components = {
			new TF1("GaussN",&PulseFit::GaussN,FitRange.first,FitRange.second,3),
			new TF1("ErfBkg",&PulseFit::GaussErf,FitRange.first,FitRange.second,3),
			new TF1("LinBkg",&PulseFit::Quad,FitRange.first,FitRange.second,3)
		};
		this->components.at(0)->SetLineColor(kMagenta);
		this->components.at(1)->SetLineColor(kGreen);
		this->components.at(2)->SetLineColor(kViolet);

		double cbkg = 0;
		double sbkg = 0;
		double width = this->FitRange.second - this->FitRange.first;
		double offset = (this->FitRange.second + this->FitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));
		double ca = area;

		this->keys = { {"Area",{0,area}},{"Mean",{1,offset}},{"Sigma",{2,width}},{"ComptonArea",{3,ca}},{"BkgOffset",{4,cbkg}},{"BkgSlope",{5,sbkg}}};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		if( this->fvalues.find("Mean") == this->fvalues.end() and this->bvalues.find("Mean") == this->bvalues.end() ){
			this->bvalues["Mean"] = this->FitRange; 
		}

		auto minbin = this->fithist->FindBin(this->FitRange.first);
		auto maxbin = this->fithist->FindBin(this->FitRange.second);
		auto integral = this->fithist->Integral(minbin,maxbin);

		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",FitRange.first,FitRange.second);

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

		this->fitfunc = new TF1("SingleTailingGaussN",&PulseFit::SingleTailingGaussN,FitRange.first,FitRange.second,4);
		this->fitfunc->SetLineColor(kRed);
		this->components = {
		};
		
		auto minbin = this->fithist->FindBin(this->FitRange.first);
		auto maxbin = this->fithist->FindBin(this->FitRange.second);

		double width = this->FitRange.second - this->FitRange.first;
		double offset = (this->FitRange.second + this->FitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));
		double tau = this->fithist->GetBinContent(maxbin) - this->fithist->GetBinContent(minbin);

		this->keys = {{"Area",{0,area}},{"Mean",{1,offset}},{"Sigma",{2,width}},{"Tau",{3,tau}}};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		if( this->fvalues.find("Mean") == this->fvalues.end() and this->bvalues.find("Mean") == this->bvalues.end() ){
			this->bvalues["Mean"] = this->FitRange; 
		}

		auto integral = this->fithist->Integral(minbin,maxbin);

		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",FitRange.first,FitRange.second);

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

		this->fitfunc = new TF1("DoubleTailingGaussN",&PulseFit::DoubleTailingGaussN,FitRange.first,FitRange.second,8);
		this->fitfunc->SetLineColor(kRed);
		this->components = {
			new TF1("TailingGaussN1",&PulseFit::SingleTailingGaussN,FitRange.first,FitRange.second,4),
			new TF1("TailingGaussN2",&PulseFit::SingleTailingGaussN,FitRange.first,FitRange.second,4)
		};
		this->components.at(0)->SetLineColor(kMagenta);
		this->components.at(1)->SetLineColor(kGreen);
		
		auto minbin = this->fithist->FindBin(this->FitRange.first);
		auto maxbin = this->fithist->FindBin(this->FitRange.second);

		double width = this->FitRange.second - this->FitRange.first;
		double offset = (this->FitRange.second + this->FitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));
		double tau = this->fithist->GetBinContent(maxbin) - this->fithist->GetBinContent(minbin);

		this->keys = {{"Area1",{0,area}},{"Mean1",{1,offset}},{"Sigma1",{2,width}},{"Tau1",{3,tau}},{"Area2",{4,area}},{"Mean2",{5,offset}},{"Sigma2",{6,width}},{"Tau2",{7,tau}}};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		if( this->fvalues.find("Mean1") == this->fvalues.end() and this->bvalues.find("Mean1") == this->bvalues.end() ){
			this->bvalues["Mean1"] = this->FitRange; 
		}
		if( this->fvalues.find("Mean2") == this->fvalues.end() and this->bvalues.find("Mean2") == this->bvalues.end() ){
			this->bvalues["Mean2"] = this->FitRange; 
		}

		auto integral = this->fithist->Integral(minbin,maxbin);

		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",FitRange.first,FitRange.second);

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

		this->fitfunc = new TF1("SingleTailingGaussNLinBkg",&PulseFit::SingleTailingGaussNLinBkg,FitRange.first,FitRange.second,6);
		this->fitfunc->SetLineColor(kRed);
		this->components = {
			new TF1("TailingGaussN1",&PulseFit::SingleTailingGaussN,FitRange.first,FitRange.second,4),
			new TF1("LinBkg",&PulseFit::Linear,FitRange.first,FitRange.second,2)
		};
		this->components.at(0)->SetLineColor(kMagenta);
		this->components.at(1)->SetLineColor(kGreen);
		
		auto minbin = this->fithist->FindBin(this->FitRange.first);
		auto maxbin = this->fithist->FindBin(this->FitRange.second);

		double width = this->FitRange.second - this->FitRange.first;
		double offset = (this->FitRange.second + this->FitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));
		double tau = this->fithist->GetBinContent(maxbin) - this->fithist->GetBinContent(minbin);
		double bkgoffset = 0.0;
		double bkgslope = 0.0;

		this->keys = {{"Area",{0,area}},{"Mean",{1,offset}},{"Sigma",{2,width}},{"Tau",{3,tau}},{"BkgOffset",{4,bkgoffset}},{"BkgSlope",{5,bkgslope}}};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		if( this->fvalues.find("Mean") == this->fvalues.end() and this->bvalues.find("Mean") == this->bvalues.end() ){
			this->bvalues["Mean"] = this->FitRange; 
		}

		auto integral = this->fithist->Integral(minbin,maxbin);

		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",FitRange.first,FitRange.second);

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

		this->fitfunc = new TF1("Erf",&PulseFit::Erf,FitRange.first,FitRange.second,3);
		this->fitfunc->SetLineColor(kRed);
		this->components = {
		};
		
		auto minbin = this->fithist->FindBin(this->FitRange.first);
		auto maxbin = this->fithist->FindBin(this->FitRange.second);

		double width = this->FitRange.second - this->FitRange.first;
		double offset = (this->FitRange.second + this->FitRange.first)/2.0;
		double area = this->fithist->GetBinContent(this->fithist->FindBin(offset));

		this->keys = {{"Area",{0,area}},{"Mean",{1,offset}},{"Sigma",{2,width}}};
		AssignFitParNames();
		VerifyFixedValues();
		VerifyBoundedValues();
		if( this->fvalues.find("Mean") == this->fvalues.end() and this->bvalues.find("Mean") == this->bvalues.end() ){
			this->bvalues["Mean"] = this->FitRange; 
		}

		auto integral = this->fithist->Integral(minbin,maxbin);

		FixAndBoundParameters();

		if( integral > 0.0 ){
			std::string option = "0SQ";
			if( this->loglikelihood ){
				option+="L";
			}
			TFitResultPtr fitresult = this->fithist->Fit(this->fitfunc,option.c_str(),"",FitRange.first,FitRange.second);

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

	void FixAndBoundParameters(){
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

	void VerifyFixedValues(){
		for( const auto& kv : this->fvalues ){
			if( this->keys.find(kv.first) == this->keys.end() ){
				throw std::runtime_error("Unknown parameter name "+kv.first);
			}
		}
	}

	void VerifyBoundedValues(){
		for( const auto& kv : this->bvalues ){
			if( this->keys.find(kv.first) == this->keys.end() ){
				throw std::runtime_error("Unknown parameter name "+kv.first);
			}
			if( kv.second.second < kv.second.first ){
				throw std::runtime_error("Bounded parameter has lowerbound higher than upperbound");
			}
		}
	}

	void AssignFitFuncParams(){
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

	void AssignFitParNames(){
		for( const auto& kv : this->keys ){
			this->fitfunc->SetParName(kv.second.first,kv.first.c_str());
		}
	}

	void WriteHistogram(bool storechi2){
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

			int minbin = this->fithist->FindBin(this->FitRange.first);
			int maxbin = this->fithist->FindBin(this->FitRange.second);
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

	template<typename OStream>
	friend OStream& operator<<(OStream& os, const PeakFitter& fitinfo) {
		for( const auto& kv : fitinfo.keys ){
			os << kv.first << " : " << fitinfo.Results.at(kv.first) << " +- " << fitinfo.Errors.at(kv.first) << " \t ";
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
