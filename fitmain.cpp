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
	const size_t max_iter = 200;
	const double xtol = 1.0e-8;
	const double gtol = 1.0e-8;
	const double ftol = 1.0e-8;

	auto *work = gsl_multifit_nlinear_alloc(T, params, fdf->n, fdf->p);
	int info;

	// initialize solver
	gsl_multifit_nlinear_init(initial_params, fdf, work);
	//iterate until convergence
	gsl_multifit_nlinear_driver(max_iter, xtol, gtol, ftol, nullptr, nullptr, &info, work);

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

double gaussian(double x, double a, double b, double c){
    const double z = (x - b) / c;
    return a * std::exp(-0.5 * z * z);
}

double gaussian_d(int j,double x, double a, double b, double c){
	const double z = (x - b) / c;
	const double e = std::exp(-0.5 * z * z);
	switch(j){
		case 0:
			return -e;
		case 1:
			return  -(a / c) * e * z;
		case 2:
			return -(a / c) * e * z * z;
		default:
			return 0.0;
	}
}

double gaussian_aa(double x,double a,double b, double c){
	return 0.0;
}

double gaussian_ab(double x,double a,double b, double c){
	const double z = (x - b) / c;
	const double e = std::exp(-0.5 * z * z);
	return -z * e / c;
}

double gaussian_ac(double x,double a,double b, double c){
	const double z = (x - b) / c;
	const double e = std::exp(-0.5 * z * z);
	return -z * z * e / c;
}

double gaussian_ba(double x,double a,double b, double c){
	return gaussian_ab(x,a,b,c);
}

double gaussian_bb(double x,double a,double b, double c){
	const double z = (x - b) / c;
	const double e = std::exp(-0.5 * z * z);
	return a * e / (c * c) * (1.0 - z*z);
}

double gaussian_bc(double x,double a,double b, double c){
	const double z = (x - b) / c;
	const double e = std::exp(-0.5 * z * z);
	return a * z * e / (c * c) * (2.0 - z*z);
}

double gaussian_ca(double x,double a,double b, double c){
	return gaussian_ac(x,a,b,c);
}

double gaussian_cb(double x,double a,double b, double c){
	return gaussian_bc(x,a,b,c);
}

double gaussian_cc(double x,double a,double b, double c){
	const double z = (x - b) / c;
	const double e = std::exp(-0.5 * z * z);
	return a * z * z * e / (c * c) * (3.0 - z*z);
}

double gaussian_vv(int j, int k,double x, double a, double b, double c){
	if( j == 0 ){
		switch( k ){
			case 0:
				return gaussian_aa(x,a,b,c);
			case 1:
				return gaussian_ab(x,a,b,c);
			case 2:
				return gaussian_ac(x,a,b,c);
			default: 
				return 0.0;
		}
	}else if( j == 1 ){
		switch( k ){
			case 0:
				return gaussian_ba(x,a,b,c);
			case 1:
				return gaussian_bb(x,a,b,c);
			case 2:
				return gaussian_bc(x,a,b,c);
			default: 
				return 0.0;
		}
	}else if( j == 2 ){
		const double z = (x - b) / c;
		const double e = std::exp(-0.5 * z * z);
		switch( k ){
			case 0:
				return gaussian_ca(x,a,b,c);
			case 1:
				return gaussian_cb(x,a,b,c);
			case 2:
				return gaussian_cc(x,a,b,c);
			default:
				return 0.0;
		}
	}else{
		return 0.0;
	}
}


template <typename Container>
auto linspace(typename Container::value_type a, typename Container::value_type b, size_t n){
    assert(b > a);
    assert(n > 1);

    Container res(n);
    const auto step = (b - a) / (n - 1);
    auto val = a;
    for(auto& e: res)
    {
        e = val;
        val += step;
    }
    return res;
}

int main(int argc, char *argv[]) {
	std::string inputfile = "test.out";

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("inputfile,i",boost::program_options::value<std::string>(&inputfile)->default_value("trace.txt"),"number of iterations for the fill command")
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
	std::vector<std::pair<double,double>> vals;
	std::ifstream input(inputfile);
	while( input >> currx >> curry ){
		vals.push_back({currx,curry});
	}
	input.close();

	for( const auto& v : vals ){
		spdlog::info("read from {} : {} {}",inputfile,v.first,v.second);
	}

	auto device = std::random_device();
	auto gen    = std::mt19937(device());

	auto xs = linspace<std::vector<double>>(0.0, 1.0, 300);
	auto ys = std::vector<double>(xs.size());

	double a = 5.0, b = 0.4, c = 0.15;

	for(size_t i = 0; i < xs.size(); i++)
	{
		auto y =  gaussian(xs[i], a, b, c);
		auto dist  = std::normal_distribution(0.0, 0.1 * y);
		ys[i] = y + dist(gen);
	}

	spdlog::info("vals : {} {} {}",a,b,c);
	auto start_time = std::chrono::high_resolution_clock::now();
	auto r1 = curve_fit(gaussian, {1.0, 0.0, 1.0}, xs, ys);
	auto stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double,std::milli> dur_r1 = (stop_time - start_time);
	spdlog::info("pure function evale : time {} ms, result : {} {} {}",dur_r1.count(),r1[0],r1[1],r1[2]);

	start_time = std::chrono::high_resolution_clock::now();
	auto r2 = curve_fit(gaussian, gaussian_d, {1.0, 0.0, 1.0}, xs, ys);
	stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double,std::milli> dur_r2 = (stop_time - start_time);
	spdlog::info("function with derivative : time {} ms, result : {} {} {}",dur_r2.count(),r2[0],r2[1],r2[2]);

	start_time = std::chrono::high_resolution_clock::now();
	auto r3 = curve_fit(gaussian, gaussian_d, gaussian_vv, {1.0, 0.0, 1.0}, xs, ys);
	stop_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double,std::milli> dur_r3 = (stop_time - start_time);
	spdlog::info("function with derivative and geodesic : time {} ms, result : {} {} {}",dur_r3.count(),r3[0],r3[1],r3[2]);


	return 0;

}
