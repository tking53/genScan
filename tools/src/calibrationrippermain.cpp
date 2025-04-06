#include "XMLConfigParser.hpp"
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <utility>
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
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include <yaml-cpp/emitter.h>
#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>

#include <PolyCalibrator.hpp>

YAML::Emitter& operator << (YAML::Emitter& out, const PolyCalibrator* pf) {
	out <<  YAML::BeginMap 
		<< YAML::Key << "HisName" << YAML::Value << pf->gChID
	        << YAML::Key << "Coefficients" << pf->Results	
	     << YAML::EndMap;
	return out;
}


struct calibrationripper{
	std::string filename;
	std::pair<double,double> peakvalue;
	std::map<std::string,std::pair<double,double>> fitvals;
	std::string hisname;

	calibrationripper(std::string s,bool usefiterror) {
		std::vector<std::string> strs;
		boost::split(strs,s,boost::is_any_of(":"));
		boost::regex fre(".*\\.((yaml)|(yml)){1}");
		boost::regex number("^(0|[1-9]\\d*)(\\.\\d+)?(e-?(0|[1-9]\\d*))?");
	
		if( strs.size() == 2 or strs.size() == 3 ){
			boost::smatch pmatch;
			boost::smatch ematch;
			if( boost::regex_search(strs[0],fre) ){
				filename = strs[0];
				if( strs.size() == 2 ){
					if( boost::regex_match(strs[1],pmatch,number) ){
						peakvalue = {std::stod(strs[1]),1.0e-6};
					}else{
						throw std::runtime_error("Invalid peak given");
					}
				}else{
					if( boost::regex_match(strs[1],pmatch,number) and boost::regex_match(strs[2],ematch,number) ){
						peakvalue = {std::stod(strs[1]),std::stod(strs[2])};
					}else{
						throw std::runtime_error("Invalid peak given");
					}
				}
			}else{
				throw std::runtime_error("Not given yaml/yml file from GenPeakFit");
			}
		}else{
			throw std::runtime_error("Unable to parse input");
		}
		YAML::Node doc = YAML::LoadFile(filename);
		auto results = doc["FitResults"];
		hisname = doc["InputHistogram"].as<std::string>();
		std::set<std::string> names;
		for( size_t ii = 0; ii < results.size(); ++ii ){
			auto fitname = results[ii]["HisName"].as<std::string>();
			auto mean = results[ii]["Values"]["Mean"].as<double>();
			auto meanerr = results[ii]["Errors"]["Mean"].as<double>();
			if( names.find(fitname) != names.end() ){
				throw std::runtime_error("Parsing "+filename+" found duplicate fit : "+fitname);
			}else{
				fitvals[fitname] = usefiterror ? std::make_pair(mean,meanerr) : std::make_pair(mean,1.0e-6);
				names.insert(fitname);
			}
		}
	}

};

int main(int argc, char *argv[]) {

	std::vector<std::string> fitfiles;
	std::vector<calibrationripper> calpoints;
	std::string configfile;
	std::string outputfile; 
	int order;
	bool fixcontstant;
	bool usefiterror;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("fitpoints,f",boost::program_options::value<std::vector<std::string>>(&fitfiles)->multitoken(),"Add file:energy pair (e.g. fit.yaml:661.657:pkerr, pkerr is optional)")
		("outputfile,o",boost::program_options::value<std::string>(&outputfile)->default_value("GenCalRipper.yaml"),"cal file to output to")
		("polyorder,p",boost::program_options::value<int>(&order)->default_value(1),"order to do calibration")
		("scale,s",boost::program_options::value<bool>(&fixcontstant)->default_value(true),"fix the constant term in the fit")
		("error,e",boost::program_options::value<bool>(&usefiterror)->default_value(true),"use the fit error from the file")
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

		for( const auto& f : fitfiles ){
			calpoints.push_back(calibrationripper(f,usefiterror));
		}

		std::set<std::string> names;
		for( const auto& c : calpoints ){
			for( const auto& kv : c.fitvals ){
				names.insert(kv.first);
			}
		}

		std::unique_ptr<PolyCalibrator> currcal;
		YAML::Emitter doc;
		doc << YAML::BeginMap << YAML::Key << "Calibration" << YAML::BeginSeq;
		for( const auto& k : names ){
			std::vector<calibrationpoint> fitpoints;
			for( const auto& c : calpoints ){
				auto search = c.fitvals.find(k);
				if( search != c.fitvals.end() ){
					fitpoints.push_back({.energy=c.peakvalue,.channel=search->second});
				}
			}
			currcal.reset(new PolyCalibrator(fitpoints,fixcontstant,order,k));
			doc << currcal.get();
		}
		doc << YAML::EndSeq << YAML::EndMap;
		std::ofstream yfile(outputfile);
		yfile << doc.c_str() << std::endl;
		yfile.close();

	}catch( std::exception& e){
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}    
}
