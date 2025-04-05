#ifndef __ROOT_FITTER_HPP__
#define __ROOT_FITTER_HPP__

#include <stdexcept>
#include <vector>
#include <tuple>

#include <pugixml.hpp>

#include "TF1.h"

#include "PulseFitFunctions.hpp"

struct RootFitter{
	std::vector<std::tuple<int,bool,bool,std::string,double,double,double>> ParamInfo;
	std::pair<double,double> FitRange;
	std::string FitFuncName;
	TF1* fitfunc;
	TH1* fithist;

	RootFitter(const pugi::xml_node& fitsettings){
		std::string fitname = fitsettings.attribute("name").as_string("");
		this->FitFuncName = fitname;

		double fitlowbound = fitsettings.attribute("min").as_double(0.0);
		double fithighbound = fitsettings.attribute("max").as_double(0.0);
		this->FitRange = {fitlowbound,fithighbound};

		for( pugi::xml_node fitparam = fitsettings.child("FitParam"); fitparam; fitparam = fitparam.next_sibling("FitParam") ){
			int idx = fitparam.attribute("idx").as_int(-1);
			double value = fitparam.attribute("value").as_double(0.0);
			bool isbounded = fitparam.attribute("bounded").as_bool(false);
			bool isfixed = fitparam.attribute("fixed").as_bool(false);
			std::string parname = fitparam.attribute("name").as_string("");
			if( isbounded ){
				double lowbound = fitparam.attribute("min").as_double(0.0);
				double highbound = fitparam.attribute("max").as_double(0.0);
				if( lowbound >= highbound or value < lowbound or value > highbound ){
					throw std::runtime_error("Bounded value not between lowbound and highbound");
				}
				this->ParamInfo.push_back({idx,isbounded,isfixed,parname,value,lowbound,highbound});
			}
			if( isfixed ){
				this->ParamInfo.push_back({idx,isbounded,isfixed,parname,value,value,value});
			}
			if( idx < 0 ){
				throw std::runtime_error("idx not assigned to fit parameter");
			}

			if( isfixed and isbounded ){
				throw std::runtime_error("Value is set to both bounded and fixed");
			}
		}
		std::sort(this->ParamInfo.begin(),this->ParamInfo.end(),[](const std::tuple<int,bool,bool,std::string,double,double,double>& a,const std::tuple<int,bool,bool,std::string,double,double,double>& b){ return std::get<0>(a) < std::get<0>(b); });
		this->fithist = nullptr;
		this->fitfunc = nullptr;
		if( fitname.compare("BSMSingleTracePulse") == 0 ){
			this->fitfunc = new TF1(fitname.c_str(),PulseFit::BSMSingleTraceFit,fitlowbound,fithighbound,8);
		}else if( fitname.compare("SingleTracePulse") == 0 ){
			this->fitfunc = new TF1(fitname.c_str(),PulseFit::SingleTraceFit,fitlowbound,fithighbound,5);
		}else{
			throw std::runtime_error("Unknown TraceFitting Function");
		}
		for( const auto& parinfo : this->ParamInfo ){
			this->fitfunc->SetParName(std::get<0>(parinfo),std::get<3>(parinfo).c_str());
		}
		this->fithist = nullptr;
	}

	~RootFitter() = default;

	RootFitter(const RootFitter&) = default;
	RootFitter(RootFitter&&) = default;
	RootFitter& operator=(const RootFitter&) = default;
	RootFitter& operator=(RootFitter&&) = default;
};

#endif
