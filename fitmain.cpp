#include <iostream>
#include <chrono>
#include <random>
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

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlinear.h>


template <class R, class... ARGS>
struct function_ripper {
	static constexpr size_t n_args = sizeof...(ARGS);
};

/**
 * This function returns the number of parameters of a given function.This
 * overload is to be used specialy with lambdas.
 */
template <class R, class... ARGS>
auto constexpr n_params(std::function<R (ARGS...)> ) {
	return function_ripper<R, ARGS...>();
}

/**
 * This function returns the number of parameters of a given function. 
 */
template <class R, class... ARGS>
auto constexpr n_params(R (ARGS...) ) 
{
	return function_ripper<R, ARGS...>();
}

template <typename F, size_t... Is>
auto gen_tuple_impl(F func, std::index_sequence<Is...> ) {
	return std::make_tuple(func(Is)...);
}

template <size_t N, typename F>
auto gen_tuple(F func) {
	return gen_tuple_impl(func, std::make_index_sequence<N>{} );
}

template<typename... Args>
struct fit_data;

template<typename C1>
struct fit_data<C1>{
	const std::vector<double>& t;
	const std::vector<double>& y;
	// the actual function to be fitted
	C1 f;
};

template<typename C1,typename C2>
struct fit_data<C1,C2>{
	const std::vector<double>& t;
	const std::vector<double>& y;
	// the actual function to be fitted
	C1 f;
	C2 df;
};

template<typename C1,typename C2,typename C3>
struct fit_data<C1,C2,C3>{
	const std::vector<double>& t;
	const std::vector<double>& y;
	// the actual function to be fitted
	C1 f;
	C2 df;
	C3 fvv;
};


template<typename FitData, int n_params>
int internal_f(const gsl_vector* x, void* params, gsl_vector *f){
	auto* d  = static_cast<FitData*>(params);
	// Convert the parameter values from gsl_vector (in x) into std::tuple
	auto init_args = [x](int index)
	{
		return gsl_vector_get(x, index);
	};
	auto parameters = gen_tuple<n_params>(init_args);

	// Calculate the error for each...
	for (size_t i = 0; i < d->t.size(); ++i)
	{
		double ti = d->t[i];
		double yi = d->y[i];
		auto func = [ti, &d](auto ...xs)
		{
			// call the actual function to be fitted
			return d->f(ti, xs...);
		};
		auto y = std::apply(func, parameters);
		gsl_vector_set(f, i, yi - y);
	}
	return GSL_SUCCESS;
}

template<typename FitData, int n_params>
int internal_df(const gsl_vector* x, void* params, gsl_matrix* J){
	auto* d  = static_cast<FitData*>(params);
	// Convert the parameter values from gsl_vector (in x) into std::tuple
	auto init_args = [x](int index)
	{
		return gsl_vector_get(x, index);
	};
	auto parameters = gen_tuple<n_params>(init_args);
	for (size_t i = 0; i < d->t.size(); ++i)
	{
		double ti = d->t[i];
		double yi = d->y[i];
		for( size_t j = 0; j < n_params; ++j )
		{
			auto func = [ti, &d, j](auto ...xs)
			{
				// call the actual function to be fitted
				return d->df(j,ti, xs...);
			};
			auto y = std::apply(func, parameters);
			gsl_matrix_set(J, i, j, y);
		}
	}

	return GSL_SUCCESS;
}

template<typename FitData, int n_params>
int internal_fvv(const gsl_vector* x, const gsl_vector* v, void* params, gsl_vector* fvv){
	auto* d  = static_cast<FitData*>(params);
	// Convert the parameter values from gsl_vector (in x) into std::tuple
	auto init_args_x = [x](int index)
	{
		return gsl_vector_get(x, index);
	};
	auto parameters = gen_tuple<n_params>(init_args_x);
	for (size_t i = 0; i < d->t.size(); ++i)
	{
		double ti = d->t[i];
		double yi = d->y[i];
		double sum = 0.0;
		for( size_t j = 0; j < n_params; ++j )
		{
			for( size_t k = j; k < n_params; ++k ){
				auto func = [ti, &d, j, k](auto ...xs)
				{
					// call the actual function to be fitted
					return d->fvv(j,k,ti, xs...);
				};
				auto y = std::apply(func, parameters);
				if( k == j ){
					auto vk = gsl_vector_get(v,k);
					sum += vk*vk*y;
				}else{
					auto vk = gsl_vector_get(v,k);
					auto vj = gsl_vector_get(v,j);
					sum += 2.0*vk*vj*y;
				}
			}
		}
		gsl_vector_set(fvv, i, sum);
	}
	return GSL_SUCCESS;
}

using func_f_type   = int (*) (const gsl_vector*, void*, gsl_vector*);
using func_df_type  = int (*) (const gsl_vector*, void*, gsl_matrix*);
using func_fvv_type = int (*) (const gsl_vector*, const gsl_vector *, void *, gsl_vector *);

