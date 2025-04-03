#include <cstdlib>
#include <iostream>
#include <numeric>
#include <vector>
#include <fstream>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>
#include <boost/circular_buffer.hpp>

int main(int argc, char *argv[]) {
	std::string inputfile = "trace.txt";
	std::string outputfile = "fit.txt";
	double tau = 0.0;
	int l = 0;
	int g = 0;
	int blen = 0;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("inputfile,i",boost::program_options::value<std::string>(&inputfile)->default_value("trace.txt"),"file to read the trace data in formatted as x y_i")
		("outputfile,o",boost::program_options::value<std::string>(&outputfile)->default_value("trap.txt"),"file to write the trace trapezoid in formatted as x y_i t_i")
		("tau,t",boost::program_options::value<double>(&tau)->default_value(1.0),"tau value used for pole zero correction if <= 0.0 no pole zero performed")
		("length,l",boost::program_options::value<int>(&l)->default_value(1),"length of filter in samples")
		("gap,g",boost::program_options::value<int>(&g)->default_value(0),"gap of filter in samples")
		("baseline,b",boost::program_options::value<int>(&blen)->default_value(0),"length of region in samples to use for baseline")
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

	if( l < 1 ){
		spdlog::error("inappropriate length < 1, provided : {}",l);
		exit(EXIT_FAILURE);
	}

	if( g < 0 ){
		spdlog::error("inappropriate gap < 0, provided : {}",g);
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

	if( traceyvals.size() <= (2*l+g) ){
		spdlog::error("Inappropriate filter values for l: {} and g: {}, 2*l+g:{} exceeds total trace length: {}",l,g,2*l+g,traceyvals.size());
		exit(EXIT_FAILURE);
	}

	if( blen < 1 or blen > traceyvals.size() ){
		spdlog::error("inappropriate baseline < 1 or > trace length : {}, provided : {}",traceyvals.size(),blen);
		exit(EXIT_FAILURE);
	}

	std::vector<double> bltrace(traceyvals);
	double bline = std::accumulate(traceyvals.begin(),traceyvals.begin()+blen,0.0)/static_cast<double>(blen);
	for( auto& v : bltrace ){
		v -= bline;
	}

	std::vector<double> pz(bltrace);
	if( tau > 0.0 ){
		for( size_t ii = 1; ii < pz.size(); ++ii ){
			pz[ii] = pz[ii - 1] + bltrace[ii] - bltrace[ii-1] + bltrace[ii-1]/tau;
		}
	}
	std::vector<double> trap(pz.size(),0.0);	

	boost::circular_buffer<double> f(pz.begin()+l+g,pz.begin()+2*l+g);
	double fsum = std::accumulate(f.begin(),f.end(),0.0);

	boost::circular_buffer<double> b(pz.begin(),pz.begin()+l);
	double bsum = std::accumulate(b.begin(),b.end(),0.0);

	trap[l] = fsum - bsum;
	for( size_t ii = 0; ii < pz.size()-(2*l+g); ++ii ){
		bsum -= b.front();
		b.push_back(pz[ii+l]);
		bsum += b.back();

		fsum -= f.front();
		f.push_back(pz[ii+2*l+g]);
		fsum += f.back();
		trap[ii+l+1] = fsum - bsum;
	}

	std::ofstream out(outputfile);
	for( size_t ii = 0; ii < tracexvals.size(); ++ii ){
		out << tracexvals[ii] << ' ' << traceyvals[ii] << ' ' << bltrace[ii] << ' ' << pz[ii] << ' ' << trap[ii] << std::endl;
	}
	out.close();

	return 0;

}
