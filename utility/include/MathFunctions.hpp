#ifndef __MATH_FUNCTIONS_HPP__
#define __MATH_FUNCTIONS_HPP__ 

#include <vector>
#include <numeric>
#include <cmath>

namespace Math{
	double CalcAngleBetweenTwoPoints(double,double,double,double);

	//integrate a vector between two bounds
	template<class U,class V>
	void Integrate(const size_t& start,const size_t& end,const std::vector<V>& arr,U& val){
		val = std::accumulate(arr.begin()+start,arr.begin()+end,0);
	}
	
	template<class U,class V>
	void Average(const size_t& start, const size_t& end,const std::vector<V>& arr,U& avg){
		Integrate(start,end,arr,avg);
		avg /= static_cast<U>(end - start);
	}
	
	template<class U,class V>
	void StdDev(const size_t& start,const size_t& end,const U& avg, const std::vector<V>& arr,U& dev){
		if( (end == (start+1)) or ((end-1) == start) ){
			dev = 0.0;
			return;
		}
		size_t sz = end - start;
		auto devfunc = [&avg,&sz](U accumulator,const V& val){
			return accumulator + ((val - avg)*(val - avg) / static_cast<U>(sz - 1));
		};

		dev = std::sqrt(std::accumulate(arr.begin()+start,arr.begin()+end,0.0,devfunc));
	}

}

#endif
