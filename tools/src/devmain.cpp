#include "boost/histogram/axis/regular.hpp"
#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include <fstream>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>
#include <boost/histogram.hpp>
#include <boost/unordered_map.hpp>
#include <boost/sort/spreadsort/string_sort.hpp>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

struct TH1B{
	std::string name;
	std::string title;
	int nbinsx;
	float xlow;
	float xhigh;
	using axes_t = std::tuple<boost::histogram::axis::regular<>>;
	using hist_t = boost::histogram::histogram<axes_t>;
	hist_t internalHist;

	TH1B(const std::string& n,const std::string& t,int nb,float xl,float xh) : name(n), title(t), nbinsx(nb), xlow(xl), xhigh(xh){
		internalHist = boost::histogram::make_histogram(std::move(boost::histogram::axis::regular<>(nbinsx,xlow,xhigh, "x")));
	}

	template<class T>
	void Fill(const T& xval){
		internalHist(xval);
	}

	template<class T>
	void Fill(const T& xval,const T& w){
		internalHist(xval,boost::histogram::weight(w));
	}

	template<class T>
	void FillN(int ntimes,T* xval,T* w,int stride = 1){
		for( int ii = 0; ii < ntimes; ii += stride ){
			internalHist(xval[ii],boost::histogram::weight(w[ii]));
		}
	}

	void Write(int ii,int jj,int kk){
		TH1* curr = new TH1F(name.c_str(),title.c_str(),nbinsx,xlow,xhigh);
		curr->SetBinContent(0,internalHist.at(-1));
		for(int ii = 0; ii < internalHist.axis().size(); ++ii) {
			auto val = internalHist.at(ii);
			//spdlog::info("{} {} {} {}",ii,internalHist.axis().bin(ii).center(),curr->GetBinCenter(ii+1),val);
			curr->SetBinContent(ii+1,val);
			curr->SetBinError(ii+1,std::sqrt(val));
		}
		curr->SetBinContent(nbinsx+1,internalHist.at(nbinsx));
		curr->SaveAs("speedtest.root");
	}
};

struct TH2B{
	std::string name;
	std::string title;
	int nbinsx;
	float xlow;
	float xhigh;
	int nbinsy;
	float ylow;
	float yhigh;
	using axes_t = std::tuple<boost::histogram::axis::regular<>,boost::histogram::axis::regular<>>;
	using hist_t = boost::histogram::histogram<axes_t>;
	hist_t internalHist;

	TH2B(const std::string& n,const std::string& t,int nbx,float xl,float xh,int nby,float yl,float yh) : name(n), title(t), nbinsx(nbx), xlow(xl), xhigh(xh), nbinsy(nby), ylow(yl), yhigh(yh){
		internalHist = boost::histogram::make_histogram(std::move(boost::histogram::axis::regular<>(nbinsx,xlow,xhigh, "x")),std::move(boost::histogram::axis::regular<>(nbinsy,ylow,yhigh, "y")));
	}

	template<class T>
	void Fill(const T& xval,const T& yval){
		internalHist(xval,yval);
	}

	template<class T>
	void Fill(const T& xval,const T& yval,const T& w){
		internalHist(xval,yval,boost::histogram::weight(w));
	}

	template<class T>
	void FillN(int ntimes,T* xval,T* yval,T* w,int stride = 1){
		for( int ii = 0; ii < ntimes; ii += stride ){
			internalHist(xval[ii],yval[ii],boost::histogram::weight(w[ii]));
		}
	}

	void Write(int ii,int jj,int kk){
		TH2* curr = new TH2F(name.c_str(),title.c_str(),nbinsx,xlow,xhigh,nbinsy,ylow,yhigh);

		curr->SetBinContent(0,0,internalHist.at(-1,-1));
		curr->SetBinContent(nbinsx+1,0,internalHist.at(nbinsx,-1));
		curr->SetBinContent(nbinsx+1,nbinsy+1,internalHist.at(nbinsx,nbinsy));
		curr->SetBinContent(0,nbinsy+1,internalHist.at(-1,nbinsy));

		for( int ii = 0; ii < internalHist.axis(0).size(); ++ii ){
			curr->SetBinContent(ii+1,0,internalHist.at(ii,-1));
		}
		for( int jj = 0; jj < internalHist.axis(1).size(); ++jj ){
			curr->SetBinContent(0,jj+1,internalHist.at(-1,jj));
		}

		for(int ii = 0; ii < internalHist.axis(0).size(); ++ii) {
			for( int jj = 0; jj < internalHist.axis(1).size(); ++jj ){
				auto val = internalHist.at(ii,jj);
				curr->SetBinContent(ii+1,jj+1,val);
				curr->SetBinError(ii+1,jj+1,std::sqrt(val));
			}
		}

		for( int ii = 0; ii < internalHist.axis(0).size(); ++ii ){
			curr->SetBinContent(ii+1,nbinsy+1,internalHist.at(ii,nbinsy));
		}
		for( int jj = 0; jj < internalHist.axis(1).size(); ++jj ){
			curr->SetBinContent(nbinsx+1,jj+1,internalHist.at(nbinsx,jj));
		}


		curr->SaveAs("speedtest.root");
	}
};


