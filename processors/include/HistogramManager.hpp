#ifndef __HISTOGRAM_MANAGER_HPP__
#define __HISTOGRAM_MANAGER_HPP__

#include <memory>
#include <stdexcept>
#include <string>
#include <fstream>
#include <type_traits>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/sort/spreadsort/string_sort.hpp>
//#include <boost/container/map.hpp>
//#include <boost/unordered_map.hpp>
#include <boost/unordered/unordered_node_map.hpp>

#include "TH1.h"
#include "TH2.h"
#include "TH2Poly.h"
#include "TCanvas.h"
#include "TFrame.h"
#include "TSocket.h"
#include "TServerSocket.h"
#include "TMonitor.h"
#include "TMessage.h"
#include "TList.h"
#include "TError.h"
#include "TROOT.h"
#include "TColor.h"

namespace PLOTS{
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

	struct HisHelper1D{
		int nbinsx;
		double xlow;
		double xhigh;
	};

	struct HisHelper2D{
		int nbinsx;
		double xlow;
		double xhigh;
		int nbinsy;
		double ylow;
		double yhigh;
	};
	
	class PlotRegistry{
		public:
			PlotRegistry(const std::string& log,const std::string oup,int PORT=9090){
				this->LogName = log;
				this->outputprefix = oup;
				this->console = spdlog::get(this->LogName)->clone("HistogramManager");
				this->FillCounter1D = 0;
				this->FillCounter2D = 0;

			
				ColorList = {
					{"kBlue",kBlue},
					{"kAzure",kAzure},
					{"kCyan",kCyan},
					{"kTeal",kTeal},
					{"kGreen",kGreen},
					{"kSpring",kSpring},
					{"kYellow",kYellow},
					{"kOrange",kOrange},
					{"kRed",kRed},
					{"kPink",kPink},
					{"kMagenta",kMagenta},
					{"kViolet",kViolet}
				};
			
				WedgeRange = {-10,4};
				WedgeDiff = (WedgeRange.second - WedgeRange.first)+1;
				LineRange = {-9,0};
				LineDiff = (LineRange.second - LineRange.first)+1;
				WedgeNames = {"kCyan","kRed","kGreen","kMagenta","kYellow","kBlue"};
				LineNames = {"kAzure","kOrange","kTeal","kPink","kSpring","kViolet"};
			
				currlineidx = 0;
				currwedgeidx = 0;
				currcoloridx = 0;

				if( PORT > 0 ){
					fServ = std::make_shared<TServerSocket>(PORT,true);
					if( not fServ->IsValid() ){
						std::string mess = "Unable to open Port : "+std::to_string(PORT)+" Bailing out";
						throw std::runtime_error(mess);
					}
					KeepListen = true;

					fMon = std::make_shared<TMonitor>();
					fMon->Add(fServ.get());

					fSockets = std::make_shared<TList>();

					fCanvas = std::make_shared<TCanvas>(log.c_str(),log.c_str(),600,600);
					fCanvas->SetFillColor(42);
					fCanvas->GetFrame()->SetFillColor(21);
					fCanvas->GetFrame()->SetBorderSize(6);
					fCanvas->GetFrame()->SetBorderMode(-1);
				}else{
					KeepListen = false;
				}
			}

			~PlotRegistry(){
				this->console->info("1D bucket count : {}, load factor {}/{}",this->Plots_1D.bucket_count(),this->Plots_1D.load_factor(),this->Plots_1D.max_load_factor());
				this->console->info("2D bucket count : {}, load factor {}/{}",this->Plots_2D.bucket_count(),this->Plots_2D.load_factor(),this->Plots_2D.max_load_factor());
				this->console->info("Num 1D Fills : {} | Num 2D Fills : {}",this->FillCounter1D,this->FillCounter2D);
			}

			int GetCurrLineColor(){
				return this->ColorList[this->LineNames[this->currlineidx]] + ((this->currlinemult%20) + this->LineRange.first); 
			}

			void ShiftLineColor(){
				++this->currlineidx;
				if( this->currlineidx == this->LineNames.size() ){
					this->currlineidx = 0;
					auto temp = (++this->currlinemult)%this->LineDiff;
					this->currlinemult = temp;
				}
				//if( ++this->currlinemult > this->LineRange.second ){
				//	this->currlineidx = (++this->currlineidx)%this->LineNames.size();
				//	this->currlinemult = this->LineRange.first;
				//}	
			}

