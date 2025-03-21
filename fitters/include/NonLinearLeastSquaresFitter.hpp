#ifndef  __NONLINEAR_LEAST_SQUARES_FITTER_HPP__
#define __NONLINEAR_LEAST_SQUARES_FITTER_HPP__

#include <map>
#include <vector>

#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>

double TraceFunc(double,double,double,double,double);
double Sin(double,double,double,double);
double sintracefunc(double,double,double,double,double,double,double,double,double);
double tracefunc(double,double,double,double,double,double);


template<typename _Scalar, int NX = Eigen::Dynamic, int NY = Eigen::Dynamic>
struct BaseFunctor{
	typedef _Scalar Scalar;
	enum {
		InputsAtCompileTime = NX,
		ValuesAtCompileTime = NY
	};
	typedef Eigen::Matrix<Scalar,InputsAtCompileTime,1> InputType;
	typedef Eigen::Matrix<Scalar,ValuesAtCompileTime,1> ValueType;
	typedef Eigen::Matrix<Scalar,ValuesAtCompileTime,InputsAtCompileTime> JacobianType;

	int m_inputs, m_values;

	BaseFunctor() : m_inputs(InputsAtCompileTime), m_values(ValuesAtCompileTime) {}
	BaseFunctor(int inputs, int values) : m_inputs(inputs), m_values(values) {}

	int inputs() const { return m_inputs; }
	int values() const { return m_values; }

	std::vector<double> xpoints;
	std::vector<double> ypoints;
	std::vector<double> weights;
	std::map<int,double> fixedvalues;
	std::map<int,std::pair<double,double>> boundedvalues;
};

struct trace_fit_functor : BaseFunctor<double>{
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
};

struct sin_trace_fit_functor : BaseFunctor<double>{
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
};


//apple doesn't support fucking concepts
//so we can't make sure that we have a derivative
//template<class T>
//concept bool IsFunctor = requires(T f){f() -> int;}
//concept bool HasDerivative = requires(T f){f.df() -> int;}
//template<HasDerivative<T>>
template<class T>
class NonLinearLeastSquaresFitter{
	public:
		NonLinearLeastSquaresFitter(T& functor) : f(functor), LMFit(functor), initsize(0), chi2(0.0), ndf(0.0), CalcCov(false),CalcCor(false){
		}

		~NonLinearLeastSquaresFitter() = default;
		void FixParameter(int idx,double val){
			this->f->fixedvalues[idx] = val;
		}

		void BoundParameter(int idx,double low,double high){
			this->f->boundedvalues[idx] = {low,high};
		}

		void BoundParameter(int idx,std::pair<double,double>& bnds){
			this->f->boundedvalues[idx] = bnds;
		}

		void Minimize(Eigen::VectorXd& init){
			[[maybe_unused]] auto ret = this->LMFit.minimize(init);
			this->initsize = init.size();
			this->chi2 = this->LMFit.fvec.dot(this->LMFit.fvec);
			this->ndf = this->LMFit.fvec.size() - (init.size() + this->f.fixedvalues.size());
			this->InternalCalcCov();
			this->InternalCalcCor();
		}

		void MaxEval(int limit){
			this->LMFit.parameters.maxfev = limit; 
		}

		void XTol(double tol){
			this->LMFit.parameters.xtol = tol;
		}

		void GTol(double tol){
			this->LMFit.parameters.gtol = tol;
		}
		
		void FTol(double tol){
			this->LMFit.parameters.ftol = tol;
		}

		int Iter() const{
			return this->LMFit.iter;
		}

		int NFev() const{
			return this->LMFit.nfev;
		}

		int NJev() const{
			return this->LMFit.njev;
		}

		double Chi2() const{
			return this->chi2;
		}

		double NDF() const{
			return this->ndf;
		}

		Eigen::LevenbergMarquardt<T,double>::JacobianType Covariance(){
			if( not this->CalcCov ){
				this->InternalCalcCov();
			}
			return this->cov;
		}

		Eigen::LevenbergMarquardt<T,double>::JacobianType Correlation(){
			if( not this->CalcCov ){
				this->InternalCalcCov();
			}
			if( not this->CalcCor ){
				this->InternalCalcCor();
			}
			return this->cor;
		}

		void Reset(){
			this->LMFit.iter = 0;
			this->LMFit.nfev = 0;
			this->LMFit.njev = 0;
		}

	private:
		T f;
		Eigen::LevenbergMarquardt<T,double> LMFit;
		int initsize;
		double chi2;
		double ndf;
		bool CalcCov;
		bool CalcCor;
		Eigen::LevenbergMarquardt<T,double>::JacobianType cov;
		Eigen::LevenbergMarquardt<T,double>::JacobianType cor;

		void InternalCalcCov(){
			Eigen::internal::covar(this->LMFit.fjac,this->LMFit.permutation.indices());
			this->cov = this->LMFit.fjac.topLeftCorner(this->initsize,this->initsize);
			this->CalcCov = true;
		}
		
		void InternalCalcCor(){
			auto diag = this->cov.diagonal().array().sqrt().inverse().matrix().asDiagonal();
			this->cor = diag*this->cov*diag;
			this->CalcCor = true;
		}
};

#endif
