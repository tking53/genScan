#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>
#include <boost/math/interpolators/makima.hpp>
#include <boost/math/interpolators/cardinal_cubic_b_spline.hpp>
#include <boost/math/interpolators/cardinal_quadratic_b_spline.hpp>

int main(int argc, char *argv[]) {
	std::string inputfile = "trace.txt";
	std::string outputfile = "makima.txt";
	int npts = 1000;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("inputfile,i",boost::program_options::value<std::string>(&inputfile)->default_value("trace.txt"),"file to read the trace data in formatted as x y_i")
		("outputfile,o",boost::program_options::value<std::string>(&outputfile)->default_value("fit.txt"),"file to write the trace fit in formatted as x y_i y_f")
		("npts,n",boost::program_options::value<int>(&npts)->default_value(1000),"number of points to dump for the spline")
		;

	boost::program_options::positional_options_description pos;

	try{
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(pos).run(), vm);
		notify(vm);
		if( vm.count("help") or argc <= 2 ){
			spdlog::info(cmdline_options);
			exit(EXIT_SUCCESS);
		}
	}catch( std::exception& e){
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}    

	double currx,curry;
	std::vector<double> tracexvals;
	std::vector<double> traceyvals;
	std::ifstream input(inputfile);
	while( input >> currx >> curry ){
		tracexvals.push_back(currx);
		traceyvals.push_back(curry);
	}
	input.close();

	auto xmin = tracexvals[0];
	auto xmax = tracexvals[tracexvals.size()-1];
	auto delta = (xmax - xmin)/static_cast<double>(npts-1);

	auto cubic_b_spline = boost::math::interpolators::cardinal_cubic_b_spline(traceyvals.begin(),traceyvals.end(),xmin,tracexvals[1]-xmin);
	auto quadratic_b_spline = boost::math::interpolators::cardinal_quadratic_b_spline(traceyvals,xmin,tracexvals[1]-xmin);
	
	auto start_time = std::chrono::high_resolution_clock::now();
	auto makima_spline = boost::math::interpolators::makima(std::move(tracexvals),std::move(traceyvals));
	auto stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float,std::milli> dur = (stop_time - start_time);
	std::cout << dur.count() << std::endl;

	std::ofstream out(outputfile);
	for( int ii = 0; ii < npts; ++ii ){
		currx = xmin+delta*ii;
		curry = makima_spline(currx);
		auto curry_cubic_b = cubic_b_spline(currx);
		auto curry_quadratic_b = quadratic_b_spline(currx);
		out << currx << " " << curry << " " << curry_cubic_b << " " << curry_quadratic_b << std::endl;
	}
	out.close();

	return 0;

}
