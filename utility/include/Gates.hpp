#ifndef __GATES_HPP__
#define __GATES_HPP__

#include <stdexcept>
#include <utility>

template<class T>
class Gate {
public:
	Gate()
		: bounds(0.0, 0.0) {
	}

	Gate(const T& a, const T& b)
		: bounds(a, b) {
		if (a > b) {
			throw std::runtime_error("Gate defined with lowerbound > upperbound");
		}
	}

	const T& GetLowerBound() const {
		return this->bounds.first;
	}

	const T& GetUpperBound() const {
		return this->bounds.second;
	}

	const std::pair<T, T>& GetBounds() const {
		return this->bounds;
	}

	template<class U>
	bool IsWithin(const U& val) const {
		return val >= this->bounds.first and val < this->bounds.second;
	}

	template<class U>
	bool IsOutside(const U& val) const {
		return val < this->bounds.first or val >= this->bounds.second;
	}

	template<class U>
	bool IsBelowLowerBound(const U& val) const {
		return val < this->bounds.first;
	}

	template<class U>
	bool IsAboveLowerBound(const U& val) const {
		return val > this->bounds.first;
	}

	template<class U>
	bool IsBelowUpperBound(const U& val) const {
		return val < this->bounds.second;
	}

	template<class U>
	bool IsAboveUpperBound(const U& val) const {
		return val > this->bounds.second;
	}

private:
	std::pair<T, T> bounds;
};

template<class T>
class BoxGate {
public:
	BoxGate()
		: XBounds(0.0, 0.0)
		, YBounds(0.0, 0.0) {
	}

	BoxGate(const T& ax, const T& bx, const T& ay, const T& by)
		: XBounds(ax, bx)
		, YBounds(ay, by) {
	}

	const T& GetLowerXBound() const {
		return this->XBounds.GetLowerBound();
	}

	const T& GetUpperXBound() const {
		return this->XBounds.GetUpperBound();
	}

	const T& GetLowerYBound() const {
		return this->YBounds.GetLowerBound();
	}

	const T& GetUpperYBound() const {
		return this->YBounds.GetUpperBound();
	}

	const Gate<T>& GetXBounds() const {
		return this->XBounds;
	}

	const Gate<T>& GetYBounds() const {
		return this->YBounds;
	}

	template<class U, class V>
	bool IsWithin(const U& xval, const V& yval) const {
		return this->XBounds.IsWithin(xval) and this->YBounds.IsWithin(yval);
	}

	template<class U, class V>
	bool IsOutside(const U& xval, const V& yval) const {
		return this->XBounds.IsOutside(xval) or this->YBounds.IsOutside(yval);
	}

	template<class U>
	bool IsBelowLowerXBound(const U& val) const {
		return this->XBounds.IsBelowLowerBound(val);
	}

	template<class U>
	bool IsAboveLowerXBound(const U& val) const {
		return this->XBounds.IsAboveLowerBound(val);
	}

	template<class U>
	bool IsBelowUpperXBound(const U& val) const {
		return this->XBounds.IsBelowUpperBound(val);
	}

	template<class U>
	bool IsAboveUpperXBound(const U& val) const {
		return this->XBounds.IsAboveUpperBound(val);
	}

	template<class U>
	bool IsBelowLowerYBound(const U& val) const {
		return this->YBounds.IsBelowLowerBound(val);
	}

	template<class U>
	bool IsAboveLowerYBound(const U& val) const {
		return this->YBounds.IsAboveLowerBound(val);
	}

	template<class U>
	bool IsBelowUpperYBound(const U& val) const {
		return this->YBounds.IsBelowUpperBound(val);
	}

	template<class U>
	bool IsAboveUpperYBound(const U& val) const {
		return this->YBounds.IsAboveUpperBound(val);
	}

private:
	Gate<T> XBounds;
	Gate<T> YBounds;
};

#endif