			void HandleSocket(TSocket* s){
				if (s->IsA() == TServerSocket::Class()) {
					// accept new connection from spy
					TSocket *sock = ((TServerSocket*)s)->Accept();
					fMon->Add(sock);
					fSockets->Add(sock);
					spdlog::get(this->LogName)->info("Accepted Connection from {}",sock->GetInetAddress().GetHostName());
				} else {
					// we only get string based requests from the spy
					char request[64];
					if (s->Recv(request, sizeof(request)) <= 0) {
						fMon->Remove(s);
						fSockets->Remove(s);
						spdlog::get(this->LogName)->info("Closed Connection from {}",s->GetInetAddress().GetHostName());
						delete s;
						return;
					}

					// send requested object back
					TMessage answer(kMESS_OBJECT);
					std::string hisname(request);
					if( Plot1DExist(hisname) ){
						answer.WriteObject(this->Plots_1D[hisname]);
					}else if( Plot2DExist(hisname) ){
						answer.WriteObject(this->Plots_2D[hisname]);
					}else{
						Error("SpyServ::HandleSocket", "unexpected message");
					}
					s->Send(answer);
				}
			}

			void KillListen(){
				KeepListen = false;
			}

			void HandleSocketHelper(){
				while(KeepListen){
					//std::this_thread::sleep_for(std::chrono::seconds(20));
					//spdlog::info("Update");
					fCanvas->Modified();
					fCanvas->Update();

					TSocket *s;
					if ((s = fMon->Select(20)) != (TSocket*)-1)
						HandleSocket(s);
					if (ROOT::Detail::HasBeenDeleted(fCanvas.get()))
						break;
					if (gROOT->IsInterrupted())
						break;
				}
			}

