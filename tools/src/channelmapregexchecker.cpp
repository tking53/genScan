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

int main(int argc, char* argv[]) {
	int numcrate;
	int nummodule;
	int numchannel;
	std::string crateregex;
	std::string moduleregex;
	std::string channelregex;
	bool individualmode;

	// clang-format off
	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message notable regex patterns All Numbers 0-15: (0|[1-9]|1[0-5]) All Even Numbers : \\d*[02468] All Odd Numbers : \\d*[13579]")
		("numcrate,x", boost::program_options::value<int>(&numcrate)->default_value(1), "number of crates in default xml")
		("nummodule,y", boost::program_options::value<int>(&nummodule)->default_value(13), "number of modules per crate in default xml")
		("numchannel,z", boost::program_options::value<int>(&numchannel)->default_value(16), "number of channels per module in default xml")
		("crateregex,a", boost::program_options::value<std::string>(&crateregex), "crate regex")
		("moduleregex,b", boost::program_options::value<std::string>(&moduleregex), "module regex")
		("channelregex,c", boost::program_options::value<std::string>(&channelregex), "channel regex")
		("individualmode,i", boost::program_options::value<bool>(&individualmode)->default_value(false), "run the full regex pattern, or as individual sections");

	boost::program_options::positional_options_description p;
	// clang-format on

	try {
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
		notify(vm);
		if (vm.count("help") or argc <= 2) {
			spdlog::info(cmdline_options);
			exit(EXIT_SUCCESS);
		}

		if (not vm.count("crateregex")) {
			throw std::runtime_error("Missing crateregex");
		}

		if (not vm.count("moduleregex")) {
			throw std::runtime_error("Missing moduleregex");
		}

		if (not vm.count("channelregex")) {
			throw std::runtime_error("Missing channelregex");
		}

		if (not individualmode) {
			boost::regex re("^" + crateregex + ":" + moduleregex + ":" + channelregex + "$");
			for (int ii = 0; ii < numcrate; ++ii) {
				for (int jj = 0; jj < nummodule; ++jj) {
					for (int kk = 0; kk < numchannel; ++kk) {
						auto currcmapid = std::to_string(ii) + ":" + std::to_string(jj) + ":" + std::to_string(kk);
						boost::smatch cmapmatch;
						if (boost::regex_match(currcmapid, cmapmatch, re, boost::regex_constants::match_continuous)) {
							spdlog::info("SUCCESS: {} on Crate : {} Module : {} Channel : {}", re.str(), ii, jj, kk);
						} else {
							spdlog::error("FAILURE: {} on Crate : {} Module : {} Channel : {}", re.str(), ii, jj, kk);
						}
					}
				}
			}
		} else {
			auto test_all = [](const std::string& restr, const std::string& type, int maxidx) {
				boost::regex re(restr);
				for (int ii = 0; ii < maxidx; ++ii) {
					auto currcmapid = std::to_string(ii);
					boost::smatch cmapmatch;
					if (boost::regex_match(currcmapid, cmapmatch, re, boost::regex_constants::match_continuous)) {
						spdlog::info("SUCCESS: {}  on {} : {}", restr, type, ii);
					} else {
						spdlog::error("FAILURE: {}  on {} : {}", restr, type, ii);
					}
				}
			};

			test_all(crateregex, "Crate", numcrate);
			test_all(moduleregex, "Module", nummodule);
			test_all(channelregex, "Channel", numchannel);
		}

	} catch (std::exception& e) {
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}
}
