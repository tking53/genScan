#include <memory>
#include <random>
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

template<class T>
void TimeCost(const std::vector<ReInfo>& g,const std::vector<ReInfo>& b,const std::vector<ReInfo>& a,boost::regex& r,int ntimes,std::vector<int> indices,T func){
	std::chrono::time_point<std::chrono::high_resolution_clock> global_start_time = std::chrono::high_resolution_clock::now();
	for( int ii = 0; ii < ntimes; ++ii ){
		auto uid = func(g,b,a,indices[ii],r);
	}
	std::chrono::time_point<std::chrono::high_resolution_clock> global_stop_time = std::chrono::high_resolution_clock::now();
	auto global_run_time = global_stop_time - global_start_time;

	const auto hrs = std::chrono::duration_cast<std::chrono::hours>(global_run_time);
	const auto mins = std::chrono::duration_cast<std::chrono::minutes>(global_run_time - hrs);
	const auto secs = std::chrono::duration_cast<std::chrono::seconds>(global_run_time - hrs - mins);
	const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(global_run_time - hrs - mins - secs);
	spdlog::info("Finished running in {} hours {} minutes {} seconds {} milliseconds",
			hrs.count(),mins.count(),secs.count(),ms.count());
}

int main(int argc, char *argv[]) {

	std::string restr;
	std::string configfile;
	int ntimes;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("regex,r",boost::program_options::value<std::string>(&restr),"input regex")
		("configfile,c",boost::program_options::value<std::string>(&configfile),"config file to run the regex checks on")
		("ntimes,n",boost::program_options::value<int>(&ntimes)->default_value(0),"number of times to test for timing purposes")
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
		std::vector<ReInfo> All;
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
						All.push_back({crid,bid,cid,uid});
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
		if( ntimes > 0 ){
			std::random_device rd;  // a seed source for the random number engine
			std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()

			if( not Good.empty() ){
				std::vector<int> successindices;
				std::uniform_int_distribution<> g(0,Good.size()-1);
				for( int ii = 0; ii < ntimes; ++ii ){
					successindices.push_back(g(gen));
				}

				TimeCost(Good,Bad,All,re,ntimes,successindices,
						[](const std::vector<ReInfo>& g,const std::vector<ReInfo>& b,const std::vector<ReInfo>& a,int idx,boost::regex& re){
						boost::smatch type_match;
						if( boost::regex_match(g[idx].UIDs,type_match,re,boost::regex_constants::match_continuous) ){
						return g[idx].UIDs;

						}
						return std::string();
						});
			}

			if( not Bad.empty() ){
				std::vector<int> failindices;
				std::uniform_int_distribution<> g(0,Bad.size()-1);
				for( int ii = 0; ii < ntimes; ++ii ){
					failindices.push_back(g(gen));
				}

				TimeCost(Good,Bad,All,re,ntimes,failindices,
						[](const std::vector<ReInfo>& g,const std::vector<ReInfo>& b,const std::vector<ReInfo>& a,int idx,boost::regex& re){
						boost::smatch type_match;
						if( boost::regex_match(b[idx].UIDs,type_match,re,boost::regex_constants::match_continuous) ){
						return b[idx].UIDs;

						}
						return std::string();
						});
			}

			std::vector<int> mixedindices;
			std::uniform_int_distribution<> g(0,All.size()-1);
			for( int ii = 0; ii < ntimes; ++ii ){
				mixedindices.push_back(g(gen));
			}

			TimeCost(Good,Bad,All,re,ntimes,mixedindices,
					[](const std::vector<ReInfo>& g,const std::vector<ReInfo>& b,const std::vector<ReInfo>& a,int idx,boost::regex& re){
					boost::smatch type_match;
					if( boost::regex_match(a[idx].UIDs,type_match,re,boost::regex_constants::match_continuous) ){
					return a[idx].UIDs;

					}
					return std::string();
					});
		}

	}catch( std::exception& e){
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}    

}
