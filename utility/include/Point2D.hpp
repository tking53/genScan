#ifndef __POINT2D_HPP__
#define __POINT2D_HPP__

#include <cmath>
#include <utility>
template<class T>
class Point2D {
public:
	Point2D(const T& xval, const T& yval)
		: x(xval)
		, y(yval) {
	}

	Point2D()
		: x(0.0)
		, y(0.0) {
	}

	~Point2D() {
	}

	Point2D(const Point2D& rhs)
		: x(rhs.x)
		, y(rhs.y) {
	}

	Point2D(Point2D&& rhs)
		: x(std::move(rhs.x))
		, y(std::move(rhs.y)) {
	}

	Point2D& operator=(const Point2D& other) {
		if (this != &other) {
			this->x = other.x;
			this->y = other.y;
		}
		return *this;
	}

	Point2D& operator=(Point2D&& other) {
		if (this != &other) {
			this->x = std::move(other.x);
			this->y = std::move(other.y);
		}
		return *this;
	}

	template<typename OStream>
	friend OStream& operator<<(OStream& os, const Point2D& p) {
		os << p.x << " " << p.y;
		return os;
	}

	const T& X() const {
		return this->x;
	}

	const T& Y() const {
		return this->y;
	}

	T Dist(const Point2D& other) const {
		return std::sqrt(this->Dist2(other));
	}

	T Dist2(const Point2D& other) const {
		T xdiff = this->x - other.x;
		T ydiff = this->y - other.y;
		return xdiff * xdiff + ydiff * ydiff;
	}

private:
	T x;
	T y;
};

#endif
