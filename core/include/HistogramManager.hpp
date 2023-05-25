#ifndef __HISTOGRAM_MANAGER_HPP__
#define __HISTOGRAM_MANAGER_HPP__

#include <map>
#include <string>

#include "TH1.h"
#include "TH2.h"

class HistogramManager{
	public:
		static void DeclareHistogramTH1I(unsigned long long,std::string,std::string,int,double,double);
		static void DeclareHistogramTH1F(unsigned long long,std::string,std::string,int,double,double);
		//static void DeclareHistogramTH1D(unsigned long long,std::string,std::string,int,double,double);
		
		static void DeclareHistogramTH2I(unsigned long long,std::string,std::string,int,double,double,int,double,double);
		static void DeclareHistogramTH2F(unsigned long long,std::string,std::string,int,double,double,int,double,double);
		//static void DeclareHistogramTH2D(unsigned long long,std::string,std::string,int,double,double,int,double,double);

		static void Fill(unsigned long long,double&);
		static void Fill(unsigned long long,double&,double&);
		
		static void FillWeighted(unsigned long long,double&,double&);
		static void FillWeighted(unsigned long long,double&,double&,double&);

		static void Initialize();
	private:
		HistogramManager();
		static HistogramManager* instance;
		std::map<unsigned long long,TH1*> Hist_1D;
		std::map<unsigned long long,TH2*> Hist_2D;
};

#endif
