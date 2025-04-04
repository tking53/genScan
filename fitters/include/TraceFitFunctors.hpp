#ifndef __TRACE_FIT_FUNCTORS_HPP__
#define __TRACE_FIT_FUNCTORS_HPP__

#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>

#include "BaseFunctor.hpp"
#include "PulseFitFunctions.hpp"

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
			fvec(ii) = this->weights[ii]*(this->ypoints[ii] - ( PulseFit::tracefunc(this->xpoints[ii],x(0),x(1),x(2),x(3),x(4)) ));
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
			fvec(ii) = this->weights[ii]*(this->ypoints[ii] - ( PulseFit::sintracefunc(this->xpoints[ii],x(0),x(1),x(2),x(3),x(4),x(5),x(6),x(7)) ));
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



#endif
