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

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlinear.h>

#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>

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
	auto j = gsl_multifit_nlinear_jac(work);
	auto cov = gsl_matrix_alloc(fdf->p,fdf->p);
	gsl_multifit_nlinear_covar (j, 0.0, cov);
	auto nfev  = fdf->nevalf;
	auto njev  = fdf->nevaldf;
	auto naev  = fdf->nevalfvv;

	// nfev - number of function evaluations
	// njev - number of Jacobian evaluations
	// naev - number of f_vv evaluations
	//logger::debug("curve fitted after ", niter, " iterations {nfev = ", nfev, "} {njev = ", njev, "} {naev = ", naev, "}");
	spdlog::info("curve fitted after {} iterations [nfev={},njev={},naev={}]",niter,nfev,njev,naev);
	spdlog::info("chisq/ndf : {}/{} -> {}",chisq,dof,chisq/dof);
	for( size_t ii = 0; ii < fdf->p; ++ii ){
		for( size_t jj = 0; jj < fdf->p; ++jj ){
			std::cout << gsl_matrix_get(cov,ii,jj) << ' ';
		}
		std::cout << std::endl;
	}

	gsl_multifit_nlinear_free(work);
	gsl_matrix_free(cov);
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

	// "This selects the Levenberg-Marquardt algorithm."
	fdf_params.trs = gsl_multifit_nlinear_trs_lm;
	// "This selects the Levenberg-Marquardt algorithm with geodesic acceleration."
	//fdf_params.trs = gsl_multifit_nlinear_trs_lmaccel;
	// "This selects dogleg."
	//fdf_params.trs = gsl_multifit_nlinear_trs_dogleg;
	// "This selects double dogleg."
	//fdf_params.trs = gsl_multifit_nlinear_trs_ddogleg;
	// "This selects 2D subspace."
	//fdf_params.trs = gsl_multifit_nlinear_trs_subspace2D;
	// "This selects steihaug-toint."
	//fdf_params.trs = gsl_multifit_nlinear_trs_cgst;
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

// Generic functor
template<typename _Scalar, int NX = Eigen::Dynamic, int NY = Eigen::Dynamic>
struct Functor{
	typedef _Scalar Scalar;
	enum {
		InputsAtCompileTime = NX,
		ValuesAtCompileTime = NY
	};
	typedef Eigen::Matrix<Scalar,InputsAtCompileTime,1> InputType;
	typedef Eigen::Matrix<Scalar,ValuesAtCompileTime,1> ValueType;
	typedef Eigen::Matrix<Scalar,ValuesAtCompileTime,InputsAtCompileTime> JacobianType;

	int m_inputs, m_values;

	Functor() : m_inputs(InputsAtCompileTime), m_values(ValuesAtCompileTime) {}
	Functor(int inputs, int values) : m_inputs(inputs), m_values(values) {}

	int inputs() const { return m_inputs; }
	int values() const { return m_values; }

};

struct trace_fit_functor : Functor<double>{
	int operator()(Eigen::VectorXd &x, Eigen::VectorXd &fvec) const{
		for( const auto& kv : this->boundedvalues ){
			if( x(kv.first) > kv.second.second ){
				x(kv.first) = kv.second.second;
			}else if( x(kv.first) < kv.second.first ){
				x(kv.first) = kv.second.first;
			}else{
			}
		}
		for( size_t ii = 0; ii < xpoints.size(); ++ii ){
			fvec(ii) = this->weights[ii]*(this->ypoints[ii] - ( tracefunc(this->xpoints[ii],x(0),x(1),x(2),x(3),x(4)) ));
		}
		int iter = 0;
		for( const auto& kv : this->fixedvalues ){
			auto offset = (x(kv.first) - kv.second);
			fvec(xpoints.size()+iter) = 1.0*offset*offset;
			++iter;
		}
		return 0;
	}
	//Jacobian is (values,inputs)
	//J(i,j) = df_i/dx_j
	//since x(0) = constant, J(i,0) = 1.0
	//since x(1) = A_p, J(i,1) = 
	//since x(2) = t_0, J(i,2) = 
	//since x(3) = t_r, J(i,3) = 
	//since x(4) = t_f, J(i,4) = 
	//int df(const Eigen::VectorXd& x, Eigen::MatrixXd& J) const{
	//	J(0, 0) = 20 * (x(0) + 3);
	//	J(0, 1) = 2 * (x(1) - 5);
	//	J(1, 0) = x(1);
	//	J(1, 1) = x(0);
	//	return 0;
	//}
	int inputs() const { return 5;}
	int values() const { return this->xpoints.size()+this->fixedvalues.size(); }
	int constraints() const { return this->fixedvalues.size(); }
	std::vector<double> xpoints;
	std::vector<double> ypoints;
	std::vector<double> weights;
	std::map<int,double> fixedvalues;
	std::map<int,std::pair<double,double>> boundedvalues;
};