			void Initialize(const int& numchannels,const int& ergsize,const int& scalarsize,const int& width_size,const int& event_size,const int& roll_size){
				this->channel_bins = numchannels;
				this->erg_bins = ergsize;
				this->scalar_bins = scalarsize;
				this->width_bins = width_size;
				this->event_bins = event_size;
				this->roll_bins = roll_size;
				RegisterPlot<TH2F>("Raw","Raw Energy; Energy (channel); Channel (arb.);",ergsize,0.0,ergsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("InternalRaw","Internal Trapezoid Filter Raw Energy; Energy (channel); Channel (arb.);",ergsize,-0.5*ergsize,0.5*ergsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("IntegralRaw","Internal Integral Filter Raw Energy; Energy (channel); Channel (arb.);",ergsize,-0.5*ergsize,0.5*ergsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("Scalar","Scalar Rate; Time (s); Channel (arb.);",scalarsize,0.0,scalarsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("Scalar_M","Scalar Rate; Time (min); Channel (arb.);",scalarsize,0.0,scalarsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("Scalar_5M","Scalar Rate; Time (5 min); Channel (arb.);",scalarsize,0.0,scalarsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("Cal","Cal. Energy; Energy (keV); Channel (arb.);",ergsize,0.0,ergsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("InternalCal","Internal Trapezoid Cal. Energy; Energy (keV); Channel (arb.);",ergsize,-0.5*ergsize,0.5*ergsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("IntegralCal","Internal Integral Cal. Energy; Energy (keV); Channel (arb.);",ergsize,-0.5*ergsize,0.5*ergsize,numchannels,0,numchannels);
				RegisterPlot<TH1F>("Event_Width","Event Width; Time (ns);",width_size,0.0,width_size);
				RegisterPlot<TH1F>("Event_Size","Num Hits Event; Event Size (arb.);",event_size,0,event_size);
				RegisterPlot<TH2F>("Event_Mult","Event Size vs Channel; Channel (arb.); Event Size (arb.);",numchannels,0,numchannels,event_size,0,event_size);
				RegisterPlot<TH2F>("Event_Scale","Event Size vs Event Width; Time (ns); Event Size (arb.);",width_size,0,width_size,event_size,0,event_size);
				RegisterPlot<TH2F>("Total_Rate","Total Rate of All Channels; Time (s); Rollover (arb.)",scalarsize,0,scalarsize,roll_size,0,roll_size);
				RegisterPlot<TH2F>("Total_Rate_M","Total Rate of All Channels; Time (min); Rollover (arb.)",scalarsize,0,scalarsize,roll_size,0,roll_size);
				RegisterPlot<TH2F>("Total_Rate_5M","Total Rate of All Channels; Time (5 min); Rollover (arb.)",scalarsize,0,scalarsize,roll_size,0,roll_size);
				RegisterPlot<TH2F>("Total_Pileup","Map of Channels that Underwent Pileup; Linearized Mod Num (arb.); Channel Num (arb.)",PLOTS::SA,0,PLOTS::SA,PLOTS::S6,0,PLOTS::S6);
				RegisterPlot<TH2F>("Total_Saturate","Map of Channels that Saturated; Linearized Mod Num (arb.); Channel Num (arb.)",PLOTS::SA,0,PLOTS::SA,PLOTS::S6,0,PLOTS::S6);
				RegisterPlot<TH2F>("Total_Hits","Map of Channels that Fired; Linearized Mod Num (arb.); Channel Num (arb.)",PLOTS::SA,0,PLOTS::SA,PLOTS::S6,0,PLOTS::S6);
				RegisterPlot<TH1F>("History_Width","History Width; Time (us);",100000,0,100000);
				RegisterPlot<TH1F>("Event_Delta","Time Between Beginning of Consecutive Events; Time (ns);",100000,0,100000);
				RegisterPlot<TH1F>("Event_Spacing","Time Between End and Beginning of Consecutive Events; Time (ns);",100000,0,100000);
				RegisterPlot<TH2F>("Trace_Size","Trace Size vs Channel; Channel (arb.); EventTrace Size (arb.);",numchannels,0,numchannels,500,0,500);
				RegisterPlot<TH2F>("QDC_0","Channel vs QDC 0; QDC (arb.); Channel (arb.);",ergsize,0,32*ergsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("QDC_1","Channel vs QDC 1; QDC (arb.); Channel (arb.);",ergsize,0,32*ergsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("QDC_2","Channel vs QDC 2; QDC (arb.); Channel (arb.);",ergsize,0,32*ergsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("QDC_3","Channel vs QDC 3; QDC (arb.); Channel (arb.);",ergsize,0,32*ergsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("QDC_4","Channel vs QDC 4; QDC (arb.); Channel (arb.);",ergsize,0,32*ergsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("QDC_5","Channel vs QDC 5; QDC (arb.); Channel (arb.);",ergsize,0,32*ergsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("QDC_6","Channel vs QDC 6; QDC (arb.); Channel (arb.);",ergsize,0,32*ergsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("QDC_7","Channel vs QDC 7; QDC (arb.); Channel (arb.);",ergsize,0,32*ergsize,numchannels,0,numchannels);
			}

			template<typename T>
			void RegisterPlot(std::string name,std::string title,const HisHelper1D& h){
				this->RegisterPlot<T>(name,title,h.nbinsx,h.xlow,h.xhigh);
			}
			
			template<typename T>
			void RegisterPlot(std::string name,std::string title,const HisHelper2D& h){
				this->RegisterPlot<T>(name,title,h.nbinsx,h.xlow,h.xhigh,h.nbinsy,h.ylow,h.yhigh);
			}

			[[nodiscard]] TH2Poly* RegisterPlot(std::string name,std::string title){
				if( not PlotExist(name) ){
					this->Plots_2DPoly[name] = new TH2Poly();
					this->Plots_2DPoly[name]->SetNameTitle(name.c_str(),title.c_str());
					this->Plots_2DPoly[name]->GetXaxis()->SetTitleSize(0.04);
					this->Plots_2DPoly[name]->GetXaxis()->SetLabelSize(0.04);
					this->Plots_2DPoly[name]->GetXaxis()->CenterTitle(true);
					this->Plots_2DPoly[name]->GetYaxis()->SetTitleSize(0.04);
					this->Plots_2DPoly[name]->GetYaxis()->SetTitleOffset(1.375);
					this->Plots_2DPoly[name]->GetYaxis()->SetLabelSize(0.04);
					this->Plots_2DPoly[name]->GetYaxis()->CenterTitle(true);
					//this->console->debug("2D plots load factor : {}, bucket counts : {} after adding {}",this->Plots_2D.load_factor(),this->Plots_2D.bucket_count(),name);
					this->PlotIDs.push_back(name);
					return this->Plots_2DPoly[name];
				}else{
					std::string mess = "Unable to register plot "+name+" as it already exists";
					this->console->error("{}",mess);
					throw mess;
				}

			}

			template<typename T>
			void RegisterPlot(std::string name,std::string title,int nbinsx,double xmin,double xmax){
				static_assert(std::is_base_of<TH1,T>::value,"T must inherit from TH1");
				if( not PlotExist(name) ){
					this->Plots_1D[name] = new T(name.c_str(),title.c_str(),nbinsx,xmin,xmax);
					this->Plots_1D[name]->GetXaxis()->SetTitleSize(0.04);
					this->Plots_1D[name]->GetXaxis()->SetLabelSize(0.04);
					this->Plots_1D[name]->GetXaxis()->CenterTitle(true);
					this->Plots_1D[name]->GetYaxis()->SetTitleSize(0.04);
					this->Plots_1D[name]->GetYaxis()->SetTitleOffset(1.375);
					this->Plots_1D[name]->GetYaxis()->SetLabelSize(0.04);
					this->Plots_1D[name]->GetYaxis()->CenterTitle(true);
					this->Plots_1D[name]->SetLineColor(this->GetCurrLineColor());
					//this->console->debug("1D plots load factor : {}, bucket counts : {} after adding {}",this->Plots_1D.load_factor(),this->Plots_1D.bucket_count(),name);
					this->ShiftLineColor();
					this->PlotIDs.push_back(name);
				}else{
					std::string mess = "Unable to register plot "+name+" as it already exists";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			template<typename T>
			void RegisterPlot(std::string name,std::string title,int nbinsx,double xmin,double xmax,int nbinsy,double ymin,double ymax){
				static_assert(std::is_base_of<TH2,T>::value,"T must inherit from TH2");
				if( not PlotExist(name) ){
					this->Plots_2D[name] = new T(name.c_str(),title.c_str(),nbinsx,xmin,xmax,nbinsy,ymin,ymax);
					this->Plots_2D[name]->GetXaxis()->SetTitleSize(0.04);
					this->Plots_2D[name]->GetXaxis()->SetLabelSize(0.04);
					this->Plots_2D[name]->GetXaxis()->CenterTitle(true);
					this->Plots_2D[name]->GetYaxis()->SetTitleSize(0.04);
					this->Plots_2D[name]->GetYaxis()->SetTitleOffset(1.375);
					this->Plots_2D[name]->GetYaxis()->SetLabelSize(0.04);
					this->Plots_2D[name]->GetYaxis()->CenterTitle(true);
					//this->console->debug("2D plots load factor : {}, bucket counts : {} after adding {}",this->Plots_2D.load_factor(),this->Plots_2D.bucket_count(),name);
					this->PlotIDs.push_back(name);
				}else{
					std::string mess = "Unable to register plot "+name+" as it already exists";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			bool PlotExist(std::string name){
				return Plot1DExist(name) or Plot2DExist(name) or Plot2DPolyExist(name);
			}

			void Fill(const std::string& name,double xval){
				auto test = this->Plots_1D.find(name);
				if( test != this->Plots_1D.end() ){
					++(this->FillCounter1D);
					test->second->Fill(xval);
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 1D plot";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void Fill(TH1* his,double xval){
				if( his != nullptr ){
					++(this->FillCounter1D);
					his->Fill(xval);
				}else{
					std::string mess = "Passed nullptr histogram to 1D Fill()";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void WeightedFill(const std::string& name,double xval,double weight){
				auto test = this->Plots_1D.find(name);
				if( test != this->Plots_1D.end() ){
					++(this->FillCounter1D);
					test->second->Fill(xval,weight);
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 1D plot";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void WeightedFill(TH1* his,double xval,double weight){
				if( his != nullptr ){
					++(this->FillCounter1D);
					his->Fill(xval,weight);
				}else{
					std::string mess = "Passed nullptr histogram to 1D WeightedFill()";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void FillN(const std::string& name,int n,double* xval,double* weight,int stride = 1){
				auto test = this->Plots_1D.find(name);
				if( test != this->Plots_1D.end() ){
					++(this->FillCounter1D);
					test->second->FillN(n,xval,weight,stride);
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 1D plot";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void FillN(TH1* his,int n,double* xval,double* weight,int stride = 1){
				if( his != nullptr ){
					++(this->FillCounter1D);
					his->FillN(n,xval,weight,stride);
				}else{
					std::string mess = "Passed nullptr histogram to 1D FillN";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void IncrementBin(const std::string& name,int binx,int count,bool calcerr){
				auto test = this->Plots_1D.find(name);
				if( test != this->Plots_1D.end() ){
					auto currcount = test->second->GetBinContent(binx) + count;
					test->second->SetBinContent(binx,currcount);
					if( not calcerr ){
						test->second->SetBinError(binx,0.0);
					}else{
						test->second->SetBinError(binx,std::sqrt(currcount));
					}
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 1D plot";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void IncrementBin(TH1* his,int binx,int count,bool calcerr){
				if( his != nullptr ){
					auto currcount = his->GetBinContent(binx) + count;
					his->SetBinContent(binx,currcount);
					if( not calcerr ){
						his->SetBinError(binx,0.0);
					}else{
						his->SetBinError(binx,std::sqrt(currcount));
					}
				}else{
					std::string mess = "Passed nullptr histogram to 1D IncrementBin";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void SetBin(const std::string& name,int binx,int count,bool calcerr){
				auto test = this->Plots_1D.find(name);
				if( test != this->Plots_1D.end() ){
					test->second->SetBinContent(binx,count);
					if( not calcerr ){
						test->second->SetBinError(binx,0.0);
					}else{
						test->second->SetBinError(binx,std::sqrt(count));
					}
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 1D plot";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void SetBin(TH1* his,int binx,int count,bool calcerr){
				if( his != nullptr ){
					his->SetBinContent(binx,count);
					if( not calcerr ){
						his->SetBinError(binx,0.0);
					}else{
						his->SetBinError(binx,std::sqrt(count));
					}
				}else{
					std::string mess = "Passed nullptr histogram to 1D SetBin";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void Fill(const std::string& name,double xval,double yval){
				auto test = this->Plots_2D.find(name);
				if( test != this->Plots_2D.end() ){
					++(this->FillCounter2D);
					test->second->Fill(xval,yval);
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 2D plot";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void Fill(TH2* his,double xval,double yval){
				if( his != nullptr ){
					++(this->FillCounter2D);
					his->Fill(xval,yval);
				}else{
					std::string mess = "Passed nullptr histogram to 2D Fill()";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void WeightedFill(const std::string& name,double xval,double yval,double weight){
				auto test = this->Plots_2D.find(name);
				if( test != this->Plots_2D.end() ){
					++(this->FillCounter2D);
					test->second->Fill(xval,yval,weight);
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 2D plot";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void WeightedFill(TH2* his,double xval,double yval,double weight){
				if( his != nullptr ){
					++(this->FillCounter2D);
					his->Fill(xval,yval,weight);
				}else{
					std::string mess = "Passed nullptr histogram to 2D WeightedFill()";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void FillN(const std::string& name,int n,double* xval,double* yval,double* weight,int stride = 1){
				auto test = this->Plots_2D.find(name);
				if( test != this->Plots_2D.end() ){
					++(this->FillCounter2D);
					test->second->FillN(n,xval,yval,weight,stride);
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 2D plot";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void FillN(TH2* his,int n,double* xval,double* yval,double* weight,int stride = 1){
				if( his != nullptr ){
					++(this->FillCounter2D);
					his->FillN(n,xval,yval,weight,stride);
				}else{
					std::string mess = "Passed nullptr histogram to 2D FillN";
					this->console->error("{}",mess);
					throw mess;
				}
			}


			void IncrementBin(const std::string& name,int binx,int biny,int count,bool calcerr){
				auto test = this->Plots_2D.find(name);
				if( test != this->Plots_2D.end() ){
					auto currcount = test->second->GetBinContent(binx,biny) + count;
					test->second->SetBinContent(binx,biny,currcount);
					if( not calcerr ){
						test->second->SetBinError(binx,biny,0.0);
					}else{
						test->second->SetBinError(binx,biny,std::sqrt(currcount));
					}
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 2D plot";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void IncrementBin(TH2* his,int binx,int biny,int count,bool calcerr){
				if( his != nullptr ){
					auto currcount = his->GetBinContent(binx,biny) + count;
					his->SetBinContent(binx,biny,currcount);
					if( not calcerr ){
						his->SetBinError(binx,biny,0.0);
					}else{
						his->SetBinError(binx,biny,std::sqrt(currcount));
					}
				}else{
					std::string mess = "Passed nullptr histogram to 2D IncrementBin()";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void SetBin(const std::string& name,int binx,int biny,int count,bool calcerr){
				auto test = this->Plots_2D.find(name);
				if( test != this->Plots_2D.end() ){
					test->second->SetBinContent(binx,biny,count);
					if( not calcerr ){
						test->second->SetBinError(binx,biny,0.0);
					}else{
						test->second->SetBinError(binx,biny,std::sqrt(count));
					}
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 2D plot";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void SetBin(TH2* his,int binx,int biny,int count,bool calcerr){
				if( his != nullptr ){
					his->SetBinContent(binx,biny,count);
					if( not calcerr ){
						his->SetBinError(binx,biny,0.0);
					}else{
						his->SetBinError(binx,biny,std::sqrt(count));
					}
				}else{
					std::string mess = "Passed nullptr histogram to 2D SetBin()";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			void WriteAllPlots(){
				boost::sort::spreadsort::string_sort(this->PlotIDs.begin(),this->PlotIDs.end());
				//std::sort(this->PlotIDs.begin(),this->PlotIDs.end());
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
					}else if( Plot2DExist(name) ){
						output << this->Plots_2D[name]->GetTitle() << '\t' 
						       << this->Plots_2D[name]->GetNbinsX() << '\t'
						       << this->Plots_2D[name]->GetXaxis()->GetXmin() << '\t'
						       << this->Plots_2D[name]->GetXaxis()->GetXmax() << '\t'
						       << this->Plots_2D[name]->GetNbinsY() << '\t'
						       << this->Plots_2D[name]->GetYaxis()->GetXmin() << '\t'
						       << this->Plots_2D[name]->GetYaxis()->GetXmax() << std::endl;
					}else{
						output << this->Plots_2DPoly[name]->GetTitle() << std::endl;
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
			//template<class T>
			//struct InternalHisInfo{
			//	std::string name;
			//	std::string title;
			//	T HisInfo;
			//};
			//struct InternalPolyHisInfo{
			//	std::string name;
			//	std::string title;
			//	std::vector<double> binxcoords;
			//	std::vector<double> binycoords;
			//};

			bool Plot1DExist(const std::string& name){
				return this->Plots_1D.find(name) != this->Plots_1D.end();
			}

			bool Plot2DExist(const std::string& name){
				return this->Plots_2D.find(name) != this->Plots_2D.end();
			}

			bool Plot2DPolyExist(const std::string& name){
				return this->Plots_2DPoly.find(name) != this->Plots_2DPoly.end();
			}

			void Write(std::string name){
				if( Plot1DExist(name) ){
					//this->Plots_1D[name]->SetLineColor(GetCurrColor());
					this->Plots_1D[name]->Write(0,2,0);
				}else if( Plot2DExist(name) ){
					this->Plots_2D[name]->Write(0,2,0);
				}else if( Plot2DPolyExist(name) ){
					this->Plots_2DPoly[name]->Write(0,2,0);
				}else{
					std::string mess = "Unable to write plot "+name+" as it doesn't exist";
					this->console->error("{}",mess);
					throw mess;
				}
			}

			std::vector<std::string> PlotIDs;
			boost::unordered_node_map<std::string,TH1*> Plots_1D;
			boost::unordered_node_map<std::string,TH2*> Plots_2D;
			boost::unordered_node_map<std::string,TH2Poly*> Plots_2DPoly;
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
			//info needed for live histogramming
			std::shared_ptr<TCanvas> fCanvas;
			std::shared_ptr<TServerSocket> fServ;
			std::shared_ptr<TMonitor> fMon;
			std::shared_ptr<TList> fSockets;
		
			std::shared_ptr<spdlog::logger> console;

			std::map<std::string,EColor> ColorList;
			std::pair<int,int> WedgeRange;
			int WedgeDiff;
			std::pair<int,int> LineRange;
			int LineDiff;
			std::vector<std::string> WedgeNames;
			std::vector<std::string> LineNames;
			size_t currlineidx;
			int currlinemult;
			size_t currwedgeidx;
			int currwedgemult;
			size_t currcoloridx;
	};

}

#endif
