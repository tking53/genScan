#ifndef __TRACE_HELPER_HPP__
#define __TRACE_HELPER_HPP__

#include <numeric>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include <initializer_list>
#include <utility>
#include <tuple>

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

		TraceHelper(const TraceHelper& other) : data(other.data), 
							data_baselinesub(other.data_baselinesub),
							TQDCSums(other.TQDCSums), 
							PreTriggerBaselineInfo(other.PreTriggerBaselineInfo),
							PostTriggerBaselineInfo(other.PostTriggerBaselineInfo),
							MaxInfo(other.MaxInfo)
	        {
		}

		TraceHelper& operator=(const TraceHelper& other){
			if( this != &other ){
				data.clear();
				data = other.data;
				data_baselinesub.clear();
				data_baselinesub = other.data_baselinesub;
				TQDCSums = other.TQDCSums;
				PreTriggerBaselineInfo = other.PreTriggerBaselineInfo;
				PostTriggerBaselineInfo = other.PostTriggerBaselineInfo;
				MaxInfo = other.MaxInfo;
			}
			return *this;
		}

		TraceHelper(TraceHelper&& other) noexcept : data(std::move(other.data)),
							    data_baselinesub(std::move(other.data_baselinesub)),
							    TQDCSums(std::move(other.TQDCSums)),
							    PreTriggerBaselineInfo(std::move(other.PreTriggerBaselineInfo)),
							    PostTriggerBaselineInfo(std::move(other.PostTriggerBaselineInfo)),
							    MaxInfo(std::move(other.MaxInfo))
		{
		}

		TraceHelper& operator=(TraceHelper&& other) noexcept{
			if( this != &other ){
				data.clear();
				data = std::move(other.data);
				data_baselinesub.clear();
				data_baselinesub = std::move(other.data_baselinesub);
				TQDCSums = std::move(other.TQDCSums);
				PreTriggerBaselineInfo = std::move(other.PreTriggerBaselineInfo);
				PostTriggerBaselineInfo = std::move(other.PostTriggerBaselineInfo);
				MaxInfo = std::move(other.MaxInfo);
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

			PreTriggerBaselineInfo = CalcAVGSTDDev(PreTrigRegion,data);
			PostTriggerBaselineInfo = CalcAVGSTDDev(PostTrigRegion,data);
			for( size_t ii = 1; ii < QDCBounds.size(); ++ii ){
				TQDCSums.push_back(Sum(QDCBounds.at(ii-1),QDCBounds.at(ii),data));
			}

			auto maxval = std::max_element(data.begin(),data.end());
			MaxInfo = std::make_pair(std::distance(data.begin(),maxval),*maxval);

			data_baselinesub = std::vector<U>(data.size(),0.0);
			size_t idx = 0;
			for( const auto& val : data ){
				data_baselinesub[idx] = static_cast<U>(val) - PreTriggerBaselineInfo.first;
				++idx;
			}
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
		
		U IntegrateRawTrace(const std::pair<size_t,size_t>& bounds){
			return this->Sum(bounds.first,bounds.second,data);
		}

		U IntegrateRawTrace(const size_t& start, const size_t& end){
			return static_cast<U>(this->Sum(start,end,data));
		}
		
		U AverageRawTrace(const std::pair<size_t,size_t>& bounds){
			return this->Average(bounds,data);
		}

		U AverageRawTrace(const size_t& start,const size_t& end){
		       return this->Average(start,end,data);
		}
		
		U IntegrateBaselineSubtractedTrace(const std::pair<size_t,size_t>& bounds){
			return this->Sum(bounds.first,bounds.second,data_baselinesub);
		}

		U IntegrateBaselineSubtractedTrace(const size_t& start, const size_t& end){
			return static_cast<U>(this->Sum(start,end,data_baselinesub));
		}
		
		U AverageBaselineSubtractedTrace(const std::pair<size_t,size_t>& bounds){
			return this->Average(bounds,data_baselinesub);
		}

		U AverageBaselineSubtractedTrace(const size_t& start,const size_t& end){
		       return this->Average(start,end,data_baselinesub);
		}

		std::tuple<U,U,U> CalcPSD(const size_t& start, const size_t& mid, const size_t& end){
			U pre = this->IntegrateBaselineSubtractedTrace(start,mid);
			U post = this->IntegrateBaselineSubtractedTrace(mid,end);
			return std::make_tuple(pre,post,post/(pre+post));
		}

	private:
		std::vector<T> data;
		std::vector<U> data_baselinesub;
		std::vector<T> TQDCSums;
		std::pair<U,U> PreTriggerBaselineInfo;
		std::pair<U,U> PostTriggerBaselineInfo;
		std::pair<size_t,T> MaxInfo;

		template<class V>
		T Sum(const size_t& start,const size_t& end,const std::vector<V>& arr){
			return std::accumulate(arr.begin()+start,arr.begin()+end,0);
		}

		template<class V>
		U Average(const std::pair<size_t,size_t>& bounds,const std::vector<V>& arr){
			return Average(bounds.first,bounds.second,arr);
		}

		template<class V>
		U Average(const size_t& start,const size_t& end,const std::vector<V>& arr){
			return std::accumulate(arr.begin()+start,arr.begin()+end,0.0)/static_cast<U>(end - start);
		}

		template<class V>
		U STDDev(const std::pair<size_t,size_t>& bounds,const U& avg,const std::vector<V>& arr){
			return STDDev(bounds.first,bounds.second,avg,arr);
		}

		template<class V>
		U STDDev(const size_t& start,const size_t& end,const U& avg,const std::vector<V>& arr){
			if( (end == (start+1)) or ((end-1) == start) )
				return 0.0;
			size_t sz = end - start;
			auto dev = [&avg,&sz](U accumulator,const T& val){
				return accumulator + ((val - avg)*(val - avg) / static_cast<U>(sz - 1));
			};

			return std::sqrt(std::accumulate(arr.begin()+start,arr.begin()+end,0.0,dev));
		}

		template<class V>
		std::pair<U,U> CalcAVGSTDDev(const std::pair<size_t,size_t>& bounds,const std::vector<V>& arr){
			U avg = Average(bounds,arr);
			return std::make_pair(avg,STDDev(bounds,avg,arr));
		}
};	

#endif
