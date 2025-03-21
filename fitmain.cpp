#include <gsl/gsl_cblas.h>
#include <gsl/gsl_matrix_double.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <tuple>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>

#include "NonLinearLeastSquaresFitter.hpp"

int main(int argc, char *argv[]) {
	std::string inputfile = "trace.txt";
	std::string outputfile = "fit.txt";
	double xmin = 30.0;
	double xmax = 80.0;
	std::string fitfunc = "tracefunc";

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("inputfile,i",boost::program_options::value<std::string>(&inputfile)->default_value("trace.txt"),"file to read the trace data in formatted as x y_i")
		("outputfile,o",boost::program_options::value<std::string>(&outputfile)->default_value("fit.txt"),"file to write the trace fit in formatted as x y_i y_f")
		("fitfunc,f",boost::program_options::value<std::string>(&fitfunc)->default_value("tracefunc"),"function to use for trace fitting [tracefunc,sintracefunc]")
		("xmin,l",boost::program_options::value<double>(&xmin)->default_value(0.0),"lower fit bound")
		("xmax,u",boost::program_options::value<double>(&xmax)->default_value(0.0),"upper fit bound")
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
		if( xmin >= xmax ){
			throw std::runtime_error("xmin >= xmax");
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
		if( currx >= xmin && currx <= xmax ){
			tracexvals.push_back(currx);
			traceyvals.push_back(curry);
		}
	}
	input.close();

	if( fitfunc.compare("sintracefunc") == 0 ){
		std::vector<Eigen::VectorXd> guesses(1000,Eigen::VectorXd(8));
		for( auto& g : guesses ){
			g(0) = 6580.0;
			g(1) = 20.0;
			g(2) = 0.0;
			g(3) = 0.5;
			g(4) = 40.0;
			g(5) = 54.0;
			g(6) = 1.0;
			g(7) = 5.0;
		}
		Eigen::VectorXd init_guess(8);
		init_guess(0) = 6580.0;
		init_guess(1) = 20.0;
		init_guess(2) = 0.0;
		init_guess(3) = 0.5;
		init_guess(4) = 40.0;
		init_guess(5) = 54.0;
		init_guess(6) = 1.0;
		init_guess(7) = 5.0;

		sin_trace_fit_functor tfit;
		tfit.xpoints = tracexvals;
		tfit.ypoints = traceyvals;
		tfit.weights = std::vector<double>(tracexvals.size(),1.0);
		Eigen::NumericalDiff<sin_trace_fit_functor> numDiff(tfit);
		NonLinearLeastSquaresFitter<Eigen::NumericalDiff<sin_trace_fit_functor>> lm(numDiff);
		lm.MaxEval(2000);
		lm.XTol(1.0e-10);

		auto start_time = std::chrono::high_resolution_clock::now();
		for( int ii = 0; ii < 1000; ++ii ){
			lm.Minimize(guesses[ii]);
		}
		auto stop_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double,std::milli> dur = (stop_time - start_time);
		lm.Minimize(init_guess);
		spdlog::info("Eigen niter: {} nfev: {}/{}, time: {} ms ",lm.Iter(),lm.NFev(),lm.NJev(),dur.count());
		spdlog::info("Eigen constant : {} ",init_guess(0));
		spdlog::info("Eigen pulse -> amp : {} delay : {} rise : {} fall : {}",init_guess(1),init_guess(2),init_guess(3),init_guess(4));
		spdlog::info("Eigen sin -> amp : {} phase : {} freq : {}",init_guess(5),init_guess(6),init_guess(7));
		auto chisq = lm.Chi2();
		auto ndf = lm.NDF();
		spdlog::info("Eigen chisq/ndof : {}/{} -> {}",chisq,ndf,chisq/ndf);
		spdlog::info("Eigen cov : \n {}",lm.Covariance());
		//spdlog::info("Eigen cor : \n {}",lm.Correlation());

		std::ofstream out(outputfile);
		for( size_t ii = 0; ii < tracexvals.size(); ++ii ){
			auto x = tracexvals[ii];
			auto y = traceyvals[ii];
			out << x << ' ' 
			    << y << ' ' 
			    << sintracefunc(x,init_guess(0),init_guess(1),init_guess(2),init_guess(3),init_guess(4),init_guess(5),init_guess(6),init_guess(7)) 
			    << std::endl;
		}
		out.close();
	}else if( fitfunc.compare("tracefunc") == 0 ){
		Eigen::VectorXd init_guess(5);
		init_guess(0) = 6580.0;
		init_guess(1) = 40.0;
		init_guess(2) = 54.0;
		init_guess(3) = 1.0;
		init_guess(4) = 5.0;

		trace_fit_functor tfit;
		tfit.xpoints = tracexvals;
		tfit.ypoints = traceyvals;
		tfit.weights = std::vector<double>(tracexvals.size(),1.0);
		//tfit.boundedvalues[0] = {4000.0,5000.0};
		Eigen::NumericalDiff<trace_fit_functor> numDiff(tfit);
		NonLinearLeastSquaresFitter<Eigen::NumericalDiff<trace_fit_functor>> lm(numDiff);
		lm.MaxEval(2000);
		lm.XTol(1.0e-10);

		auto start_time = std::chrono::high_resolution_clock::now();
		for( int ii = 0; ii < 1000; ++ii ){
			init_guess(0) = 6580.0;
			init_guess(1) = 40.0;
			init_guess(2) = 54.0;
			init_guess(3) = 1.0;
			init_guess(4) = 5.0;
			lm.Minimize(init_guess);
		}
		auto stop_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double,std::milli> dur = (stop_time - start_time);
		spdlog::info("Eigen niter: {} nfev: {}/{}, time: {} ms ",lm.Iter(),lm.NFev(),lm.NJev(),dur.count());
		spdlog::info("Eigen constant : {} ",init_guess(0));
		spdlog::info("Eigen pulse -> amp : {} delay : {} rise : {} fall : {}",init_guess(1),init_guess(2),init_guess(3),init_guess(4));
		auto chisq = lm.Chi2();
		auto ndf = lm.NDF();
		spdlog::info("Eigen chisq/ndof : {}/{} -> {}",chisq,ndf,chisq/ndf);
		spdlog::info("Eigen cov : \n {}",lm.Covariance());
		//spdlog::info("Eigen cor : \n {}",lm.Correlation());

		std::ofstream out(outputfile);
		for( size_t ii = 0; ii < tracexvals.size(); ++ii ){
			auto x = tracexvals[ii];
			auto y = traceyvals[ii];
			out << x << ' ' 
			    << y << ' ' 
			    << tracefunc(x,init_guess(0),init_guess(1),init_guess(2),init_guess(3),init_guess(4)) 
			    << std::endl;
		}
		out.close();
	}else{
		spdlog::error("unknown fit function {}",fitfunc);
		spdlog::info(cmdline_options);
		exit(EXIT_FAILURE);
	}

	return 0;

}