int main(int argc, char *argv[]) {
	unsigned int MaxNumErg = 87916018;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("numval,n",boost::program_options::value<unsigned int>(&MaxNumErg)->default_value(1000000),"number of iterations for the fill command")
		;


	boost::program_options::positional_options_description p;

	try{
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
        	notify(vm);
		if( vm.count("help") or argc <= 2 ){
			spdlog::info(cmdline_options);
			exit(EXIT_SUCCESS);
		}
	}catch( std::exception& e){
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}    


	std::random_device rd;
	auto randGen = std::mt19937_64(rd());
	auto randNum = std::uniform_real_distribution<float>(-8192.0,8192.0);
	//const unsigned int MaxNumErg = 1000000;
	std::vector<float> erglistx;
	std::vector<float> erglisty;
	for( unsigned int ii = 0; ii < MaxNumErg; ++ii ){
		erglistx.push_back(randNum(randGen));
		erglisty.push_back(randNum(randGen));
	}

	TFile* dump = new TFile("totallynewfilename.root","RECREATE");

	TH1* RH1 = new TH1F("RH1","",2048,0,2048);
	TH2* RH2 = new TH2F("RH2","",2048,0,2048,2048,0,2048);

	auto BH1R = boost::histogram::make_histogram(boost::histogram::axis::regular<>(2048,0,2048, "x"));
	auto BH2R = boost::histogram::make_histogram(boost::histogram::axis::regular<>(2048,0,2048, "x"),boost::histogram::axis::regular<>(2048,0,2048,"y"));

	std::vector<boost::histogram::axis::regular<>> axes1 = { boost::histogram::axis::regular<>(2048,0,2048,"x") };
	std::vector<boost::histogram::axis::regular<>> axes2 = { boost::histogram::axis::regular<>(2048,0,2048,"x"),boost::histogram::axis::regular<>(2048,0,2048,"y") };
	
	auto BH1RD = boost::histogram::make_histogram(std::move(axes1));
	auto BH2RD = boost::histogram::make_histogram(std::move(axes2));

	TH1B* COMBO = new TH1B("BRH1","",2048,0,2048);
	TH2B* COMBO2 = new TH2B("BRH2","",2048,0,2048,16354,0,2048);

	auto start_time = std::chrono::high_resolution_clock::now();
	for( size_t ii = 0 ; ii < MaxNumErg; ++ii ){
		RH1->Fill(erglistx.at(ii));
	}
	//RH1->SaveAs("origdump.root");
	auto stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float,std::milli> dur_r1 = (stop_time - start_time);

	start_time = std::chrono::high_resolution_clock::now();
	for( size_t ii = 0 ; ii < MaxNumErg; ++ii ){
		BH1R(erglistx.at(ii));
	}
	stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float,std::milli> dur_b1 = (stop_time - start_time);

	for( size_t ii = 0 ; ii < MaxNumErg; ++ii ){
		BH1RD(erglistx.at(ii));
	}
	stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float,std::milli> dur_b1d = (stop_time - start_time);

	start_time = std::chrono::high_resolution_clock::now();
	for( size_t ii = 0 ; ii < MaxNumErg; ++ii ){
		RH2->Fill(erglistx.at(ii),erglisty.at(ii));
	}
	stop_time = std::chrono::high_resolution_clock::now();
	//RH2->SaveAs("dumproot.root");
	std::chrono::duration<float,std::milli> dur_r2 = (stop_time - start_time);

	start_time = std::chrono::high_resolution_clock::now();
	for( size_t ii = 0 ; ii < MaxNumErg; ++ii ){
		BH2R(erglistx.at(ii),erglisty.at(ii));
	}
	stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float,std::milli> dur_b2 = (stop_time - start_time);

	start_time = std::chrono::high_resolution_clock::now();
	for( size_t ii = 0 ; ii < MaxNumErg; ++ii ){
		BH2RD(erglistx.at(ii),erglisty.at(ii));
	}
	stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float,std::milli> dur_b2d = (stop_time - start_time);

	start_time = std::chrono::high_resolution_clock::now();
	for( size_t ii = 0 ; ii < MaxNumErg; ++ii ){
		COMBO->Fill(erglistx.at(ii));
	}
	//COMBO->Write(0,2,0);
	stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float,std::milli> dur_br1d = (stop_time - start_time);

	start_time = std::chrono::high_resolution_clock::now();
	for( size_t ii = 0 ; ii < MaxNumErg; ++ii ){
		COMBO2->Fill(erglistx.at(ii),erglisty.at(ii));
	}
	stop_time = std::chrono::high_resolution_clock::now();
	//COMBO2->Write(0,2,0);
	std::chrono::duration<float,std::milli> dur_br2d = (stop_time - start_time);
	dump->Close();


	spdlog::info("NFills : {} | 1D -> Boost : {} | 1D -> Boost (dyn.) : {} | 1D-> Root : {} | Combo : {}",MaxNumErg,dur_b1.count(),dur_b1d.count(),dur_r1.count(),dur_br1d.count());
	spdlog::info("NFills : {} | 2D -> Boost : {} | 2D -> Boost (dyn.) : {} | 2D-> Root : {} | Combo : {}",MaxNumErg,dur_b2.count(),dur_b2d.count(),dur_r2.count(),dur_br2d.count());

}
