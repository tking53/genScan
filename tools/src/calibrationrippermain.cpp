#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <ostream>
#include <spdlog/fmt/fmt.h>
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

#include <pugixml.hpp>

#include <PolyCalibrator.hpp>

YAML::Emitter& operator << (YAML::Emitter& out, const PolyCalibrator* pf) {
	out <<  YAML::BeginMap 
		<< YAML::Key << "HisName" << YAML::Value << pf->FitName
		<< YAML::Key << "Coefficients" << pf->Results	
		<< YAML::EndMap;
	return out;
}

struct txtripper{
	int crateid;
	int modid;
	int chanid;
	std::vector<double> pars;
};

struct calibrationripper{
	std::string filename;
	std::pair<double,double> peakvalue;
	std::map<std::string,std::pair<double,double>> fitvals;
	std::string hisname;
	int MaxCrates;
	int MaxCardsPerCrate;
	int MaxChannelsPerBoard;

	calibrationripper(std::string s,bool usefiterror,const std::string parname) {
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
		MaxCrates = doc["MAX_CRATES"].as<int>(1);
		MaxCardsPerCrate = doc["MAX_CARDS_PER_CRATE"].as<int>(13);
		MaxChannelsPerBoard = doc["MAX_CHANNELS_PER_BOARD"].as<int>(16);
		std::set<std::string> names;
		for( size_t ii = 0; ii < results.size(); ++ii ){
			auto fitname = results[ii]["HisName"].as<std::string>();
			auto mean = results[ii]["Values"][parname].as<double>();
			auto meanerr = results[ii]["Errors"][parname].as<double>();
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
	std::string logfile; 
	std::string txtfile;
	std::string parname;
	int order;
	bool fixcontstant;
	bool usefiterror;
	bool apply;
	bool txtmode;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("apply,a",boost::program_options::value<bool>(&apply)->default_value(true),"apply to a configfile")
		("configfile,c",boost::program_options::value<std::string>(&configfile),"configfile to read in and adjust")
		("error,e",boost::program_options::value<bool>(&usefiterror)->default_value(true),"use the fit error from the file")
		("fitpoints,f",boost::program_options::value<std::vector<std::string>>(&fitfiles)->multitoken(),"Add file:energy pair (e.g. fit.yaml:661.657:pkerr, pkerr is optional)")
		("help,h", "produce help message")
		("logfile,l",boost::program_options::value<std::string>(&logfile)->default_value("GenCalRipper.yaml"),"log file to output new calibration params to")
		("mode,m",boost::program_options::value<bool>(&txtmode)->default_value(false),"operate in mode where we parse a txt file instead of yaml, see txtfile option for more info")
		("namedparameter,n",boost::program_options::value<std::string>(&parname)->default_value("Mean"),"parameter name used to gen calibration for")
		("outputfile,o",boost::program_options::value<std::string>(&outputfile),"configfile to output to")
		("polyorder,p",boost::program_options::value<int>(&order)->default_value(1),"order to do calibration")
		("scale,s",boost::program_options::value<bool>(&fixcontstant)->default_value(true),"fix the constant term in the fit")
		("txtfile,t",boost::program_options::value<std::string>(&txtfile)->default_value("CalMap.txt"),"txt file formatted as crate module channel pars")
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

		if( not txtmode ){
			std::set<int> CrateMax;
			std::set<int> BoardMax;
			std::set<int> ChannelMax;
			for( const auto& f : fitfiles ){
				calpoints.push_back(calibrationripper(f,usefiterror,parname));
				auto i = calpoints.back().MaxCrates;
				if( CrateMax.empty() ){
					CrateMax.insert(i);
				}else{
					if( CrateMax.find(i) == CrateMax.end() ){
						throw std::runtime_error("Found multiple crate settings");
					}
				}
				auto j = calpoints.back().MaxCardsPerCrate;
				if( BoardMax.empty() ){
					BoardMax.insert(j);
				}else{
					if( BoardMax.find(j) == BoardMax.end() ){
						throw std::runtime_error("Found multiple board settings");
					}
				}
				auto k = calpoints.back().MaxChannelsPerBoard;
				if( ChannelMax.empty() ){
					ChannelMax.insert(k);
				}else{
					if( ChannelMax.find(k) == ChannelMax.end() ){
						throw std::runtime_error("Found multiple channel settings");
					}
				}

			}
			auto j = (*BoardMax.begin());
			auto k = (*ChannelMax.begin());

			if( not vm.count("outputfile") and apply ){
				outputfile = configfile+".calibrated";
			}

			std::set<std::string> names;
			for( const auto& c : calpoints ){
				for( const auto& kv : c.fitvals ){
					names.insert(kv.first);
				}
			}

			std::vector<std::unique_ptr<PolyCalibrator>> currcal;
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
				currcal.push_back(std::make_unique<PolyCalibrator>(fitpoints,fixcontstant,order,k));
				doc << currcal.back().get();
			}
			doc << YAML::EndSeq << YAML::EndMap;
			std::ofstream yfile(logfile);
			yfile << doc.c_str() << std::endl;
			yfile.close();

			if( apply ){
				pugi::xml_document inputconfig;
				auto loadres = inputconfig.load_file(configfile.c_str());
				if( not loadres ){
					throw std::runtime_error(loadres.description());
				}

				pugi::xml_node Configuration = inputconfig.child("Configuration");
				pugi::xml_node Map = Configuration.child("Map");
				auto calc_gchid = [&j,&k](const int& a,const int& b, const int& c){
					return a*j*k + b*k + c + 1;
				};
				for( pugi::xml_node Crate = Map.child("Crate"); Crate; Crate = Crate.next_sibling("Crate") ){
					auto a = Crate.attribute("number").as_int();
					for( pugi::xml_node Module = Crate.child("Module"); Module; Module = Module.next_sibling("Module") ){
						auto b = Module.attribute("number").as_int();
						for( pugi::xml_node Channel = Module.child("Channel"); Channel; Channel = Channel.next_sibling("Channel") ){
							auto cid = Channel.attribute("number").as_int();
							auto gchid = calc_gchid(a,b,cid);
							for( const auto& c : currcal ){
								std::size_t found = c->FitName.find_last_of("_x");
								auto cgChID = std::stoi(c->FitName.substr(found+1));
								if( gchid == cgChID ){
									std::string newvalue = "";
									for( const auto& kv : c->Results ){
										if( kv.second == 0.0 ){
											newvalue += fmt::format("{:.1f} ",kv.second);
										}else if( std::abs(kv.second) < 1.0e-3 or std::abs(kv.second) > 1.0e3 ){
											newvalue += fmt::format("{:.6e} ",kv.second);
										}else{
											newvalue += fmt::format("{:.6f} ",kv.second);
										}
									}
									pugi::xml_node Calibration = Channel.child("Calibration");
									if( Calibration ){
										Calibration.text() = newvalue.c_str();
										switch(c->Results.size()){
											case 2:
												Calibration.attribute("model") = "linear";
												break;
											case 3:
												Calibration.attribute("model") = "quadratic";
												break;
											case 4:
												Calibration.attribute("model") = "cubic";
												break;
											default:
												Calibration.attribute("model") = "unknown";
												break;
										}
									}
									break;
								}
							}
						}
					}
				}
				inputconfig.save_file(outputfile.c_str());
			}
		}else{
			std::vector<txtripper> cals;

			std::string line;
			std::ifstream input(txtfile);
			while( std::getline(input,line) ){
				std::stringstream ss(line);
				cals.push_back(txtripper());
				ss >> cals.back().crateid;
				ss >> cals.back().modid;
				ss >> cals.back().chanid;

				double val;
				std::vector<double> data;
				while( ss >> val ){
					data.push_back(val);
				}
				cals.back().pars = data;		
			}
			input.close();

			//for( const auto& e : cals ){
			//	std::cout << e.crateid << " " << e.modid << " " << e.chanid << " ";
			//	for( const auto& v : e.pars ){
			//		std::cout << v << " ";
			//	}
			//	std::cout << std::endl;
			//}

			if( apply ){
				pugi::xml_document inputconfig;
				auto loadres = inputconfig.load_file(configfile.c_str());
				if( not loadres ){
					throw std::runtime_error(loadres.description());
				}

				auto IsCorrectChannel = [](int x,int y,int z,const txtripper& t){
					return x == t.crateid and y == t.modid and z == t.chanid;
				};

				pugi::xml_node Configuration = inputconfig.child("Configuration");
				pugi::xml_node Map = Configuration.child("Map");
				for( pugi::xml_node Crate = Map.child("Crate"); Crate; Crate = Crate.next_sibling("Crate") ){
					auto a = Crate.attribute("number").as_int();
					for( pugi::xml_node Module = Crate.child("Module"); Module; Module = Module.next_sibling("Module") ){
						auto b = Module.attribute("number").as_int();
						for( pugi::xml_node Channel = Module.child("Channel"); Channel; Channel = Channel.next_sibling("Channel") ){
							auto cid = Channel.attribute("number").as_int();
							for( const auto& c : cals ){
								if( IsCorrectChannel(a,b,cid,c) ){
									std::string newvalue = "";
									for( const auto& kv : c.pars ){
										if( kv == 0.0 ){
											newvalue += fmt::format("{:.1f} ",kv);
										}else if( std::abs(kv) < 1.0e-3 or std::abs(kv) > 1.0e3 ){
											newvalue += fmt::format("{:.6e} ",kv);
										}else{
											newvalue += fmt::format("{:.6f} ",kv);
										}
									}
									pugi::xml_node Calibration = Channel.child("Calibration");
									if( Calibration ){
										Calibration.text() = newvalue.c_str();
										switch(c.pars.size()){
											case 2:
												Calibration.attribute("model") = "linear";
												break;
											case 3:
												Calibration.attribute("model") = "quadratic";
												break;
											case 4:
												Calibration.attribute("model") = "cubic";
												break;
											default:
												Calibration.attribute("model") = "unknown";
												break;
										}
									}
									break;
								}
							}
						}
					}
				}
				inputconfig.save_file(outputfile.c_str());
			}

		}
	}catch( std::exception& e){
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}    
}
