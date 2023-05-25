#include <stdexcept>
#include <sstream>

#include "HistogramManager.hpp"

HistogramManager* HistogramManager::instance = nullptr;

HistogramManager::HistogramManager(){
}

void HistogramManager::Initialize(){
	if( instance != nullptr ){
		throw std::runtime_error("HistogramManager::Initialize() second attempt at Initialization"); 
	}else{
		instance = new HistogramManager();
	}
}

void HistogramManager::DeclareHistogramTH1I(unsigned long long hisid,std::string name,std::string title,int xbins,double xmin,double xmax){
	if( instance->Hist_1D.find(hisid) != instance->Hist_1D.end() ){
		std::stringstream ss;
		ss << "HistogramManager::DeclareHistogramTH1I() histogram with the id \""
		   << hisid << "\" already exists.";
		throw std::runtime_error(ss.str());
	}else{
		instance->Hist_1D[hisid] = new TH1I(name.c_str(),title.c_str(),xbins,xmin,xmax);
	}
}

void HistogramManager::DeclareHistogramTH1F(unsigned long long hisid,std::string name,std::string title,int xbins,double xmin,double xmax){
	if( instance->Hist_1D.find(hisid) != instance->Hist_1D.end() ){
		std::stringstream ss;
		ss << "HistogramManager::DeclareHistogramTH1F() histogram with the id \""
		   << hisid << "\" already exists.";
		throw std::runtime_error(ss.str());
	}else{
		instance->Hist_1D[hisid] = new TH1F(name.c_str(),title.c_str(),xbins,xmin,xmax);
	}
}

//void HistogramManager::DeclareHistogramTH1D(unsigned long long hisid,std::string name,std::string title,int xbins,double xmin,double xmax){
//	if( instance->Hist_1D.find(hisid) != instance->Hist_1D.end() ){
//		std::stringstream ss;
//		ss << "HistogramManager::DeclareHistogramTH1D() histogram with the id \""
//		   << hisid << "\" already exists.";
//		throw std::runtime_error(ss.str());
//	}else{
//		instance->Hist_1D[hisid] = new TH1D(name.c_str(),title.c_str(),xbins,xmin,xmax);
//	}
//}

void HistogramManager::DeclareHistogramTH2I(unsigned long long hisid,std::string name,std::string title,int xbins,double xmin,double xmax,int ybins,double ymin,double ymax){
	if( instance->Hist_2D.find(hisid) != instance->Hist_2D.end() ){
		std::stringstream ss;
		ss << "HistogramManager::DeclareHistogramTH2I() histogram with the id \""
		   << hisid << "\" already exists.";
		throw std::runtime_error(ss.str());
	}else{
		instance->Hist_2D[hisid] = new TH2I(name.c_str(),title.c_str(),xbins,xmin,xmax,ybins,ymin,ymax);
	}
}

void HistogramManager::DeclareHistogramTH2F(unsigned long long hisid,std::string name,std::string title,int xbins,double xmin,double xmax,int ybins,double ymin,double ymax){
	if( instance->Hist_2D.find(hisid) != instance->Hist_2D.end() ){
		std::stringstream ss;
		ss << "HistogramManager::DeclareHistogramTH2F() histogram with the id \""
		   << hisid << "\" already exists.";
		throw std::runtime_error(ss.str());
	}else{
		instance->Hist_2D[hisid] = new TH2F(name.c_str(),title.c_str(),xbins,xmin,xmax,ybins,ymin,ymax);
	}
}

//void HistogramManager::DeclareHistogramTH2D(unsigned long long hisid,std::string name,std::string title,int xbins,double xmin,double xmax,int ybins,double ymin,double ymax){
//	if( instance->Hist_2D.find(hisid) != instance->Hist_2D.end() ){
//		std::stringstream ss;
//		ss << "HistogramManager::DeclareHistogramTH2D() histogram with the id \""
//		   << hisid << "\" already exists.";
//		throw std::runtime_error(ss.str());
//	}else{
//		instance->Hist_2D[hisid] = new TH2D(name.c_str(),title.c_str(),xbins,xmin,xmax,ybins,ymin,ymax);
//	}
//}

void HistogramManager::Fill(unsigned long long hisid,double& val){
	try{
		instance->Hist_1D.at(hisid)->Fill(val);
	}catch( std::out_of_range const& e ){
		std::stringstream ss;
		ss << "HistogramManager::Fill() 1D histogram with the id \""
		   << hisid << "\" does not exist.";
		throw std::runtime_error(ss.str());
	}
}

void HistogramManager::Fill(unsigned long long hisid,double& val1,double& val2){
	try{
		instance->Hist_2D.at(hisid)->Fill(val1,val2);
	}catch( std::out_of_range const& e ){
		std::stringstream ss;
		ss << "HistogramManager::Fill() 2D histogram with the id \""
		   << hisid << "\" does not exist.";
		throw std::runtime_error(ss.str());
	}
}

void HistogramManager::FillWeighted(unsigned long long hisid,double& val,double& weight){
	try{
		instance->Hist_1D.at(hisid)->Fill(val,weight);
	}catch( std::out_of_range const& e ){
		std::stringstream ss;
		ss << "HistogramManager::Fill() 1D histogram with the id \""
		   << hisid << "\" does not exist.";
		throw std::runtime_error(ss.str());
	}
}

void HistogramManager::FillWeighted(unsigned long long hisid,double& val1,double& val2,double& weight){
	try{
		instance->Hist_2D.at(hisid)->Fill(val1,val2,weight);
	}catch( std::out_of_range const& e ){
		std::stringstream ss;
		ss << "HistogramManager::Fill() 2D histogram with the id \""
		   << hisid << "\" does not exist.";
		throw std::runtime_error(ss.str());
	}
}
