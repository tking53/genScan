#ifndef __KMEANS_HPP__
#define __KMEANS_HPP__

#include <vector>
#include <random>
#include <iostream>

#include "Point2D.hpp"

template<class T>
class KMeans {
public:
	enum FINISHCODE {
		MAXITER,
		TOLERANCE
	};

	KMeans(const std::vector<Point2D<T>>& data, int kvalue)
		: kmeans(kvalue)
		, points(data) {
		// determine initial set of K Means, randomly assign each point to a centroid
		this->centroids = std::vector<Point2DSet>(this->kmeans, Point2DSet());
		std::random_device rd;
		std::mt19937_64 randGen(rd());
		std::uniform_int_distribution<size_t> randidx(0, this->points.size());
		;
		for (size_t ii = 0; ii < this->kmeans; ++ii) {
			this->centroids[ii].mean = this->points[randidx(randGen)];
		}
	}

	double SingleStep() {
		std::vector<Point2DSet> temp(this->kmeans, Point2DSet());
		for (const auto& p : points) {
			auto idx = this->FindClosestCentroid(p);
			temp[idx].InsertPoint(p);
		}

		for (auto& c : temp) {
			c.UpdateMean();
		}

		double tol = 0.0;
		for (size_t ii = 0; ii < this->kmeans; ++ii) {
			tol += this->centroids[ii].mean.Dist(temp[ii].mean);
		}
		this->centroids = temp;

		return tol;
	}

	FINISHCODE Run(int maxiter, double tol) {
		int iter = 0;
		double delta = 0.0;
		FINISHCODE finish = FINISHCODE::MAXITER;
		do {
			delta = this->SingleStep();
			// std::cout << iter << " " << delta << std::endl;
			++iter;
			if (delta < tol) {
				finish = FINISHCODE::TOLERANCE;
			}
		} while ((iter < maxiter) and (delta > tol));
		return finish;
	}

	size_t FindClosestCentroid(const Point2D<T>& p) {
		size_t idx = 0;
		auto dist = p.Dist(this->centroids[0].mean);
		for (size_t ii = 1; ii < this->centroids.size(); ++ii) {
			auto currdist = p.Dist(this->centroids[ii].mean);
			if (currdist < dist) {
				dist = currdist;
				idx = ii;
			}
		}
		return idx;
	}

	std::vector<Point2D<T>> GetCentroids() const {
		std::vector<Point2D<T>> cvals;
		for (const auto& p : this->centroids) {
			cvals.push_back(p.mean);
		}
		return cvals;
	}

	struct Point2DSet {
		Point2D<T> mean;

		double xavg = 0.0;
		double yavg = 0.0;
		size_t npoints = 0;

		void UpdateMean() {
			if (npoints > 0) {
				xavg /= static_cast<double>(npoints);
				yavg /= static_cast<double>(npoints);
			}
			mean = Point2D<T>(xavg, yavg);
		}

		void InsertPoint(const Point2D<T>& p) {
			xavg += p.X();
			yavg += p.Y();
			++npoints;
		}
	};

private:
	int kmeans;
	std::vector<Point2D<T>> points;
	std::vector<Point2DSet> centroids;
};

#endif
