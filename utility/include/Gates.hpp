#ifndef __GATES_HPP__
#define __GATES_HPP__

#include <stdexcept>
#include <utility>

template<class T>
class Gate{
	public:
		Gate() : bounds(0.0,0.0){
		}

		Gate(const T& a,const T& b) : bounds(a,b){
			if( a > b ){
				throw std::runtime_error("Gate defined with lowerbound > upperbound");
			}
		}

		const T& GetLowerBound() const{
		       return this->bounds.first;
		}	       

		const T& GetUpperBound() const{
			return this->bounds.second;
		}

		const std::pair<T,T>& GetBounds() const{
			return this->bounds;
		}

		template<class U>
		bool IsWithin(const U& val) const{
			return val >= this->bounds.first and val <= this->bounds.second;
		}

		template<class U>
		bool IsOutside(const U& val) const{
			return val < this->bounds.first or val > this->bounds.second;
		}

		template<class U>
		bool IsBelowLowerBound(const U& val) const{
			return val < this->bounds.first;
		}

		template<class U>
		bool IsAboveLowerBound(const U& val) const{
			return val > this->bounds.first;
		}

		template<class U>
		bool IsBelowUpperBound(const U& val) const{
			return val < this->bounds.second;
		}

		template<class U>
		bool IsAboveUpperBound(const U& val) const{
			return val > this->bounds.second;
		}
	private:
		std::pair<T,T> bounds;
};

#endif
