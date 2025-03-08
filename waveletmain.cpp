#include <cstdlib>
#include <iostream>
#include <chrono>
#include <memory>
#include <vector>
#include <fstream>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>

#include <gsl/gsl_sort.h>
#include <gsl/gsl_wavelet.h>

int main(int argc, char *argv[]) {
	std::string inputfile = "trace.txt";
	std::string outputfile = "fit.txt";
	size_t nkeep = 20;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("inputfile,i",boost::program_options::value<std::string>(&inputfile)->default_value("trace.txt"),"file to read the trace data in formatted as x y_i")
		("outputfile,o",boost::program_options::value<std::string>(&outputfile)->default_value("wavelet.txt"),"file to write the trace fit in formatted as x y_i y_f")
		("nkeep,n",boost::program_options::value<size_t>(&nkeep)->default_value(20),"number of wavelets to keep")
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
	auto numadd = [](size_t v){
		size_t p = v;
		p--;
		p |= v >> 1;
		p |= v >> 2;
		p |= v >> 4;
		p |= v >> 8;
		p |= v >> 16;
		p++;
		return v - p;
	};
	for( size_t ii = 0; ii < numadd(traceyvals.size()); ++ii ){
		traceyvals.push_back(0.0);
	}

	gsl_wavelet* w = gsl_wavelet_alloc (gsl_wavelet_daubechies, 4);
	gsl_wavelet_workspace* work = gsl_wavelet_workspace_alloc (traceyvals.size());
	std::shared_ptr<double[]> data(new double[traceyvals.size()]);
	for( size_t ii = 0; ii < traceyvals.size(); ++ii ){
		data[ii] = traceyvals[ii];
	}
	gsl_wavelet_transform_forward (w, data.get(), 1, traceyvals.size(), work);

	std::shared_ptr<double[]> abscoeff(new double[traceyvals.size()]);
	for( size_t ii = 0; ii < traceyvals.size(); ++ii ){
		abscoeff[ii] = std::abs(data[ii]);
	}
	std::shared_ptr<size_t[]> idx(new size_t[traceyvals.size()]);
	gsl_sort_index (idx.get(), abscoeff.get(), 1, traceyvals.size());
	for( size_t ii = 0; ii < traceyvals.size(); ++ii ){
		if( (ii + nkeep) < traceyvals.size() ){
			data[idx[ii]] = 0.0;
		}
	}
	gsl_wavelet_transform_inverse (w, data.get(), 1, traceyvals.size(), work);

	std::ofstream out(outputfile);
	for( size_t ii = 0; ii < tracexvals.size(); ++ii ){
		out << tracexvals[ii] << ' ' << traceyvals[ii] << ' ' << data[ii] << std::endl;
	}
	out.close();

	return 0;

}