gsl_vector* internal_make_gsl_vector_ptr(const std::vector<double>& vec){
	auto* result = gsl_vector_alloc(vec.size());
	int i = 0;
	for(const auto e: vec)
	{
		gsl_vector_set(result, i, e);
		i++;
	}
	return result;
}

std::vector<double> internal_solve_system(gsl_vector* initial_params, gsl_multifit_nlinear_fdf *fdf,gsl_multifit_nlinear_parameters *params){
	// This specifies a trust region method
	const gsl_multifit_nlinear_type *T = gsl_multifit_nlinear_trust;
	const size_t max_iter = 1000;
	const double xtol = 1.0e-6;
	const double gtol = 1.0e-6;
	const double ftol = 1.0e-6;

	auto *work = gsl_multifit_nlinear_alloc(T, params, fdf->n, fdf->p);
	gsl_vector * f = gsl_multifit_nlinear_residual(work);
	double chisq;
	double dof = fdf->n - fdf->p;
	int info;

	// initialize solver
	gsl_multifit_nlinear_init(initial_params, fdf, work);
	//iterate until convergence
	auto start_time = std::chrono::high_resolution_clock::now();
	gsl_multifit_nlinear_driver(max_iter, xtol, gtol, ftol, nullptr, nullptr, &info, work);
	auto stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double,std::milli> dur = (stop_time - start_time);
	spdlog::info("trace fit : time {} ms",dur.count());
	gsl_blas_ddot(f, f, &chisq);

	// result will be stored here
	gsl_vector * y    = gsl_multifit_nlinear_position(work);
	auto result = std::vector<double>(initial_params->size);

	for(int i = 0; i < result.size(); i++)
	{
		result[i] = gsl_vector_get(y, i);
	}

	auto niter = gsl_multifit_nlinear_niter(work);
	auto nfev  = fdf->nevalf;
	auto njev  = fdf->nevaldf;
	auto naev  = fdf->nevalfvv;

	// nfev - number of function evaluations
	// njev - number of Jacobian evaluations
	// naev - number of f_vv evaluations
	//logger::debug("curve fitted after ", niter, " iterations {nfev = ", nfev, "} {njev = ", njev, "} {naev = ", naev, "}");
	spdlog::info("curve fitted after {} iterations [nfev={},njev={},naev={}]",niter,nfev,njev,naev);
	spdlog::info("chisq/ndf : {}/{} -> {}",chisq,dof,chisq/dof);

	gsl_multifit_nlinear_free(work);
	gsl_vector_free(initial_params);
	return result;
}

template<typename C1,typename C2,typename C3>
std::vector<double> curve_fit_impl(func_f_type f, func_df_type df, func_fvv_type fvv, gsl_vector* initial_params, fit_data<C1,C2,C3>& fd){
	assert(fd.t.size() == fd.y.size());

	auto fdf = gsl_multifit_nlinear_fdf();
	auto fdf_params = gsl_multifit_nlinear_default_parameters();

	fdf.f   = f;
	fdf.df  = df;
	fdf.fvv = fvv;
	fdf.n   = fd.t.size();
	fdf.p   = initial_params->size;
	fdf.params = &fd;

	// "This selects the Levenberg-Marquardt algorithm with geodesic acceleration."
	fdf_params.trs = gsl_multifit_nlinear_trs_lmaccel;
	return internal_solve_system(initial_params, &fdf, &fdf_params);
}

template<typename C1,typename C2>
std::vector<double> curve_fit_impl(func_f_type f, func_df_type df, func_fvv_type fvv, gsl_vector* initial_params, fit_data<C1,C2>& fd){
	assert(fd.t.size() == fd.y.size());

	auto fdf = gsl_multifit_nlinear_fdf();
	auto fdf_params = gsl_multifit_nlinear_default_parameters();

	fdf.f   = f;
	fdf.df  = df;
	fdf.fvv = fvv;
	fdf.n   = fd.t.size();
	fdf.p   = initial_params->size;
	fdf.params = &fd;

	// "This selects the Levenberg-Marquardt algorithm with geodesic acceleration."
	fdf_params.trs = gsl_multifit_nlinear_trs_lmaccel;
	return internal_solve_system(initial_params, &fdf, &fdf_params);
}

template<typename C1>
std::vector<double> curve_fit_impl(func_f_type f, func_df_type df, func_fvv_type fvv, gsl_vector* initial_params, fit_data<C1>& fd){
	assert(fd.t.size() == fd.y.size());

	auto fdf = gsl_multifit_nlinear_fdf();
	auto fdf_params = gsl_multifit_nlinear_default_parameters();

	fdf.f   = f;
	fdf.df  = df;
	fdf.fvv = fvv;
	fdf.n   = fd.t.size();
	fdf.p   = initial_params->size;
	fdf.params = &fd;

	// "This selects the Levenberg-Marquardt algorithm with geodesic acceleration."
	fdf_params.trs = gsl_multifit_nlinear_trs_lmaccel;
	return internal_solve_system(initial_params, &fdf, &fdf_params);
}



