#ifndef __TRACE_HELPER_HPP__
#define __TRACE_HELPER_HPP__

#include <numeric>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include <initializer_list>
#include <utility>

template<class T,class U>
class TraceHelper final{
	public:
		TraceHelper() : data(std::vector<T>()){
		}

		TraceHelper(const unsigned int len) : data(std::vector<T>(len)){
		}

		TraceHelper(const std::vector<T>& val) : data(val){
		}

		TraceHelper(std::initializer_list<T> val) : data(val){
		}

		TraceHelper(const TraceHelper& other) : data(other.data){
			TQDCSums = other.TQDCSums;
			PreTriggerBaselineInfo = other.PreTriggerBaselineInfo;
			PostTriggerBaselineInfo = other.PostTriggerBaselineInfo;
			MaxInfo = other.MaxInfo;
		}

		TraceHelper& operator=(const TraceHelper& other){
			if( this != &other ){
				data.clear();
				data = other.data;
				TQDCSums = other.TQDCSums;
				PreTriggerBaselineInfo = other.PreTriggerBaselineInfo;
				PostTriggerBaselineInfo = other.PostTriggerBaselineInfo;
				MaxInfo = other.MaxInfo;
			}
			return *this;
		}

		TraceHelper(TraceHelper&& other) noexcept : data(std::move(other.data)){
			TQDCSums = other.TQDCSums;
			PreTriggerBaselineInfo = other.PreTriggerBaselineInfo;
			PostTriggerBaselineInfo = other.PostTriggerBaselineInfo;
			MaxInfo = other.MaxInfo;
		}

		TraceHelper& operator=(TraceHelper&& other){
			if( this != &other ){
				data.clear();
				data = std::move(other.data);
				TQDCSums = other.TQDCSums;
				PreTriggerBaselineInfo = other.PreTriggerBaselineInfo;
				PostTriggerBaselineInfo = other.PostTriggerBaselineInfo;
				MaxInfo = other.MaxInfo;
			}
			return *this;
		}
		
		const std::vector<T>& GetData() const{
			return data;
		}

		std::vector<T>& GetRawData(){
			return data;
		}


		size_t GetSize() const{
			return data.size();
		}

		void AnalyzeWaveform(const std::pair<size_t,size_t>& PreTrigRegion,const std::pair<size_t,size_t>& PostTrigRegion,const std::vector<size_t>& QDCBounds){
			if( PreTrigRegion.first >= PreTrigRegion.second ){
				throw "TraceHelper::AnalyzeWaveform(const std::pair<size_t,size_t>& PreTrigRegion,const std::pair<size_t,size_t>& PostTrigRegion,const std::vector<size_t>& QDCBounds) PreTrigRegion.first > PreTrigRegion.second";
			}

			if( PreTrigRegion.first > data.size() ){
				throw "TraceHelper::AnalyzeWaveform(const std::pair<size_t,size_t>& PreTrigRegion,const std::pair<size_t,size_t>& PostTrigRegion,const std::vector<size_t>& QDCBounds) PreTrigRegion.first >= data.size()";
			}

			if( PreTrigRegion.second > data.size() ){
				throw "TraceHelper::AnalyzeWaveform(const std::pair<size_t,size_t>& PreTrigRegion,const std::pair<size_t,size_t>& PostTrigRegion,const std::vector<size_t>& QDCBounds) PreTrigRegion.second >= data.size()";
			}


			if( PostTrigRegion.first >= PostTrigRegion.second ){
				throw "TraceHelper::AnalyzeWaveform(const std::pair<size_t,size_t>& PostTrigRegion,const std::pair<size_t,size_t>& PostTrigRegion,const std::vector<size_t>& QDCBounds) PostTrigRegion.first > PostTrigRegion.second";
			}

			if( PostTrigRegion.first > data.size() ){
				throw "TraceHelper::AnalyzeWaveform(const std::pair<size_t,size_t>& PostTrigRegion,const std::pair<size_t,size_t>& PostTrigRegion,const std::vector<size_t>& QDCBounds) PostTrigRegion.first >= data.size()";
			}

			if( PostTrigRegion.second > data.size() ){
				throw "TraceHelper::AnalyzeWaveform(const std::pair<size_t,size_t>& PostTrigRegion,const std::pair<size_t,size_t>& PostTrigRegion,const std::vector<size_t>& QDCBounds) PostTrigRegion.second >= data.size()";
			}


			for( size_t ii = 0; ii < QDCBounds.size(); ++ii ){
				if( QDCBounds.at(ii) > data.size() ){
					std::string mess = "TraceHelper::AnalyzeWaveform(const std::pair<size_t,size_t>& PostTrigRegion,const std::pair<size_t,size_t>& PostTrigRegion,const std::vector<size_t>& QDCBounds) QDCBounds["+std::to_string(ii)+"] >= data.size()";
					throw mess;
				}
			}

			for( size_t ii = 1; ii < QDCBounds.size(); ++ii ){
				if( QDCBounds.at(ii) <= QDCBounds.at(ii-1) ){
					std::string mess = "TraceHelper::AnalyzeWaveform(const std::pair<size_t,size_t>& PostTrigRegion,const std::pair<size_t,size_t>& PostTrigRegion,const std::vector<size_t>& QDCBounds) QDCBounds["+std::to_string(ii)+"] <= QDCBounds["+std::to_string(ii-1)+"]";
					throw mess;
				}
			}

			PreTriggerBaselineInfo = CalcAVGSTDDev(PreTrigRegion);
			PostTriggerBaselineInfo = CalcAVGSTDDev(PostTrigRegion);
			for( size_t ii = 1; ii < QDCBounds.size(); ++ii ){
				TQDCSums.push_back(Sum(QDCBounds.at(ii-1),QDCBounds.at(ii)));
			}

			auto maxval = std::max_element(data.begin(),data.end());
			MaxInfo = std::make_pair(std::distance(data.begin(),maxval),*maxval);
		}

