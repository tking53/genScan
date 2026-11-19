#ifndef __INTEGRAL_FILTER_HPP__
#define __INTEGRAL_FILTER_HPP__

#include <vector>
#include <numeric>
#include <boost/circular_buffer.hpp>

template<class U>
struct IntegrationFilter {
	int bll;
	int blu;
	int il;
	int iu;

	IntegrationFilter(int a, int b, int c, int d)
		: bll(a)
		, blu(b)
		, il(c)
		, iu(d) {
	}
	~IntegrationFilter() = default;
	IntegrationFilter(const IntegrationFilter&) = default;
	IntegrationFilter(IntegrationFilter&&) = default;
	IntegrationFilter& operator=(const IntegrationFilter&) = default;
	IntegrationFilter& operator=(IntegrationFilter&&) = default;

	double RunFilter(const std::vector<U>& trace) {
		double bl = std::accumulate(trace.begin() + bll, trace.begin() + blu, 0.0) / static_cast<double>(blu - bll);
		return std::accumulate(trace.begin() + il, trace.begin() + iu, 0.0) - bl * (iu - il);
	}
};

#endif
