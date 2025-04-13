#include <memory>
#include <set>
#include <stdexcept>
#include <string>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include <pugixml.hpp>
#include <tuple>

struct ReInfo{
	std::tuple<int,int,int> CrBCIDs;
	std::string UIDs;
	ReInfo(int crid,int bid,int cid,const std::string& uid) : CrBCIDs({crid,bid,cid}), UIDs(uid){
	}	
	
	template<typename OStream>
	friend OStream& operator<<(OStream& os, const ReInfo& r) {
		os << "Crate : " << std::get<0>(r.CrBCIDs)
		   << " Module : " << std::get<1>(r.CrBCIDs)
	   	   << " Channel : " << std::get<2>(r.CrBCIDs)
	           << " uid : " << r.UIDs;	   
		return os;
	}

};

int main(int argc, char *argv[]) {

	std::string restr;
	std::string configfile;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("regex,r",boost::program_options::value<std::string>(&restr),"input regex")
		("configfile,c",boost::program_options::value<std::string>(&configfile),"config file to run the regex checks on")
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

		if( not vm.count("regex") ){
			throw std::runtime_error("Missing regex");
		}

		if( not vm.count("configfile") ){
			throw std::runtime_error("Missing configfile");
		}

		boost::regex re(restr);
		pugi::xml_document inputfile;
		auto res = inputfile.load_file(configfile.c_str());
		if( !res ){
			throw std::runtime_error(res.description());
		}	

		std::vector<ReInfo> Good;
		std::vector<ReInfo> Bad;
		auto Configuration = inputfile.child("Configuration");
		auto Map = Configuration.child("Map");
		for( auto Crate = Map.child("Crate"); Crate; Crate = Crate.next_sibling("Crate") ){
			auto crid = Crate.attribute("number").as_int();
			for( auto Module = Crate.child("Module"); Module; Module = Module.next_sibling("Module") ){
				auto bid = Module.attribute("number").as_int();
				for( auto Channel = Module.child("Channel"); Channel; Channel = Channel.next_sibling("Channel") ){
						auto cid = Channel.attribute("number").as_int();
						std::string type = Channel.attribute("type").as_string("");
						std::string subtype = Channel.attribute("subtype").as_string("");
						std::string group = Channel.attribute("group").as_string("");
						std::set<std::string> taglist = {};
						std::string tags = Channel.attribute("tags").as_string("");
						boost::regex word_regex("(\\w+)");
						auto words_begin = boost::sregex_iterator(tags.begin(),tags.end(), word_regex);
						auto words_end = boost::sregex_iterator();
						for (boost::sregex_iterator i = words_begin; i != words_end; ++i) {
							boost::smatch match = *i;
							taglist.insert(match.str());
						}
						std::string uid = type + ":" + subtype + ":" + group;
						for( auto& currtag : taglist ){
							uid += ":" + currtag;
						}
						if( boost::regex_match(uid,re) ){
							Good.push_back({crid,bid,cid,uid});
						}else{
							Bad.push_back({crid,bid,cid,uid});
						}
				}
			}	
		}
		spdlog::info("=============================SUCCESSFUL=============================");
		for( const auto& g : Good ){
			spdlog::info("{}",g);
		}
		spdlog::info("=============================SUCCESSFUL=============================");
		spdlog::error("===========================NOT SUCCESSFUL=============================");
		for( const auto& b : Bad ){
			spdlog::error("{}",b);
		}
		spdlog::error("===========================NOT SUCCESSFUL=============================");

	}catch( std::exception& e){
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}    

}
