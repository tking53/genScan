#include <TNamed.h>
#include <fstream>
#include <map>
#include <ostream>
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

std::map<std::string,double> ParseFixedValues(const std::vector<std::string>& values){
	std::map<std::string,double> retvals;
	for( const auto& s : values ){
		std::vector<std::string> strs;
		boost::split(strs,s,boost::is_any_of(":"));
		boost::regex number("^(?!-0(\\.0+)?(e|$))-?(0|[1-9]\\d*)(\\.\\d+)?(e-?(0|[1-9]\\d*))?");

		if( strs.size() == 2 ){
			boost::smatch pmatch;
			std::string valname = strs[0];
			if( retvals.find(valname) != retvals.end() ){
				throw std::runtime_error("Given Fixed Parameter multiple times");
			}else{
				if( boost::regex_match(strs[1],pmatch,number) ){
					retvals[valname] = std::stod(strs[1]);
				}else{
					throw std::runtime_error("Invalid fixed value, not a number");
				}
			}
		}else{
			throw std::runtime_error("Unable to parse input");
		}
	}
	return retvals;
}

std::map<std::string,std::pair<double,double>> ParseBoundedValues(const std::vector<std::string>& values){
	std::map<std::string,std::pair<double,double>> retvals;
	for( const auto& s : values ){
		std::vector<std::string> strs;
		boost::split(strs,s,boost::is_any_of(":"));
		boost::regex number("^(?!-0(\\.0+)?(e|$))-?(0|[1-9]\\d*)(\\.\\d+)?(e-?(0|[1-9]\\d*))?");

		if( strs.size() == 3 ){
			boost::smatch lmatch;
			boost::smatch umatch;
			std::string valname = strs[0];
			if( retvals.find(valname) != retvals.end() ){
				throw std::runtime_error("Given Bounded Parameter multiple times");
			}else{
				if( boost::regex_match(strs[1],lmatch,number) and boost::regex_match(strs[2],lmatch,number) ){
					retvals[valname] = {std::stod(strs[1]),std::stod(strs[2])};
				}else{
					throw std::runtime_error("Invalid bounded pair, not two numbers");
				}
			}
		}else{
			throw std::runtime_error("Unable to parse input");
		}
	}
	return retvals;
}

std::vector<std::pair<double,double>> ParseGates(const std::vector<std::string>& values){
	std::vector<std::pair<double,double>> retvals;
	for( const auto& s : values ){
		std::vector<std::string> strs;
		boost::split(strs,s,boost::is_any_of(":"));
		boost::regex number("^(?!-0(\\.0+)?(e|$))-?(0|[1-9]\\d*)(\\.\\d+)?(e-?(0|[1-9]\\d*))?");

		if( strs.size() == 2 ){
			boost::smatch lmatch;
			boost::smatch umatch;
			if( boost::regex_match(strs[0],lmatch,number) and boost::regex_match(strs[1],lmatch,number) ){
				retvals.push_back({std::stod(strs[0]),std::stod(strs[1])});
			}else{
				throw std::runtime_error("Invalid bounded pair, not two numbers");
			}
		}else{
			throw std::runtime_error("Unable to parse input");
		}
	}
	return retvals;
}