/**
 * Performs a non-linear least-squares fit.
 * 
 * @param f a function of type double (double x, double c1, double c2, ..., double cn)
 * where  c1, ..., cn are the coefficients to be fitted.
 * @param initial_params intial guess for the parameters. The size of the array must to 
 * be equal to the number of coefficients to be fitted.
 * @param x the idependent data.
 * @param y the dependent data, must to have the same size as x.
 * @return std::vector<double> with the computed coefficients
 */
template<typename CallableFunction,typename CallableDerivative,typename CallableGeodesic>
std::vector<double> curve_fit(CallableFunction f, CallableDerivative df, CallableGeodesic fvv, const std::vector<double>& initial_params, const std::vector<double>& x, const std::vector<double>& y){
	// We can't pass lambdas without convert to std::function.
	constexpr auto n = decltype(n_params(std::function(f)))::n_args - 1;
	assert(initial_params.size() == n);

	auto params = internal_make_gsl_vector_ptr(initial_params);
	auto fd = fit_data<CallableFunction,CallableDerivative,CallableGeodesic>{x, y, f,df, fvv};
	return curve_fit_impl(internal_f<decltype(fd), n>, internal_df<decltype(fd),n>, internal_fvv<decltype(fd),n>, params, fd);
}

template<typename CallableFunction,typename CallableDerivative>
std::vector<double> curve_fit(CallableFunction f, CallableDerivative df, const std::vector<double>& initial_params, const std::vector<double>& x, const std::vector<double>& y){
	// We can't pass lambdas without convert to std::function.
	constexpr auto n = decltype(n_params(std::function(f)))::n_args - 1;
	assert(initial_params.size() == n);

	auto params = internal_make_gsl_vector_ptr(initial_params);
	auto fd = fit_data<CallableFunction,CallableDerivative>{x, y, f,df};
	return curve_fit_impl(internal_f<decltype(fd), n>, internal_df<decltype(fd),n>, nullptr, params, fd);
}

template<typename CallableFunction>
std::vector<double> curve_fit(CallableFunction f, const std::vector<double>& initial_params, const std::vector<double>& x, const std::vector<double>& y){
	// We can't pass lambdas without convert to std::function.
	constexpr auto n = decltype(n_params(std::function(f)))::n_args - 1;
	assert(initial_params.size() == n);

	auto params = internal_make_gsl_vector_ptr(initial_params);
	auto fd = fit_data<CallableFunction>{x, y, f};
	return curve_fit_impl(internal_f<decltype(fd), n>, nullptr, nullptr, params, fd);
}

double Sin(double t,double a,double p,double f){
	return a*std::sin(f*(t+p));
}

double TraceFunc(double t,double a,double d,double r,double f){
	return a*((1.0/(std::exp(-(t-d)/r)+1.0))*(1.0/(std::exp((t-d)/f)+1.0)));
}

double sintracefunc(double t,double c,double sa,double sp,double sf,double pa,double pd,double pr,double pf){
	double SinVal = Sin(t,sa,sp,sf);
	double PulseVal = TraceFunc(t,pa,pd,pr,pf);
	return c + SinVal + PulseVal;
}

double tracefunc(double t,double c,double pa,double pd,double pr,double pf){
	double PulseVal = TraceFunc(t,pa,pd,pr,pf);
	return c + PulseVal;
}

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
		auto r = curve_fit(sintracefunc, {6580.0,20.0,0.0,0.5,40.0,54.0,1.0,5.0},tracexvals,traceyvals);
		spdlog::info("constant : {} ",r[0]);
		spdlog::info("sin -> amp : {} phase : {} freq : {}",r[1],r[2],r[3]);
		spdlog::info("pulse -> amp : {} delay : {} rise : {} fall : {}",r[4],r[5],r[6],r[7]);

		std::ofstream out(outputfile);
		for( size_t ii = 0; ii < tracexvals.size(); ++ii ){
			auto x = tracexvals[ii];
			auto y = traceyvals[ii];
			out << x << ' ' << y << ' ' << sintracefunc(x,r[0],r[1],r[2],r[3],r[4],r[5],r[6],r[7]) << std::endl;
		}
		out.close();
	}else if( fitfunc.compare("tracefunc") == 0 ){
		auto r = curve_fit(tracefunc, {6580.0,40.0,54.0,1.0,5.0},tracexvals,traceyvals);
		spdlog::info("constant : {} ",r[0]);
		spdlog::info("pulse -> amp : {} delay : {} rise : {} fall : {}",r[1],r[2],r[3],r[4]);

		std::ofstream out(outputfile);
		for( size_t ii = 0; ii < tracexvals.size(); ++ii ){
			auto x = tracexvals[ii];
			auto y = traceyvals[ii];
			out << x << ' ' << y << ' ' << tracefunc(x,r[0],r[1],r[2],r[3],r[4]) << std::endl;
		}
		out.close();
	}else{
		spdlog::error("unknown fit function {}",fitfunc);
		spdlog::info(cmdline_options);
		exit(EXIT_FAILURE);
	}

	return 0;

}