		U GetPreTriggerBaseline() const{
			return PreTriggerBaselineInfo.first;
		}

		U GetPreTriggerBaselineSTDDev() const{
			return PreTriggerBaselineInfo.second;
		}

		U GetPostTriggerBaseline() const{
			return PostTriggerBaselineInfo.first;
		}

		U GetPostTriggerBaselineSTDDev() const{
			return PostTriggerBaselineInfo.second;
		}

		std::vector<T> GetTQDCSums() const{
			return TQDCSums;
		}

		size_t GetMaxPos() const{
			return MaxInfo.first;
		}

		T GetMaxVal() const{
			return MaxInfo.second;
		}

		std::pair<size_t,T> GetMaxInfo() const{
			return MaxInfo;
		}

		std::pair<U,U> GetPreTriggerBaselineInfo() const{
			return PreTriggerBaselineInfo;
		}

		std::pair<U,U> GetPostTriggerBaselineInfo() const{
			return PostTriggerBaselineInfo;
		}

	private:
		std::vector<T> data;
		std::vector<T> TQDCSums;
		std::pair<U,U> PreTriggerBaselineInfo;
		std::pair<U,U> PostTriggerBaselineInfo;
		std::pair<size_t,T> MaxInfo;

		T Sum(const size_t& start,const size_t& end){
			return std::accumulate(data.begin()+start,data.begin()+end,0);
		}

		U Average(const std::pair<size_t,size_t>& bounds){
			return Average(bounds.first,bounds.second);
		}

		U Average(const size_t& start,const size_t& end){
			return std::accumulate(data.begin()+start,data.begin()+end,0.0)/static_cast<U>(end - start);
		}

		U STDDev(const std::pair<size_t,size_t>& bounds,const U& avg){
			return STDDev(bounds.first,bounds.second,avg);
		}

		U STDDev(const size_t& start,const size_t& end,const U& avg){
			if( (end == (start+1)) or ((end-1) == start) )
				return 0.0;
			size_t sz = end - start;
			auto dev = [&avg,&sz](U accumulator,const T& val){
				return accumulator + ((val - avg)*(val - avg) / static_cast<U>(sz - 1));
			};

			return std::sqrt(std::accumulate(data.begin()+start,data.begin()+end,0.0,dev));
		}

		std::pair<U,U> CalcAVGSTDDev(const std::pair<size_t,size_t>& bounds){
			U avg = Average(bounds);
			return std::make_pair(avg,STDDev(bounds,avg));
		}
};	

#endif
