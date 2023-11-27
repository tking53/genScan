#ifndef __HISTOGRAM_MANAGER_HPP__
#define __HISTOGRAM_MANAGER_HPP__

#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <regex>
#include <string>
#include <fstream>
#include <type_traits>
#include <mutex>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TCanvas.h"
#include "TFrame.h"
#include "TSocket.h"
#include "TServerSocket.h"
#include "TMonitor.h"
#include "TMessage.h"
#include "TRandom.h"
#include "TList.h"
#include "TError.h"
#include "TROOT.h"

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
	
	class PlotRegistry{
		public:
			PlotRegistry(const std::string& log,const std::string oup,int PORT=9090){
				this->LogName = log;
				this->outputprefix = oup;
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

			void RandFill(){
				float px, py;
				for (Int_t i = 0; ; i++) {
					gRandom->Rannor(px,py);
					Fill("Raw",px,py);
					gRandom->Rannor(px,py);
					Fill("Scalar",px,py);
					gRandom->Rannor(px,py);
					Fill("Cal",px,py);
					if( not KeepListen )
						break;
				}
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

			void Initialize(const int& numchannels,const int& ergsize,const int& scalarsize){
				RegisterPlot<TH2F>("Raw","Raw Energy; Energy (channel); Channel (arb.);",ergsize,0.0,ergsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("Scalar","Scalar Rate; Time (s); Channel (arb.);",scalarsize,0.0,scalarsize,numchannels,0,numchannels);
				RegisterPlot<TH2F>("Cal","Cal. Energy; Energy (keV); Channel (arb.);",ergsize,0.0,ergsize,numchannels,0,numchannels);
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
					this->Plots_1D[name]->Fill(xval);
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 1D plot";
					throw mess;
				}
			}

			void WeightedFill(const std::string& name,double xval,double weight){
				if( Plot1DExist(name) ){
					this->Plots_1D[name]->Fill(xval,weight);
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 1D plot";
					throw mess;
				}
			}

			void FillN(const std::string& name,int n,double* xval,double* weight,int stride = 1){
				if( Plot1DExist(name) ){
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
					this->Plots_2D[name]->Fill(xval,yval);
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 2D plot";
					throw mess;
				}
			}

			void WeightedFill(const std::string& name,double xval,double yval,double weight){
				if( Plot2DExist(name) ){
					this->Plots_2D[name]->Fill(xval,yval,weight);
				}else{
					std::string mess = "Plot : "+name+" does not exist as a 2D plot";
					throw mess;
				}
			}

			void FillN(const std::string& name,int n,double* xval,double* yval,double* weight,int stride = 1){
				if( Plot2DExist(name) ){
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
				std::sort(this->PlotIDs.begin(),this->PlotIDs.end(),[](std::string& a,std::string& b){
						std::vector<std::string> atokens;
						std::vector<std::string> btokens;
						std::regex re("\\d+");

						std::sregex_token_iterator abegin(a.begin(), a.end(), re);
						std::sregex_token_iterator bbegin(b.begin(), b.end(), re);
						std::sregex_token_iterator end;

						std::copy(abegin, end, std::back_inserter(atokens));
						std::copy(bbegin, end, std::back_inserter(btokens));

						std::vector<int> avalues;
						std::vector<int> bvalues;

						for( auto& t : atokens )
						avalues.push_back(std::stoi(t));

						for( auto& t : btokens )
						bvalues.push_back(std::stoi(t));

						for( size_t ii = 0; ii < std::min(avalues.size(),bvalues.size()); ++ii ){
							if( avalues.at(ii) != bvalues.at(ii) )
								return avalues.at(ii) < bvalues.at(ii);
						}
						return true;

				});
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
			std::unordered_map<std::string,TH1*> Plots_1D;
			std::unordered_map<std::string,TH2*> Plots_2D;
			std::string LogName;
			std::string outputprefix;

			bool KeepListen;

			//info needed for live histogramming
			std::shared_ptr<TCanvas> fCanvas;
			std::shared_ptr<TServerSocket> fServ;
			std::shared_ptr<TMonitor> fMon;
			std::shared_ptr<TList> fSockets;
	};

}

#endif