struct sin_trace_fit_functor : Functor<double>{
	int operator()(Eigen::VectorXd &x, Eigen::VectorXd &fvec) const{
		for( size_t ii = 0; ii < xpoints.size(); ++ii ){
			fvec(ii) = this->weights[ii]*(this->ypoints[ii] - ( sintracefunc(this->xpoints[ii],x(0),x(1),x(2),x(3),x(4),x(5),x(6),x(7)) ));
		}
		int iter = 0;
		for( const auto& kv : this->fixedvalues ){
			auto offset = (x(kv.first) - kv.second);
			fvec(xpoints.size()+iter) = 1.0*offset*offset;
			++iter;
		}
		return 0;
	}
	int inputs() const { return 8;}
	int values() const { return this->xpoints.size()+this->fixedvalues.size(); }
	int constraints() const { return this->fixedvalues.size(); }
	std::vector<double> xpoints;
	std::vector<double> ypoints;
	std::vector<double> weights;
	std::map<int,double> fixedvalues;
};


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
		Eigen::LevenbergMarquardt<Eigen::NumericalDiff<sin_trace_fit_functor>,double> lm(numDiff);
		lm.parameters.maxfev = 2000;
		lm.parameters.xtol = 1.0e-10;

		auto start_time = std::chrono::high_resolution_clock::now();
		int ret = lm.minimize(init_guess);
		auto stop_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double,std::milli> dur = (stop_time - start_time);
		spdlog::info("Eigen niter: {} nfev: {}/{}, time: {} ms \n {}",lm.iter,lm.nfev,lm.njev,dur.count(),init_guess);
		auto f = lm.fvec;
		auto chisq = f.dot(f);
		auto ndf = f.size() - (init_guess.size() + tfit.fixedvalues.size());
		auto j = lm.fjac;
		auto cov = (j.transpose()*j).inverse()*(chisq/ndf);
		spdlog::info("chisq/ndof : {}/{} -> {}",chisq,ndf,chisq/ndf);
		spdlog::info("cov : \n {}",cov);

		std::ofstream out(outputfile);
		for( size_t ii = 0; ii < tracexvals.size(); ++ii ){
			auto x = tracexvals[ii];
			auto y = traceyvals[ii];
			out << x << ' ' 
			    << y << ' ' 
			    << sintracefunc(x,r[0],r[1],r[2],r[3],r[4],r[5],r[6],r[7]) << ' '
			    << sintracefunc(x,init_guess(0),init_guess(1),init_guess(2),init_guess(3),init_guess(4),init_guess(5),init_guess(6),init_guess(7)) 
			    << std::endl;
		}
		out.close();
	}else if( fitfunc.compare("tracefunc") == 0 ){
		auto r = curve_fit(tracefunc, {6580.0,40.0,54.0,1.0,5.0},tracexvals,traceyvals);
		spdlog::info("constant : {} ",r[0]);
		spdlog::info("pulse -> amp : {} delay : {} rise : {} fall : {}",r[1],r[2],r[3],r[4]);

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
		Eigen::LevenbergMarquardt<Eigen::NumericalDiff<trace_fit_functor>,double> lm(numDiff);
		//Eigen::LevenbergMarquardt<trace_fit_functor> lm(tfit);
		lm.parameters.maxfev = 2000;
		lm.parameters.xtol = 1.0e-10;

		auto start_time = std::chrono::high_resolution_clock::now();
		//int ret = lm.minimizeInit(init_guess);
		//ret = lm.minimizeOneStep(init_guess);
		int ret = lm.minimize(init_guess);
		auto stop_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double,std::milli> dur = (stop_time - start_time);
		spdlog::info("Eigen niter: {} nfev: {}/{}, time: {} ms ",lm.iter,lm.nfev,lm.njev,dur.count());
		spdlog::info("Eigen constant : {} ",init_guess(0));
		spdlog::info("Eigen pulse -> amp : {} delay : {} rise : {} fall : {}",init_guess(1),init_guess(2),init_guess(3),init_guess(4));
		auto f = lm.fvec;
		auto chisq = f.dot(f);
		auto ndf = f.size() - (init_guess.size() + tfit.fixedvalues.size());
		Eigen::internal::covar(lm.fjac,lm.permutation.indices());
		auto cov = lm.fjac.topLeftCorner(init_guess.size(),init_guess.size());
		auto diag = cov.diagonal().array().sqrt().inverse().matrix().asDiagonal();
		spdlog::info("Eigen chisq/ndof : {}/{} -> {}",chisq,ndf,chisq/ndf);
		spdlog::info("Eigen cov : \n {}",cov);
		spdlog::info("Eigen cor : \n {}",diag*cov*diag);

		std::ofstream out(outputfile);
		for( size_t ii = 0; ii < tracexvals.size(); ++ii ){
			auto x = tracexvals[ii];
			auto y = traceyvals[ii];
			out << x << ' ' 
			    << y << ' ' 
			    << tracefunc(x,r[0],r[1],r[2],r[3],r[4]) << ' ' 
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
