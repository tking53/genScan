#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include <fstream>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>

int main(int argc, char* argv[]) {
	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()("help,h", "produce help message");

	boost::program_options::positional_options_description p;

	try {
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
		notify(vm);
		if (vm.count("help") or argc <= 2) {
			spdlog::info(cmdline_options);
			exit(EXIT_SUCCESS);
		}
	} catch (std::exception& e) {
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}
}
