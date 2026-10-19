#ifndef __BASE_FUNCTOR_HPP__
#define __BASE_FUNCTOR_HPP__

#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>

template<typename _Scalar, int NX = Eigen::Dynamic, int NY = Eigen::Dynamic>
struct BaseFunctor {
	typedef _Scalar Scalar;
	enum {
		InputsAtCompileTime = NX,
		ValuesAtCompileTime = NY
	};
	typedef Eigen::Matrix<Scalar, InputsAtCompileTime, 1> InputType;
	typedef Eigen::Matrix<Scalar, ValuesAtCompileTime, 1> ValueType;
	typedef Eigen::Matrix<Scalar, ValuesAtCompileTime, InputsAtCompileTime> JacobianType;

	int m_inputs, m_values;

	BaseFunctor()
		: m_inputs(InputsAtCompileTime)
		, m_values(ValuesAtCompileTime) {
	}
	BaseFunctor(int inputs, int values)
		: m_inputs(inputs)
		, m_values(values) {
	}

	int inputs() const {
		return m_inputs;
	}
	int values() const {
		return m_values;
	}

	std::vector<double> xpoints;
	std::vector<double> ypoints;
	std::vector<double> weights;
	std::map<int, double> fixedvalues;
	std::map<int, std::pair<double, double>> boundedvalues;
};

#endif
