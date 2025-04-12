#ifndef __TRAPEZOID_FILTER_HPP__
#define __TRAPEZOID_FILTER_HPP__

#include <vector>
#include <numeric>
#include <boost/circular_buffer.hpp>

template<class T,class U>
struct TrapezoidFilter{
	int l;
	int g;
	int blen;
	T tau;	
	std::vector<T> bltrace;
	std::vector<T> pz;
	std::vector<T> trap;

	boost::circular_buffer<T> f;
	boost::circular_buffer<T> b;

	TrapezoidFilter(int ll,int gg,int bb,T tt) : l(ll), g(gg), blen(bb), tau(tt){
	}
	~TrapezoidFilter() = default;
	TrapezoidFilter(const TrapezoidFilter&) = default;
	TrapezoidFilter(TrapezoidFilter&&) = default;
	TrapezoidFilter& operator=(const TrapezoidFilter&) = default;
	TrapezoidFilter& operator=(TrapezoidFilter&&) = default;

	double RunFilter(const std::vector<U>& traceyvals){
		bltrace = std::vector<T>(traceyvals.begin(),traceyvals.end());
		T bline = std::accumulate(bltrace.begin(),bltrace.begin()+blen,0.0)/static_cast<T>(blen);
		for( auto& v : bltrace ){
			v -= bline;
		}

		pz = std::vector<T>(bltrace);
		if( tau > 0.0 ){
			for( size_t ii = 1; ii < pz.size(); ++ii ){
				pz[ii] = pz[ii - 1] + bltrace[ii] - bltrace[ii-1] + bltrace[ii-1]/tau;
			}
		}
		trap = std::vector<T>(pz.size(),0.0);	

		f = boost::circular_buffer<T>(pz.begin()+l+g,pz.begin()+2*l+g);
		T fsum = std::accumulate(f.begin(),f.end(),0.0);

		b = boost::circular_buffer<T>(pz.begin(),pz.begin()+l);
		T bsum = std::accumulate(b.begin(),b.end(),0.0);

		trap[0] = fsum - bsum;
		for( size_t ii = 0; ii < pz.size()-(2*l+g); ++ii ){
			bsum -= b.front();
			b.push_back(pz[ii+l]);
			bsum += b.back();

			fsum -= f.front();
			f.push_back(pz[ii+2*l+g]);
			fsum += f.back();
			trap[ii+1] = fsum - bsum;
		}
		return trap[2*l+g-1];
	}
};


#endif