int main(int argc, char *argv[]) {

	std::string outputprefix;
	std::string inputfile;
	std::string hisname;
	int dimensionality;
	std::string axis;
	std::vector<int> indices;
	std::vector<std::string> boundedparams;
	std::vector<std::string> fixedparams;
	std::vector<std::string> gates;
	double xlow;
	double xhigh;
	bool quiet;
	bool chi2;
	bool storechi2;
	int mode;
	std::map<std::string,double> fixedvalues; 
	std::map<std::string,std::pair<double,double>> boundedvalues; 
	std::vector<std::pair<double,double>> gatevalues;

	std::string FittingMessage = "peak fitting mode";
       	FittingMessage += "\n0->GaussN+LinBkg";
       	FittingMessage += "\n1->GaussN+CompBkg+LinBkg";
       	FittingMessage += "\n2->SingleTailGaussN";
       	FittingMessage += "\n3->DoubleTailGaussN";
       	FittingMessage += "\n4->SingleTailGaussN+LinBkg";

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("projectionindices,p",boost::program_options::value<std::vector<int>>(&indices)->multitoken(),"indices to project on if 2d histogram")
		("gate,g",boost::program_options::value<std::vector<std::string>>(&gates)->multitoken(),"values to gate within in 2d histogram")
		("boundparameter,b",boost::program_options::value<std::vector<std::string>>(&boundedparams)->multitoken(),"parameter to bound name:low:high")
		("fixparameter,f",boost::program_options::value<std::vector<std::string>>(&fixedparams)->multitoken(),"parameter to bound name:value")
		("lowerbound,l",boost::program_options::value<double>(&xlow),"lower bound to perform fit")
		("upperbound,u",boost::program_options::value<double>(&xhigh),"upper bound to perform fit")
		("inputfile,i",boost::program_options::value<std::string>(&inputfile),"file to get the histogram from")
		("outputprefix,o",boost::program_options::value<std::string>(&outputprefix)->default_value("GenPeakFitterResults"),"file to output to fit info to")
		("numdimension,n",boost::program_options::value<int>(&dimensionality)->default_value(1),"dimensionality of histogram (1,2)")
		("mode,m",boost::program_options::value<int>(&mode)->default_value(0),FittingMessage.c_str())
		("axis,a",boost::program_options::value<std::string>(&axis)->default_value("x"),"axis to project onto (x,y,X,Y) if 2D")
		("data,d",boost::program_options::value<std::string>(&hisname),"histogram to manipulate")
		("quiet,q",boost::program_options::value<bool>(&quiet)->default_value(false),"quiet output")
		("chi2,c",boost::program_options::value<bool>(&chi2)->default_value(true),"chi2 fit, or loglikelihood")
		("storechi2,s",boost::program_options::value<bool>(&storechi2)->default_value(true),"store chi2 plot")
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
		auto numgates = gatevalues.size();

		if( not vm.count("lowerbound") ){
			spdlog::error("missing lowerbound");
			exit(EXIT_FAILURE);
		}
	        if( not	vm.count("upperbound") ){
			spdlog::error("missing upperbound");
			exit(EXIT_FAILURE);
		}	
		if( dimensionality == 2 and numproj < 1 and numgates < 1){
			spdlog::error("dimensionality is 2, but no projections or gates given");
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

		fixedvalues = ParseFixedValues(fixedparams);
		boundedvalues = ParseBoundedValues(boundedparams);
		gatevalues = ParseGates(gates);
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
					pfs.push_back(new PeakFitter(xlow,xhigh,chi2,mode,histofit,fixedvalues,boundedvalues));
				}
				int idx = 0;
				for( const auto& g : gatevalues ){
					auto name = std::string(mainhis->GetName())+"_gate_"+axis+std::to_string(idx);
					if( axis.compare("x") == 0 ){
						auto minbin = dynamic_cast<TH2*>(mainhis)->GetYaxis()->FindBin(g.first);
						auto maxbin = dynamic_cast<TH2*>(mainhis)->GetYaxis()->FindBin(g.second);
						histofit = dynamic_cast<TH2*>(mainhis)->ProjectionY(name.c_str(),minbin,maxbin);
					}else{
						auto minbin = dynamic_cast<TH2*>(mainhis)->GetXaxis()->FindBin(g.first);
						auto maxbin = dynamic_cast<TH2*>(mainhis)->GetXaxis()->FindBin(g.second);
						histofit = dynamic_cast<TH2*>(mainhis)->ProjectionX(name.c_str(),minbin,maxbin);
					}
					histofit->SetDirectory(0);
					pfs.push_back(new PeakFitter(xlow,xhigh,chi2,mode,histofit,fixedvalues,boundedvalues));
					++idx;
				}
			}else if( boost::regex_search(histype,re1d) ){
				histofit = dynamic_cast<TH1*>(mainhis);
				histofit->SetDirectory(0);
				pfs.push_back(new PeakFitter(xlow,xhigh,chi2,mode,histofit,fixedvalues,boundedvalues));
			}else{
				throw std::runtime_error("not passed a TH1 or TH2 histogram");
			}
			auto MaxCrates = rfile->Get("MAX_CRATES");
			auto MaxCardsPerCrate = rfile->Get("MAX_CARDS_PER_CRATE");
			auto MaxChannelsPerBoard = rfile->Get("MAX_CHANNELS_PER_BOARD");
			rfile->Close();
			auto outputfile = outputprefix+".root";
			auto ofile = new TFile(outputfile.c_str(),"RECREATE");
			for( const auto& f : pfs ){
				f->WriteHistogram(storechi2);
			}
			ofile->Close();
			YAML::Emitter  doc;
			doc << YAML::BeginMap;
			doc << YAML::Key << "InputFile" << YAML::Value << inputfile;
			doc << YAML::Key << "InputHistogram" << YAML::Value << hisname;
			if( MaxCrates != nullptr ){
				doc << YAML::Key << "MAX_CRATES" << YAML::Value << MaxCrates->GetTitle();
			}
			if( MaxCardsPerCrate != nullptr ){
				doc << YAML::Key << "MAX_CARDS_PER_CRATE" << YAML::Value << MaxCardsPerCrate->GetTitle();
			}
			if( MaxChannelsPerBoard != nullptr ){
				doc << YAML::Key << "MAX_CHANNELS_PER_BOARD" << YAML::Value << MaxChannelsPerBoard->GetTitle();
			}
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
