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

struct processor_predicate {
	std::string id;

	processor_predicate(const std::string& val) : id(val){
	}

	bool operator()(pugi::xml_attribute attr) const {
		return strcmp(attr.name(), "name") == 0;
	}

	bool operator()(pugi::xml_node node) const {
		return strcmp(node.attribute("name").as_string(""),id.c_str()) == 0;
	}
};

int main(int argc, char *argv[]) {

	std::string configfile;
	std::vector<std::string> procs;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("names,n",boost::program_options::value<std::vector<std::string>>(&procs),"[name1 name2 ... nameN] names of the processors/analyzers to dump info about")
		("configfile,c",boost::program_options::value<std::string>(&configfile),"configfile to read in and print the portions of the config which have name=name1 or name=name2, etc.")
		;


	boost::program_options::positional_options_description p;
	p.add("names",-1);

	try{
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
		notify(vm);
		if( vm.count("help") or argc <= 2 ){
			spdlog::info(cmdline_options);
			exit(EXIT_SUCCESS);
		}

		pugi::xml_document inputconfig;
		auto loadres = inputconfig.load_file(configfile.c_str());
		if( not loadres ){
			throw std::runtime_error(loadres.description());
		}

		pugi::xml_node Configuration = inputconfig.child("Configuration");
		for( const auto& n : procs ){
			auto proc = Configuration.find_node(processor_predicate(n));
			if( proc ){
				proc.print(std::cout);
			}
		}

	}catch( std::exception& e){
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}    
}
