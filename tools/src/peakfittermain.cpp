#include <fstream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include <yaml-cpp/emitter.h>
#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

#include "TFile.h"
#include "TH1.h"
#include "TH2.h"

#include "StringManipFunctions.hpp"
#include "PeakFitter.hpp"

YAML::Emitter& operator << (YAML::Emitter& out, const PeakFitter* pf) {
	out <<  YAML::BeginMap << YAML::Key << "HisName" << YAML::Value << pf->fithist->GetName() 
			       << YAML::Key << "Range" << YAML::BeginMap 
			       		<< YAML::Key << "Low" << YAML::Value << pf->FitRange.first
			       		<< YAML::Key << "High" << YAML::Value << pf->FitRange.second << YAML::EndMap
			       << YAML::Key << "Values" << YAML::Value << pf->Results 
			       << YAML::Key << "Errors" << YAML::Value << pf->Errors 
			       << YAML::Key << "ReducedChi2" << YAML::Value << pf->Results.at("Chi2")/pf->Errors.at("NDF") 
	     << YAML::EndMap;
	return out;
}

int main(int argc, char *argv[]) {

	std::string outputprefix;
	std::string inputfile;
	std::string hisname;
	int dimensionality;
	std::string axis;
	std::vector<int> indices;
	double xlow;
	double xhigh;
	bool quiet;
	bool chi2;
	int mode;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("projectionindices,p",boost::program_options::value<std::vector<int>>(&indices),"indices to project on if 2d histogram")
		("lowerbound,l",boost::program_options::value<double>(&xlow),"lower bound to perform fit")
		("upperbound,u",boost::program_options::value<double>(&xhigh),"upper bound to perform fit")
		("inputfile,i",boost::program_options::value<std::string>(&inputfile),"file to get the histogram from")
		("outputprefix,o",boost::program_options::value<std::string>(&outputprefix)->default_value("GenPeakFitterResults"),"file to output to fit info to")
		("numdimension,n",boost::program_options::value<int>(&dimensionality)->default_value(1),"dimensionality of histogram (1,2)")
		("mode,m",boost::program_options::value<int>(&mode)->default_value(0),"peak fitting mode 0->GaussNLinBkg")
		("axis,a",boost::program_options::value<std::string>(&axis)->default_value("x"),"axis to project onto (x,y,X,Y) if 2D")
		("data,d",boost::program_options::value<std::string>(&hisname),"histogram to manipulate")
		("quiet,q",boost::program_options::value<bool>(&quiet)->default_value(false),"quiet output")
		("chi2,c",boost::program_options::value<bool>(&chi2)->default_value(true),"chi2 fit, or loglikelihood")
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

		auto numproj = indices.size();

		if( not vm.count("lowerbound") ){
			spdlog::error("missing lowerbound");
			exit(EXIT_FAILURE);
		}
	        if( not	vm.count("upperbound") ){
			spdlog::error("missing upperbound");
			exit(EXIT_FAILURE);
		}	
		if( dimensionality == 2 or numproj < 1){
			spdlog::error("dimensionality is 2, but no projections given");
			exit(EXIT_FAILURE);
		}

		if( not vm.count("data") ){
			spdlog::error("Not provided histogram to fit");
			exit(EXIT_FAILURE);
		}

		if( not vm.count("inputfile") ){
			spdlog::error("Not provided inputfile containing histogram to fit");
			exit(EXIT_FAILURE);
		}

		axis = StringManip::tolower(axis);
		if( axis.compare("x") != 0 and axis.compare("y") != 0 ){
			spdlog::error("unknown axis projection : {}",axis);
			exit(EXIT_FAILURE);
		}

	}catch( std::exception& e){
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}    

	try{
		auto rfile = new TFile(inputfile.c_str(),"READ");
		auto mainhis = rfile->Get(hisname.c_str()); 
		std::vector<PeakFitter*> pfs;
		if( mainhis != nullptr ){
			auto histype = std::string(mainhis->ClassName());
			boost::regex re2d("TH2");
			boost::regex re1d("TH1");
			TH1* histofit;
			if( boost::regex_search(histype, re2d) ){
				for( const auto& idx : indices ){
					auto name = std::string(mainhis->GetName())+"_proj_"+axis+std::to_string(idx);
					if( axis.compare("x") == 0 ){
						histofit = dynamic_cast<TH2*>(mainhis)->ProjectionX(name.c_str(),idx,idx);
					}else{
						histofit = dynamic_cast<TH2*>(mainhis)->ProjectionY(name.c_str(),idx,idx);
					}	
					histofit->SetDirectory(0);
					pfs.push_back(new PeakFitter(xlow,xhigh,chi2,mode,histofit));
				}
			}else if( boost::regex_search(histype,re1d) ){
				histofit = dynamic_cast<TH1*>(mainhis);
				histofit->SetDirectory(0);
				pfs.push_back(new PeakFitter(xlow,xhigh,chi2,mode,histofit));
			}else{
				throw std::runtime_error("not passed a TH1 or TH2 histogram");
			}
			rfile->Close();
			auto outputfile = outputprefix+".root";
			auto ofile = new TFile(outputfile.c_str(),"RECREATE");
			for( const auto& f : pfs ){
				f->WriteHistogram();
			}
			ofile->Close();
			YAML::Emitter  doc;
			doc << YAML::BeginMap;
			doc << YAML::Key << "InputFile" << YAML::Value << inputfile;
			doc << YAML::Key << "InputHistogram" << YAML::Value << hisname;
			doc << YAML::Key << "FitResults";
			doc << pfs;
			doc << YAML::EndMap;

			std::ofstream yfile(outputprefix+"Report.yaml");
			yfile << doc.c_str() << std::endl;
			yfile.close();
		}else{
			throw std::runtime_error("histogram does not exist");
		}
	}catch( std::exception& e){
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}    


}
