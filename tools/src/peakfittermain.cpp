#include <TNamed.h>
#include <cstdlib>
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
#include <boost/describe.hpp>

#include <yaml-cpp/emitter.h>
#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

#include "TFile.h"
#include "TH1.h"
#include "TH2.h"

#include "StringManipFunctions.hpp"
#include "PeakFitter.hpp"

YAML::Emitter& operator << (YAML::Emitter& out, const PeakFitter1D* pf) {
	out <<  YAML::BeginMap << YAML::Key << "HisName" << YAML::Value << pf->fithist->GetName() 
		<< YAML::Key << "Range" 
		<< YAML::BeginMap 
		<< YAML::Key << "XLow" << YAML::Value << pf->XFitRange.first
		<< YAML::Key << "XHigh" << YAML::Value << pf->XFitRange.second 
		<< YAML::EndMap
		<< YAML::Key << "FitName" << YAML::Value << pf->fitname;
	out << YAML::Key << "Bounded" << YAML::BeginMap;
	out << YAML::Key << "NValues" << YAML::Value << pf->bvalues.size();
	for( const auto& kv : pf->bvalues ){
		out << YAML::Key << kv.first << 
			YAML::BeginMap << 
			YAML::Key << "Min" << YAML::Value << kv.second.first <<
			YAML::Key << "Max" << YAML::Value << kv.second.second <<
			YAML::EndMap;
	}	
	out << YAML::EndMap;
	out << YAML::Key << "Fixed" << YAML::BeginMap;
	out << YAML::Key << "NValues" << YAML::Value << pf->fvalues.size();
	for( const auto& kv : pf->fvalues ){
		out << YAML::Key << kv.first <<
			YAML::BeginMap <<
			YAML::Key << "Value" << YAML::Value << kv.second <<
			YAML::EndMap;
	}
	out << YAML::EndMap;

	out 		       << YAML::Key << "Values" << YAML::Value << pf->Results 
		<< YAML::Key << "Errors" << YAML::Value << pf->Errors 
		<< YAML::Key << "ReducedChi2" << YAML::Value << pf->Results.at("Chi2")/pf->Errors.at("NDF") 
		<< YAML::EndMap;
	return out;
}

