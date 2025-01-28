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

#include <TH1.h>
#include <TH2.h>

namespace BOOSTPLOTS{
	const int S0 = 1;
	const int S1 = 2;
	const int S2 = 4;
	const int S3 = 8;
	const int S4 = 16;
	const int S5 = 32;
	const int S6 = 64;
	const int S7 = 128;
	const int S8 = 256;
	const int S9 = 512;
	const int SA = 1024;
	const int SB = 2048;
	const int SC = 4096;
	const int SD = 8192;
	const int SE = 16384;
	const int SF = 32768;
	const int SG = 65536;

	class PlotRegistry{
		public:
			PlotRegistry(const std::string& log,const std::string oup,int PORT=9090){
				this->LogName = log;
				this->outputprefix = oup;
				this->console = spdlog::get(this->LogName)->clone("HistogramManager");
				this->FillCounter1D = 0;
				this->FillCounter2D = 0;
				this->KeepListen = true;
			}

			~PlotRegistry(){
				this->console->info("Num 1D Fills : {} | Num 2D Fills : {}",this->FillCounter1D,this->FillCounter2D);
			}

			void KillListen(){
				KeepListen = false;
			}

			void Initialize(const int& numchannels,const int& ergsize,const int& scalarsize,const int& width_size,const int& event_size,const int& roll_size){
				this->channel_bins = numchannels;
				this->erg_bins = ergsize;
				this->scalar_bins = scalarsize;
				this->width_bins = width_size;
				this->event_bins = event_size;
				this->roll_bins = roll_size;
				RegisterPlot<TH2F>("Raw","Raw Energy; Energy (channel); Channel (arb.);",ergsize,0.0,ergsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("Scalar","Scalar Rate; Time (s); Channel (arb.);",scalarsize,0.0,scalarsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("Cal","Cal. Energy; Energy (keV); Channel (arb.);",ergsize,0.0,ergsize,numchannels,0,numchannels);
				RegisterPlot<TH1F>("Event_Width","Event Width; Time (ns);",width_size,0.0,width_size);
				RegisterPlot<TH1F>("Event_Size","Num Hits Event; Event Size (arb.);",event_size,0,event_size);
				RegisterPlot<TH2F>("Event_Mult","Event Size vs Channel; Channel (arb.); Event Size (arb.);",numchannels,0,numchannels,event_size,0,event_size);
				RegisterPlot<TH2F>("Event_Scale","Event Size vs Event Width; Time (ns); Event Size (arb.);",width_size,0,width_size,event_size,0,event_size);
				RegisterPlot<TH2F>("Total_Rate","Total Rate of All Channels; Time(s); Rollover (arb.)",scalarsize,0,scalarsize,roll_size,0,roll_size);
				RegisterPlot<TH2F>("Total_Pileup","Map of Channels that Underwent Pileup; Linearized Mod Num (arb.); Channel Num (arb.)",BOOSTPLOTS::SA,0,BOOSTPLOTS::SA,BOOSTPLOTS::S6,0,BOOSTPLOTS::S6);
				RegisterPlot<TH2F>("Total_Saturate","Map of Channels that Saturated; Linearized Mod Num (arb.); Channel Num (arb.)",BOOSTPLOTS::SA,0,BOOSTPLOTS::SA,BOOSTPLOTS::S6,0,BOOSTPLOTS::S6);
				RegisterPlot<TH2F>("Total_Hits","Map of Channels that Fired; Linearized Mod Num (arb.); Channel Num (arb.)",BOOSTPLOTS::SA,0,BOOSTPLOTS::SA,BOOSTPLOTS::S6,0,BOOSTPLOTS::S6);
			}

			template<typename T>
			void RegisterPlot(std::string name,std::string title,int nbinsx,double xmin,double xmax){
				static_assert(std::is_base_of<TH1,T>::value,"T must inherit from TH1");
				if( not PlotExist(name) ){
					this->Plots_1D[name] = new T(name.c_str(),title.c_str(),nbinsx,xmin,xmax);
					this->PlotIDs.push_back(name);
				}else{
					std::string mess = "Unable to register plot "+name+" as it already exists";
					throw mess;
				}
			}

			template<typename T>
			void RegisterPlot(std::string name,std::string title,int nbinsx,double xmin,double xmax,int nbinsy,double ymin,double ymax){
				static_assert(std::is_base_of<TH2,T>::value,"T must inherit from TH2");
				if( not PlotExist(name) ){
					this->Plots_2D[name] = new T(name.c_str(),title.c_str(),nbinsx,xmin,xmax,nbinsy,ymin,ymax);
					this->PlotIDs.push_back(name);
				}else{
					std::string mess = "Unable to register plot "+name+" as it already exists";
					throw mess;
				}
			}

			bool PlotExist(std::string name){
				return Plot1DExist(name) or Plot2DExist(name);
			}

			void Fill(const std::string& name,double xval){
				if( Plot1DExist(name) ){
					++(this->FillCounter1D);
					this->Plots_1D[name]->Fill(xval);
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 1D plot";
					throw mess;
				}
			}

			void WeightedFill(const std::string& name,double xval,double weight){
				if( Plot1DExist(name) ){
					++(this->FillCounter1D);
					this->Plots_1D[name]->Fill(xval,weight);
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 1D plot";
					throw mess;
				}
			}

			void FillN(const std::string& name,int n,double* xval,double* weight,int stride = 1){
				if( Plot1DExist(name) ){
					++(this->FillCounter1D);
					this->Plots_1D[name]->FillN(n,xval,weight,stride);
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 1D plot";
					throw mess;
				}
			}

			void IncrementBin(const std::string& name,int binx,int count,bool calcerr){
				if( Plot1DExist(name) ){
					auto currcount = this->Plots_1D[name]->GetBinContent(binx) + count;
					this->Plots_1D[name]->SetBinContent(binx,currcount);
					if( not calcerr ){
						this->Plots_1D[name]->SetBinError(binx,0.0);
					}else{
						this->Plots_1D[name]->SetBinError(binx,std::sqrt(currcount));
					}
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 1D plot";
					throw mess;
				}
			}

			void Fill(const std::string& name,double xval,double yval){
				if( Plot2DExist(name) ){
					++(this->FillCounter2D);
					this->Plots_2D[name]->Fill(xval,yval);
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 2D plot";
					throw mess;
				}
			}

			void WeightedFill(const std::string& name,double xval,double yval,double weight){
				if( Plot2DExist(name) ){
					++(this->FillCounter2D);
					this->Plots_2D[name]->Fill(xval,yval,weight);
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 2D plot";
					throw mess;
				}
			}

			void FillN(const std::string& name,int n,double* xval,double* yval,double* weight,int stride = 1){
				if( Plot2DExist(name) ){
					++(this->FillCounter2D);
					this->Plots_2D[name]->FillN(n,xval,yval,weight,stride);
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 2D plot";
					throw mess;
				}
			}

			void IncrementBin(const std::string& name,int binx,int biny,int count,bool calcerr){
				if( Plot2DExist(name) ){
					auto currcount = this->Plots_2D[name]->GetBinContent(binx,biny) + count;
					this->Plots_2D[name]->SetBinContent(binx,biny,currcount);
					if( not calcerr ){
						this->Plots_2D[name]->SetBinError(binx,biny,0.0);
					}else{
						this->Plots_2D[name]->SetBinError(binx,biny,std::sqrt(currcount));
					}
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 2D plot";
					throw mess;
				}
			}

			void WriteAllPlots(){
				boost::sort::spreadsort::string_sort(this->PlotIDs.begin(),this->PlotIDs.end());
				for( auto& name : this->PlotIDs ){
					Write(name);
				}
			}

			void WriteInfo(){
				std::ofstream output(this->outputprefix+".list");
				output << "name \t title \t nbinsx \t xmin \t xmax \t nbinsy \t ymin \t ymax" << std::endl;
				for( auto& name : PlotIDs ){
					output << name << '\t';
					if( Plot1DExist(name) ){
						output << this->Plots_1D[name]->GetTitle() << '\t' 
						       << this->Plots_1D[name]->GetNbinsX() << '\t'
						       << this->Plots_1D[name]->GetXaxis()->GetXmin() << '\t'
						       << this->Plots_1D[name]->GetXaxis()->GetXmax() << std::endl;
					}else{
						output << this->Plots_2D[name]->GetTitle() << '\t' 
						       << this->Plots_2D[name]->GetNbinsX() << '\t'
						       << this->Plots_2D[name]->GetXaxis()->GetXmin() << '\t'
						       << this->Plots_2D[name]->GetXaxis()->GetXmax() << '\t'
						       << this->Plots_2D[name]->GetNbinsY() << '\t'
						       << this->Plots_2D[name]->GetYaxis()->GetXmin() << '\t'
						       << this->Plots_2D[name]->GetYaxis()->GetXmax() << std::endl;
					}
				}
				output.close();
			}

			int GetChannelBins() const{
				return this->channel_bins;
			}

			int GetEnergyBins() const{
				return this->erg_bins;
			}

			int GetScalarBins() const{
				return this->scalar_bins;
			}

			int GetEventWidthBins() const{
				return this->width_bins;
			}

			int GetEventSizeBins() const{
				return this->event_bins;
			}

			int GetScalarRolloverBins() const{
				return this->roll_bins;
			}
		private:
			bool Plot1DExist(const std::string& name){
				return this->Plots_1D.find(name) != this->Plots_1D.end();
			}

			bool Plot2DExist(const std::string& name){
				return this->Plots_2D.find(name) != this->Plots_2D.end();
			}

			void Write(std::string name){
				if( Plot1DExist(name) ){
					//this->Plots_1D[name]->SetLineColor(GetCurrColor());
					this->Plots_1D[name]->Write(0,2,0);
				}else if( Plot2DExist(name) ){
					this->Plots_2D[name]->Write(0,2,0);
				}else{
					std::string mess = "Unable to write plot "+name+" as it doesn't exist";
					throw mess;
				}
			}

			std::vector<std::string> PlotIDs;
			boost::unordered_map<std::string,TH1*> Plots_1D;
			boost::unordered_map<std::string,TH2*> Plots_2D;
			std::string LogName;
			std::string outputprefix;

			bool KeepListen;

			int channel_bins;
			int erg_bins;
			int scalar_bins;
			int width_bins;
			int event_bins;
			int roll_bins;
			unsigned long long FillCounter1D;
			unsigned long long FillCounter2D;
		
			std::shared_ptr<spdlog::logger> console;
	};

}


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
	auto randNum = std::uniform_real_distribution<double>(0.0,16384.0);
	//const unsigned int MaxNumErg = 1000000;
	std::vector<double> erglistx;
	std::vector<double> erglisty;
	for( unsigned int ii = 0; ii < MaxNumErg; ++ii ){
		erglistx.push_back(randNum(randGen));
		erglisty.push_back(randNum(randGen));
	}

