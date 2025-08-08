#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
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

typedef std::map<std::string,double> tofshift;

struct pidprocessor_predicate {
	bool operator()(pugi::xml_attribute attr) const {
		return strcmp(attr.name(), "name") == 0;
	}

	bool operator()(pugi::xml_node node) const {
		return strcmp(node.attribute("name").as_string(""),"PidProcessor") == 0;
	}
};

int main(int argc, char *argv[]) {

	std::string configfile;
	std::string outputdir;
	std::vector<std::string> yamlfiles;
	std::vector<std::string> fitfiles;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("file,f",boost::program_options::value<std::vector<std::string>>(&yamlfiles),"[file1 file2 file3 ...] list of files used for input")
		("pidmap,p",boost::program_options::value<std::vector<std::string>>(&fitfiles)->multitoken(),"Add PID_X:id pair (e.g. PID_7:6)")
		("configfile,c",boost::program_options::value<std::string>(&configfile),"configfile to read in and regenerate new configs from")
		("outputdir,o",boost::program_options::value<std::string>(&outputdir),"directory to output to, name will be based on the parse rootfile name in the input yaml")
		;


	boost::program_options::positional_options_description p;
        p.add("file", -1);

	try{
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
		notify(vm);
		if( vm.count("help") or argc <= 2 ){
			spdlog::info(cmdline_options);
			exit(EXIT_SUCCESS);
		}

		std::map<std::string,tofshift> shifts;
		//open a yaml file
		for( const auto& f : yamlfiles ){
			YAML::Node doc = YAML::LoadFile(f);
			std::string hisname = doc["HisName"].as<std::string>();
			auto results = doc["Results"];
			for( size_t ii = 0; ii < results.size(); ++ii ){
				auto filename = results[ii]["FileName"].as<std::string>();
				auto mean = results[ii]["Mean"].as<double>();
				shifts[filename][hisname] = mean;
			}
		}

		for( const auto& kv : shifts ){
			pugi::xml_document inputconfig;
			auto loadres = inputconfig.load_file(configfile.c_str());
			if( not loadres ){
				throw std::runtime_error(loadres.description());
			}

			pugi::xml_node Configuration = inputconfig.child("Configuration");
			auto proc = Configuration.find_node(pidprocessor_predicate());
			if( proc ){
				//we've got a fresh copy of the xml, so we just update as needed and dump to new file based on the parsed name of kv.first
				for( const auto& id_val : kv.second ){
					//need to add node for each of the id based on the provided mapping
				}
			}else{
				throw std::runtime_error("No PidProcessor found in the Config File");
			}
		}

	}catch( std::exception& e){
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}    
}
