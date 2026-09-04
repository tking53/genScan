#ifndef __NONLINEAR_LEAST_SQUARES_FITTER_HPP__
#define __NONLINEAR_LEAST_SQUARES_FITTER_HPP__

#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>

// #include "BaseFunctor.hpp"

// apple doesn't support fucking concepts
// so we can't make sure that we have a derivative
// template<class T>
// concept bool IsFunctor = requires(T f){f() -> int;}
// concept bool HasDerivative = requires(T f){f.df() -> int;}
// template<HasDerivative<T>>
template<class T>
class NonLinearLeastSquaresFitter {
public:
	NonLinearLeastSquaresFitter(T& functor)
		: f(functor)
		, LMFit(functor)
		, initsize(0)
		, chi2(0.0)
		, ndf(0.0)
		, CalcCov(false)
		, CalcCor(false) {
	}

	~NonLinearLeastSquaresFitter() = default;
	void FixParameter(int idx, double val) {
		this->f->fixedvalues[idx] = val;
	}

	void BoundParameter(int idx, double low, double high) {
		this->f->boundedvalues[idx] = {low, high};
	}

	void BoundParameter(int idx, std::pair<double, double>& bnds) {
		this->f->boundedvalues[idx] = bnds;
	}

	void Minimize(Eigen::VectorXd& init) {
		[[maybe_unused]] auto ret = this->LMFit.minimize(init);
		this->initsize = init.size();
		this->chi2 = this->LMFit.fvec.dot(this->LMFit.fvec);
		this->ndf = this->LMFit.fvec.size() - (init.size() + this->f.fixedvalues.size());
		this->InternalCalcCov();
		this->InternalCalcCor();
	}

	void MaxEval(int limit) {
		this->LMFit.parameters.maxfev = limit;
	}

	void XTol(double tol) {
		this->LMFit.parameters.xtol = tol;
	}

	void GTol(double tol) {
		this->LMFit.parameters.gtol = tol;
	}

	void FTol(double tol) {
		this->LMFit.parameters.ftol = tol;
	}

	int Iter() const {
		return this->LMFit.iter;
	}

	int NFev() const {
		return this->LMFit.nfev;
	}

	int NJev() const {
		return this->LMFit.njev;
	}

	double Chi2() const {
		return this->chi2;
	}

	double NDF() const {
		return this->ndf;
	}

	typename Eigen::LevenbergMarquardt<T, double>::JacobianType Covariance() {
		if (not this->CalcCov) {
			this->InternalCalcCov();
		}
		return this->cov;
	}

	typename Eigen::LevenbergMarquardt<T, double>::JacobianType Correlation() {
		if (not this->CalcCov) {
			this->InternalCalcCov();
		}
		if (not this->CalcCor) {
			this->InternalCalcCor();
		}
		return this->cor;
	}

	void Reset() {
		this->LMFit.iter = 0;
		this->LMFit.nfev = 0;
		this->LMFit.njev = 0;
	}

private:
	T f;
	Eigen::LevenbergMarquardt<T, double> LMFit;
	int initsize;
	double chi2;
	double ndf;
	bool CalcCov;
	bool CalcCor;
	typename Eigen::LevenbergMarquardt<T, double>::JacobianType cov;
	typename Eigen::LevenbergMarquardt<T, double>::JacobianType cor;

	void InternalCalcCov() {
		Eigen::internal::covar(this->LMFit.fjac, this->LMFit.permutation.indices());
		this->cov = this->LMFit.fjac.topLeftCorner(this->initsize, this->initsize);
		this->CalcCov = true;
	}

	void InternalCalcCor() {
		auto diag = this->cov.diagonal().array().sqrt().inverse().matrix().asDiagonal();
		this->cor = diag * this->cov * diag;
		this->CalcCor = true;
	}
};

#endif