	TH1* RH1 = new TH1I("RH1","",16384,0,16384);
	TH2* RH2 = new TH2I("RH2","",16384,0,16384,16384,0,16384);

	auto BH1R = boost::histogram::make_histogram(boost::histogram::axis::regular<>(16384,0,16384, "x"));
	auto BH2R = boost::histogram::make_histogram(boost::histogram::axis::regular<>(16384,0,16384, "x"),boost::histogram::axis::regular<>(16384,0,16384,"y"));

	std::vector<boost::histogram::axis::regular<>> axes1 = { boost::histogram::axis::regular<>(16384,0,16384,"x") };
	std::vector<boost::histogram::axis::regular<>> axes2 = { boost::histogram::axis::regular<>(16384,0,16384,"x"),boost::histogram::axis::regular<>(16384,0,16384,"y") };
	
	auto BH1RD = boost::histogram::make_histogram(std::move(axes1));
	auto BH2RD = boost::histogram::make_histogram(std::move(axes2));

	auto start_time = std::chrono::high_resolution_clock::now();
	for( size_t ii = 0 ; ii < MaxNumErg; ++ii ){
		RH1->Fill(erglistx.at(ii));
	}
	auto stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double,std::milli> dur_r1 = (stop_time - start_time);

	start_time = std::chrono::high_resolution_clock::now();
	for( size_t ii = 0 ; ii < MaxNumErg; ++ii ){
		BH1R(erglistx.at(ii));
	}
	stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double,std::milli> dur_b1 = (stop_time - start_time);
;
	for( size_t ii = 0 ; ii < MaxNumErg; ++ii ){
		BH1RD(erglistx.at(ii));
	}
	stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double,std::milli> dur_b1d = (stop_time - start_time);