YAML::Emitter& operator << (YAML::Emitter& out, const PeakFitter2D* pf) {
	out <<  YAML::BeginMap << YAML::Key << "HisName" << YAML::Value << pf->fithist->GetName() 
		<< YAML::Key << "Range" 
		<< YAML::BeginMap 
		<< YAML::Key << "XLow" << YAML::Value << pf->XFitRange.first
		<< YAML::Key << "XHigh" << YAML::Value << pf->XFitRange.second 
		<< YAML::Key << "YLow" << YAML::Value << pf->YFitRange.first
		<< YAML::Key << "YHigh" << YAML::Value << pf->YFitRange.second 
		<< YAML::EndMap;
	out << YAML::Key << "Bounded" << YAML::BeginMap;
	out << YAML::Key << "NValues" << YAML::Value << pf->bvalues.size();
	for( const auto& kv : pf->bvalues ){
		out << YAML::Key << kv.first << 
			YAML::BeginMap << 
			YAML::Key << "Min" << YAML::Value << kv.second.first <<
			YAML::Key << "Max" << YAML::Value << kv.second.second <<
			YAML::EndMap;
	}	
	out << YAML::EndMap;
	out << YAML::Key << "Fixed" << YAML::BeginMap;
	out << YAML::Key << "NValues" << YAML::Value << pf->fvalues.size();
	for( const auto& kv : pf->fvalues ){
		out << YAML::Key << kv.first <<
			YAML::BeginMap <<
			YAML::Key << "Value" << YAML::Value << kv.second <<
			YAML::EndMap;
	}
	out << YAML::EndMap;
	out 	       << YAML::Key << "Values" << YAML::Value << pf->Results 
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
					throw std::runtime_error("Invalid bounded pair : "+s+", not two numbers");
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
	std::vector<double> low;
	std::vector<double> high;
	bool quiet;
	bool chi2;
	bool storechi2;
	int mode;
	std::map<std::string,double> fixedvalues; 
	std::map<std::string,std::pair<double,double>> boundedvalues; 
	std::vector<std::pair<double,double>> gatevalues;
	double tol;
	double ellipse;
	int npoints;
	int xrebin;
	int yrebin;

	constexpr auto oned = describe_enumerators_as_array<FitTypes::OneDim>();
	constexpr auto twod = describe_enumerators_as_array<FitTypes::TwoDim>();

	std::string FittingMessage = "peak fitting mode modenum:function";
	FittingMessage += "\n=========== 1D Fits ===========";
	for( auto const& x: oned ){
		FittingMessage += "\n"+std::to_string(x.value)+":"+std::string(x.name);
	}
	FittingMessage += "\n=========== 1D Fits ===========";
	FittingMessage += "\n=========== 2D Fits ===========";
	for( auto const& x: twod ){
		FittingMessage += "\n"+std::to_string(x.value)+":"+std::string(x.name);
	}
	FittingMessage += "\n=========== 2D Fits ===========";

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("axis,a",boost::program_options::value<std::string>(&axis)->default_value("x"),"axis to project onto (x,y,X,Y) if 2D")
		("boundparameter,b",boost::program_options::value<std::vector<std::string>>(&boundedparams)->multitoken(),"parameter to bound name:low:high")
		("chi2,c",boost::program_options::value<bool>(&chi2)->default_value(true),"chi2 fit, or loglikelihood")
		("data,d",boost::program_options::value<std::string>(&hisname),"histogram to manipulate")
		("ellipse,e",boost::program_options::value<double>(&ellipse)->default_value(3.0),"uncertainty ellipse size")
		("fixparameter,f",boost::program_options::value<std::vector<std::string>>(&fixedparams)->multitoken(),"parameter to bound name:value")
		("gate,g",boost::program_options::value<std::vector<std::string>>(&gates)->multitoken(),"values to gate within in 2d histogram")
		("help,h", "produce help message")
		("inputfile,i",boost::program_options::value<std::string>(&inputfile),"file to get the histogram from")
		("lowerbound,l",boost::program_options::value<std::vector<double>>(&low)->multitoken(),"lower bound to perform fit, if 1 provided then is XLow, if 2 provided then Xlow, Ylow")
		("mode,m",boost::program_options::value<int>(&mode)->default_value(0),FittingMessage.c_str())
		("numdimension,n",boost::program_options::value<int>(&dimensionality)->default_value(1),"dimensionality of histogram (1,2)")
		("outputprefix,o",boost::program_options::value<std::string>(&outputprefix)->default_value("GenPeakFitterResults"),"file to output to fit info to")
		("projectionindices,p",boost::program_options::value<std::vector<int>>(&indices)->multitoken(),"indices to project on if 2d histogram")
		("quiet,q",boost::program_options::value<bool>(&quiet)->default_value(false),"quiet output")
		("tolerance,r",boost::program_options::value<double>(&tol)->default_value(1.0e-6),"tolerance used to determine if we're too close to the limits")
		("storechi2,s",boost::program_options::value<bool>(&storechi2)->default_value(true),"store chi2 plot")
		("tpoints,t",boost::program_options::value<int>(&npoints)->default_value(15),"npoints in the uncertainty ellipse tcut")
		("upperbound,u",boost::program_options::value<std::vector<double>>(&high)->multitoken(),"upper bound to perform fit, if 1 provided then Xhigh, if 2 then Xhigh,Yhigh")
		("xrebin,x",boost::program_options::value<int>(&xrebin)->default_value(0),"rebin factor for the x direction")
		("yrebin,y",boost::program_options::value<int>(&yrebin)->default_value(0),"rebin factor for the y direction")
		;


	boost::program_options::positional_options_description p;

	bool is1dfit = false;
	try{
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
		notify(vm);
		if( vm.count("help") or argc <= 2 ){
			spdlog::info(cmdline_options);
			exit(EXIT_SUCCESS);
		}
		for( const auto& x : oned ){
			if( x.value == mode ){
				is1dfit = true;
				break;
			}
		}	

		bool isknownfit = false;
		for( const auto& x : oned ){
			if( x.value == mode ){
				isknownfit = true;
				break;
			}
		}
		for( const auto& x : twod ){
			if( x.value == mode ){
				isknownfit = true;
				break;
			}
		}
		if( not isknownfit ){
			spdlog::error("unknown fit type {}, see help for list of known fits. If you added one recently double check you added it to the BOOST_DESCRIBE_ENUM macro",mode);
			exit(EXIT_FAILURE);
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
		if( dimensionality == 2 and numproj < 1 and numgates < 1 and is1dfit ){
			spdlog::error("dimensionality is 2, but no projections or gates given, and not fitting a 2D dataset");
			exit(EXIT_FAILURE);
		}

		if( is1dfit ){
			if( low.size() != 1 ){
				spdlog::error("did not provide 1 lowerbound for fitting 1D function");
				exit(EXIT_FAILURE);
			}
			if( high.size() != 1 ){
				spdlog::error("did not provide 1 upperbound for fitting 1D function");
				exit(EXIT_FAILURE);
			}
		}else{
			if( low.size() != 2 ){
				spdlog::error("did not provide 2 lowerbound for fitting 2D function");
				exit(EXIT_FAILURE);
			}
			if( high.size() != 2 ){
				spdlog::error("did not provide 2 upperbound for fitting 2D function");
				exit(EXIT_FAILURE);
			}
		}
		for( size_t ii = 0; ii < low.size(); ++ii ){
			if( high[ii] < low[ii] ){
				if( ii == 0 ){
					spdlog::error("upper xbound less than lower xbound");
					exit(EXIT_FAILURE);
				}else{
					spdlog::error("upper ybound less than lower ybound");
					exit(EXIT_FAILURE);
				}
			}
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
		std::vector<PeakFitter1D*> pfs1d;
		std::vector<PeakFitter2D*> pfs2d;
		if( mainhis != nullptr ){
			auto histype = std::string(mainhis->ClassName());
			boost::regex re2d("TH2");
			boost::regex re1d("TH1");
			TH1* histofit;
			if( boost::regex_search(histype, re2d) ){
				if( is1dfit ){
					for( const auto& idx : indices ){
						auto name = std::string(mainhis->GetName())+"_proj_"+axis+std::to_string(idx);
						if( axis.compare("x") == 0 ){
							histofit = dynamic_cast<TH2*>(mainhis)->ProjectionX(name.c_str(),idx,idx);
						}else{
							histofit = dynamic_cast<TH2*>(mainhis)->ProjectionY(name.c_str(),idx,idx);
						}	
						histofit->SetDirectory(0);
						if( xrebin > 0 ){
							histofit->RebinX(xrebin);
						}
						pfs1d.push_back(new PeakFitter1D(low[0],high[0],chi2,mode,histofit,fixedvalues,boundedvalues));
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
						if( xrebin > 0 ){
							histofit->RebinX(xrebin);
						}
						pfs1d.push_back(new PeakFitter1D(low[0],high[0],chi2,mode,histofit,fixedvalues,boundedvalues));
						++idx;
					}
				}else{
					TH2* histofit2d = dynamic_cast<TH2*>(mainhis); 
					histofit2d->SetDirectory(0);
					if( xrebin > 0 ){
						histofit2d->RebinX(xrebin);
					}
					if( yrebin > 0 ){
						histofit2d->RebinY(yrebin);
					}
					pfs2d.push_back(new PeakFitter2D(low[0],high[0],low[1],high[1],chi2,mode,histofit2d,fixedvalues,boundedvalues,ellipse,npoints));
				}
			}else if( boost::regex_search(histype,re1d) ){
				histofit = dynamic_cast<TH1*>(mainhis);
				histofit->SetDirectory(0);
				pfs1d.push_back(new PeakFitter1D(low[0],high[0],chi2,mode,histofit,fixedvalues,boundedvalues));
			}else{
				throw std::runtime_error("not passed a TH1 or TH2 histogram");
			}
			auto MaxCrates = rfile->Get("MAX_CRATES");
			auto MaxCardsPerCrate = rfile->Get("MAX_CARDS_PER_CRATE");
			auto MaxChannelsPerBoard = rfile->Get("MAX_CHANNELS_PER_BOARD");
			rfile->Close();
			auto outputfile = outputprefix+".root";
			auto ofile = new TFile(outputfile.c_str(),"RECREATE");
			auto IsWithinTol = [](double t,double a,double tol){
				return std::abs(t-a) <= tol;
			};
			if( pfs1d.size() > 0 ){
				for( const auto& f : pfs1d ){
					f->WriteHistogram(storechi2);
					auto keys = f->GetKeys();
					auto bounds = f->GetBValues();
					for( const auto& kv : keys ){
						auto p = (*f)[kv.first];
						if( p.second >= std::abs(p.first) ){
							spdlog::warn("{}: The error for {} is larger than its fit value {} +- {}",
									f->GetHisName(),kv.first,p.first,p.second);
						}
						auto bres = bounds.find(kv.first);
						if( bres != bounds.end() ){
							if( IsWithinTol(p.first,bres->second.first,tol) ){
								spdlog::warn("{}: {} is at the lower limit of its bounds [{},{},{}]",
										f->GetHisName(),
										kv.first,bres->second.first,p.first,bres->second.second);
							}
							if( IsWithinTol(p.first,bres->second.second,tol) ){
								spdlog::warn("{}: {} is at the upper limit of its bounds [{},{},{}]",
										f->GetHisName(),
										kv.first,bres->second.first,p.first,bres->second.second);
							}
						}
					}
				}
			}
			if( pfs2d.size() > 0 ){
				for( const auto& f : pfs2d ){
					f->WriteHistogram(storechi2);
					auto keys = f->GetKeys();
					auto bounds = f->GetBValues();
					for( const auto& kv : keys ){
						auto p = (*f)[kv.first];
						if( p.second >= std::abs(p.first) ){
							spdlog::warn("{}: The error for {} is larger than its fit value {} +- {}",
									f->GetHisName(),kv.first,p.first,p.second);
						}
						auto bres = bounds.find(kv.first);
						if( bres != bounds.end() ){
							if( IsWithinTol(p.first,bres->second.first,tol) ){
								spdlog::warn("{}: {} is at the lower limit of its bounds [{},{},{}]",
										f->GetHisName(),
										kv.first,bres->second.first,p.first,bres->second.second);
							}
							if( IsWithinTol(p.first,bres->second.second,tol) ){
								spdlog::warn("{}: {} is at the upper limit of its bounds [{},{},{}]",
										f->GetHisName(),
										kv.first,bres->second.first,p.first,bres->second.second);
							}
						}
					}
				}
			}
			ofile->Close();
			YAML::Emitter  doc;
			doc << YAML::BeginMap;
			doc << YAML::Key << "InputFile" << YAML::Value << inputfile;
			doc << YAML::Key << "InputHistogram" << YAML::Value << hisname;
			doc << YAML::Key << "Ellipse" << YAML::Value << ellipse;
			doc << YAML::Key << "NPoints" << YAML::Value << npoints;
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
			if( pfs1d.size() > 0 ){
				doc << pfs1d;
			}
			if( pfs2d.size() > 0 ){
				doc << pfs2d;
			}
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
