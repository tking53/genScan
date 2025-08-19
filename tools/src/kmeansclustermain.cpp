#include <fstream>
#include <map>
#include <ostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include <yaml-cpp/emitter.h>
#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

#include "TFile.h"
#include "TH2.h"

#include "Point2D.hpp"
#include "KMeans.hpp"
#include "CutManager.hpp"

int main(int argc, char *argv[]) {

	std::string inputfile;
	std::string hisname;
	std::string cutname;
	int kmeans;
	int maxiter;
	double tol;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("inputfile,i",boost::program_options::value<std::string>(&inputfile),"file to get the histogram from")
		("data,d",boost::program_options::value<std::string>(&hisname),"historam to run kmeans algorithm on")
		("cut,c",boost::program_options::value<std::string>(&cutname)->default_value(""),"cut to use to separate the data")
		("nummeans,n",boost::program_options::value<int>(&kmeans),"number of means to cluster into")
		("maxiters,m",boost::program_options::value<int>(&maxiter),"maximum number of iterations")
		("tolerance,t",boost::program_options::value<double>(&tol),"tolerance for early exiting")
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

	try{
		auto rfile = new TFile(inputfile.c_str(),"READ");
		auto mainhis = rfile->Get(hisname.c_str()); 
		if( mainhis != nullptr ){
			auto histype = std::string(mainhis->ClassName());
			boost::regex re2d("TH2");

			std::shared_ptr<CUTS::CutRegistry> CutManager(new CUTS::CutRegistry(""));
			if( boost::regex_search(histype, re2d) ){
				std::vector<Point2D<double>> data;
				TH2* his = reinterpret_cast<TH2*>(mainhis);

				if( not cutname.empty() ){
					CutManager->AddCut("cut",cutname);
				}
				
				std::random_device rd;
				std::mt19937_64 randGen(rd());
				std::uniform_real_distribution<double> randdist(0.0,1.0);
				
				auto xaxis = his->GetXaxis();
				auto yaxis = his->GetYaxis();
				for( int ii = 1; ii < his->GetNbinsX()+1 ; ++ii ){
					auto xwidth = xaxis->GetBinUpEdge(ii) - xaxis->GetBinLowEdge(ii);
					auto xval = xaxis->GetBinCenter(ii);
					for( int jj = 1; jj <  his->GetNbinsY(); ++jj ){
						auto ywidth = yaxis->GetBinUpEdge(jj) - yaxis->GetBinLowEdge(jj);
					       	auto yval = yaxis->GetBinCenter(jj);	
						if( his->GetBinContent(ii,jj) > 0 and CutManager->IsWithin("cut",xval,yval) ){
							//possible to swap this to a weighting rather than each value itself
							for( int kk = 0; kk < his->GetBinContent(ii,jj); ++kk ){ 
								auto x = xaxis->GetBinLowEdge(ii) + randdist(randGen)*xwidth;
								auto y = yaxis->GetBinLowEdge(jj) + randdist(randGen)*ywidth;
								data.push_back(Point2D<double>(x,y));
							}
						}
					}
				}
				KMeans<double> partitions(data,kmeans);
				auto finish = partitions.Run(maxiter,tol);

				if( finish == KMeans<double>::FINISHCODE::MAXITER ){
					spdlog::info("Reached max iterations");
				}else{
					spdlog::info("Reached centroid tolerance means");
				}
				std::ofstream means("Means.dat");
				for( const auto& p : partitions.GetCentroids() ){
					means << p << std::endl;
				}
				means.close();

				std::vector<std::vector<Point2D<double>>> clusters(kmeans,std::vector<Point2D<double>>());
				for( const auto& p : data){
					auto idx = partitions.FindClosestCentroid(p);
					clusters[idx].push_back(p);
				}
				for( size_t ii = 0; ii < kmeans; ++ii ){
					std::string oname = "Cluster_" + std::to_string(ii) + ".dat"; 
					std::ofstream out(oname);
					for( const auto& p : clusters[ii] ){
						out << p << std::endl;
					}
					out.close();
				}

			}else{
				throw std::runtime_error("not passed a TH2 histogram");
			}
		}else{
			throw std::runtime_error("histogram does not exist");
		}
	}catch( std::exception& e){
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}    


}