	start_time = std::chrono::high_resolution_clock::now();
	for( size_t ii = 0 ; ii < MaxNumErg; ++ii ){
		RH2->Fill(erglistx.at(ii),erglisty.at(ii));
	}
	stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double,std::milli> dur_r2 = (stop_time - start_time);

	start_time = std::chrono::high_resolution_clock::now();
	for( size_t ii = 0 ; ii < MaxNumErg; ++ii ){
		BH2R(erglistx.at(ii),erglisty.at(ii));
	}
	stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double,std::milli> dur_b2 = (stop_time - start_time);

	start_time = std::chrono::high_resolution_clock::now();
	for( size_t ii = 0 ; ii < MaxNumErg; ++ii ){
		BH2RD(erglistx.at(ii),erglisty.at(ii));
	}
	stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double,std::milli> dur_b2d = (stop_time - start_time);

	spdlog::info("NFills : {} | 1D -> Boost : {} | 1D -> Boost (dyn.) : {} | 1D-> Root : {}",MaxNumErg,dur_b1.count(),dur_b1d.count(),dur_r1.count());
	spdlog::info("NFills : {} | 2D -> Boost : {} | 2D -> Boost (dyn.) : {} | 2D-> Root : {}",MaxNumErg,dur_b2.count(),dur_b2d.count(),dur_r2.count());

}
