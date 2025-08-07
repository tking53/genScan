#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <vector>
#include <fstream>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/program_options.hpp>
#include <boost/circular_buffer.hpp>

template<class T>
struct Kernel{
	T weight;
	boost::circular_buffer<T> values;
	T sum;
	std::vector<T> datavalues;
	Kernel(const std::vector<T>& v,size_t b,size_t sz,const T& w=1.0) : weight(w){
		values = boost::circular_buffer<T>(v.begin()+b,v.begin()+b+sz);
		for( auto& d : values ){
			d *= weight;
		}
		sum = std::accumulate(values.begin(),values.end(),0.0);
		datavalues.push_back(sum);

		for( size_t ii = b+sz; ii < v.size(); ++ii ){
			sum -= values.front();
			values.push_back(v.at(ii)*weight);
			sum += values.back();
			datavalues.push_back(sum);
		}	
	}
	T& operator[](size_t idx){
		return datavalues[idx];
	}

	const T& operator[](size_t idx) const{
		return datavalues.at(idx);
	}

	size_t size() const{
		return datavalues.size();
	}
};

template<class T>
struct SNIP{
	std::vector<T> curriter;
	std::vector<T> previter;

	SNIP(const std::vector<T>& data,size_t iter){
		previter = std::vector<T>(data.begin(),data.end());
		for( auto& p : previter ){
			p = std::log(std::log(std::sqrt(p+1)+1)+1);
		}
		curriter = std::vector<T>(previter.begin(),previter.end());
		for( size_t pp = 0; pp < iter; ++pp ){
			for( size_t ii = pp; ii < (data.size()-pp); ++ii ){
				curriter[ii] = std::min<T>(previter[ii],(previter[ii-pp]+previter[ii+pp])/2.0);
			}
			previter = curriter;
		}
		for( auto& d : curriter){
			auto dd = std::exp(std::exp(d)-1.0)-1.0;
			d = dd*dd - 1.0;
		}
	}

	T& operator[](size_t idx){
		return curriter[idx];
	}

	const T& operator[](size_t idx) const{
		return curriter.at(idx);
	}


};

//template<class T>
//struct PeakLocator{
//	std::vector<T> datavalues;
//	boost::circular_buffer<T> predistance;
//	boost::circular_buffer<T> postdistance;
//	PeakLocator(const std::vector<T>& v,size_t k){
//	}
//
//	T& operator[](size_t idx){
//		return datavalues[idx];
//	}
//
//	const T& operator[](size_t idx) const{
//		return datavalues.at(idx);
//	}
//
//	size_t size() const{
//		return datavalues.size();
//	}
//};

//std::vector<int> smoothedZScore(std::vector<float> input){
//	//lag 5 for the smoothing functions
//	int lag = 20;
//	//3.5 standard deviations for signal
//	float threshold = 3.5;
//	//between 0 and 1, where 1 is normal influence, 0.5 is half
//	float influence = .5;
//
//	if (input.size() <= lag + 2)
//	{
//		std::vector<int> emptyVec;
//		return emptyVec;
//	}
//
//	auto mean = [](const std::vector<float>& data){
//		return std::accumulate(data.begin(),data.end(),0.0)/static_cast<float>(data.size());
//	};
//
//	auto stdDev = [=](const std::vector<float>& data){
//		auto avg = mean(data);
//		size_t sz = data.size();
//		auto devfunc = [&avg,&sz](float accumulator,const float& val){
//			return accumulator + ((val - avg)*(val - avg))/static_cast<float>(sz - 1);
//		};
//		return std::sqrt(std::accumulate(data.begin(),data.end(),0.0,devfunc));
//	};
//
//	//Initialise variables
//	std::vector<int> signals(input.size(), 0.0);
//	std::vector<float> filteredY(input.size(), 0.0);
//	std::vector<float> avgFilter(input.size(), 0.0);
//	std::vector<float> stdFilter(input.size(), 0.0);
//	std::vector<float> subVecStart(input.begin(), input.begin() + lag);
//	avgFilter[lag] = mean(subVecStart);
//	stdFilter[lag] = stdDev(subVecStart);
//
//	for (size_t i = lag + 1; i < input.size(); i++)
//	{
//		if (std::abs(input[i] - avgFilter[i - 1]) > threshold * stdFilter[i - 1])
//		{
//			if (input[i] > avgFilter[i - 1])
//			{
//				signals[i] = 1; //# Positive signal
//			}
//			else
//			{
//				signals[i] = -1; //# Negative signal
//			}
//			//Make influence lower
//			filteredY[i] = influence* input[i] + (1 - influence) * filteredY[i - 1];
//		}
//		else
//		{
//			signals[i] = 0; //# No signal
//			filteredY[i] = input[i];
//		}
//		//Adjust the filters
//		std::vector<float> subVec(filteredY.begin() + i - lag, filteredY.begin() + i);
//		avgFilter[i] = mean(subVec);
//		stdFilter[i] = stdDev(subVec);
//	}
//	return signals;
//}

int main(int argc, char *argv[]) {
	std::string inputfile;
	std::string outputfile;
	int width;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message")
		("inputfile,i",boost::program_options::value<std::string>(&inputfile)->default_value("histo.txt"),"file to read the histo data in formatted as x y_i")
		("outputfile,o",boost::program_options::value<std::string>(&outputfile)->default_value("peaks.txt"),"file to write the histo trapezoid in formatted as x y_i t_i")
		("length,l",boost::program_options::value<int>(&width)->default_value(10),"width of filter in bins")
		;

	boost::program_options::positional_options_description pos;

	try{
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(pos).run(), vm);
		notify(vm);
		if( vm.count("help") or argc <= 2 ){
			spdlog::info(cmdline_options);
			exit(EXIT_SUCCESS);
		}

		float currx,curry;
		std::vector<float> histoxvals;
		std::vector<float> histoyvals;
		std::ifstream input(inputfile);
		while( input >> currx >> curry ){
			histoxvals.push_back(currx);
			histoyvals.push_back(curry);
		}
		input.close();

		if( histoyvals.size() <= (3*width) ){
			spdlog::error("Inappropriate width choice 3*{} exceeds total histo length: {}",width,histoyvals.size());
			exit(EXIT_FAILURE);
		}

		//Kernel<float> preregion(histoyvals,0,width,1.0);
		//Kernel<float> midregion(histoyvals,width,width,2.0);
		//Kernel<float> postregion(histoyvals,2*width,width,1.0);

		//std::vector<float> filter(width+(width-1)/2,0.0);
		//for( size_t ii = 0; ii < std::min({preregion.datavalues.size(),midregion.datavalues.size(),postregion.datavalues.size()}); ++ii ){
		//	filter.push_back(midregion[ii] - (preregion[ii]+postregion[ii]));
		//}
		//for( size_t ii = 0; ii < (width+width/2); ++ii ){
		//	filter.push_back(0.0);
		//}

		SNIP<float> filter(histoyvals,width);
		std::ofstream out(outputfile);
		for( size_t ii = 0; ii < histoxvals.size(); ++ii ){
			out << histoxvals[ii] << ' ' << histoyvals[ii] << ' ' << filter[ii] << ' ' << histoyvals[ii] - filter[ii] << std::endl;
		}
		out.close();
	}catch( std::exception& e){
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}   
	return 0;

}
