#ifndef __TRACE_HELPER_HPP__
#define __TRACE_HELPER_HPP__

#include <algorithm>
#include <vector>
#include <string>
#include <initializer_list>
#include <utility>
#include <tuple>
#include <iostream>

#include "MathFunctions.hpp"

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
							MaxInfo(other.MaxInfo),
							FixedPSD(other.FixedPSD),
							FractionalPSD(other.FractionalPSD),
							BaselineSubMax(other.BaselineSubMax)
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
				FixedPSD = other.FixedPSD;
				FractionalPSD = other.FractionalPSD;
				BaselineSubMax = other.BaselineSubMax;
			}
			return *this;
		}

		TraceHelper(TraceHelper&& other) noexcept : data(std::move(other.data)),
							    data_baselinesub(std::move(other.data_baselinesub)),
							    TQDCSums(std::move(other.TQDCSums)),
							    PreTriggerBaselineInfo(std::move(other.PreTriggerBaselineInfo)),
							    PostTriggerBaselineInfo(std::move(other.PostTriggerBaselineInfo)),
							    MaxInfo(std::move(other.MaxInfo)),
							    FixedPSD(std::move(other.FixedPSD)),
							    FractionalPSD(std::move(other.FractionalPSD)),
							    BaselineSubMax(other.BaselineSubMax)
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
				FixedPSD = std::move(other.FixedPSD);
				FractionalPSD = std::move(other.FractionalPSD);
				BaselineSubMax = other.BaselineSubMax;
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

			Math::Average(PreTrigRegion.first,PreTrigRegion.second,this->data,this->PreTriggerBaselineInfo.first);
			Math::StdDev(PreTrigRegion.first,PreTrigRegion.second,this->PreTriggerBaselineInfo.first,this->data,this->PreTriggerBaselineInfo.second);

			Math::Average(PostTrigRegion.first,PostTrigRegion.second,this->data,this->PostTriggerBaselineInfo.first);
			Math::StdDev(PostTrigRegion.first,PostTrigRegion.second,this->PostTriggerBaselineInfo.first,this->data,this->PostTriggerBaselineInfo.second);
			for( size_t ii = 1; ii < QDCBounds.size(); ++ii ){
				T sum;
				Math::Integrate(QDCBounds.at(ii-1),QDCBounds.at(ii),this->data,sum);
				this->TQDCSums.push_back(sum);
			}

			auto maxval = std::max_element(this->data.begin(),this->data.end());
			this->MaxInfo = std::make_pair(std::distance(this->data.begin(),maxval),*maxval);

			this->data_baselinesub = std::vector<U>(data.size(),0.0);
			size_t idx = 0;
			for( const auto& val : data ){
				this->data_baselinesub[idx] = static_cast<U>(val) - this->PreTriggerBaselineInfo.first;
				++idx;
			}

			this->BaselineSubMax = this->data_baselinesub[this->MaxInfo.first];

			//std::cout << "bline: (" <<  PreTriggerBaselineInfo.first << ',' << PreTriggerBaselineInfo.second 
			//	  << ") max: (" << MaxInfo.first << ',' << MaxInfo.second 
			//	  << ") maxsub: " << BaselineSubMax 
			//	  << std::endl; 
		}

		const U& GetPreTriggerBaseline() const{
			return this->PreTriggerBaselineInfo.first;
		}

		const U& GetPreTriggerBaselineSTDDev() const{
			return this->PreTriggerBaselineInfo.second;
		}

		const U& GetPostTriggerBaseline() const{
			return this->PostTriggerBaselineInfo.first;
		}

		const U& GetPostTriggerBaselineSTDDev() const{
			return this->PostTriggerBaselineInfo.second;
		}

		const std::vector<T>& GetTQDCSums() const{
			return this->TQDCSums;
		}

		const size_t& GetMaxPos() const{
			return this->MaxInfo.first;
		}

		const T& GetMaxVal() const{
			return this->MaxInfo.second;
		}

		const std::pair<size_t,T>& GetMaxInfo() const{
			return this->MaxInfo;
		}

		const std::pair<U,U>& GetPreTriggerBaselineInfo() const{
			return this->PreTriggerBaselineInfo;
		}

		const std::pair<U,U>& GetPostTriggerBaselineInfo() const{
			return this->PostTriggerBaselineInfo;
		}
		
		U IntegrateRawTrace(const std::pair<size_t,size_t>& bounds) const{
			U val;
			Math::Integrate(bounds.first,bounds.second,this->data,val);
			return val;
		}

		U IntegrateRawTrace(const size_t& start, const size_t& end) const{
			U val;
			Math::Integrate(start,end,this->data,val);
			return val;
		}
		
		U AverageRawTrace(const std::pair<size_t,size_t>& bounds) const{
			U val;
			Math::Average(bounds.first,bounds.second,this->data,val);
			return val;
		}

		U AverageRawTrace(const size_t& start,const size_t& end) const{
			U val;
			Math::Average(start,end,this->data,val);
			return val;
		}
		
		U IntegrateBaselineSubtractedTrace(const std::pair<size_t,size_t>& bounds) const{
			U val;
			Math::Integrate(bounds.first,bounds.second,this->data_baselinesub,val);
			return val;
		}

		U IntegrateBaselineSubtractedTrace(const size_t& start, const size_t& end) const{
			U val;
			Math::Integrate(start,end,this->data_baselinesub,val);
			return val;
		}
		
		U AverageBaselineSubtractedTrace(const std::pair<size_t,size_t>& bounds) const{
			U val;
			Math::Average(bounds.first,bounds.second,this->data_baselinesub,val);
			return val;
		}

		U AverageBaselineSubtractedTrace(const size_t& start,const size_t& end) const{
			U val;
			Math::Average(start,end,this->data_baselinesub,val);
			return val;
		}

		const U& GetBaselineSubtractedMaxValue() const{
			return this->BaselineSubMax;
		}

		void CalcFixedPSD(const size_t& start, const size_t& mid, const size_t& end){
			U pre = this->IntegrateBaselineSubtractedTrace(start,mid);
			U post = this->IntegrateBaselineSubtractedTrace(mid,end);
			this->FixedPSD = std::make_tuple(pre,post,post/(pre+post));
		}

		void CalcFractionalPSD(const size_t& presize, const size_t& postsize, const float& fraction){
			size_t mid = 0;
			U stopval = (BaselineSubMax*fraction);
			for( size_t idx = MaxInfo.first; idx < data_baselinesub.size(); ++idx ){
				if( data_baselinesub[idx] <= stopval ){
					mid = idx;
					break;
				}	
			}
			size_t start = ((mid - presize) < mid) ? (mid - presize) : 0;
			size_t stop = ((mid + postsize) < data_baselinesub.size()) ? (mid + postsize) : (data_baselinesub.size() - mid);
			U pre = this->IntegrateBaselineSubtractedTrace(start,mid);
			U post = this->IntegrateBaselineSubtractedTrace(mid,stop);
			this->FractionalPSD = std::make_tuple(pre,post,post/(pre+post));
		}

		const std::tuple<U,U,U>& GetFixedPSD() const{
			return this->FixedPSD;
		}

		const std::tuple<U,U,U>& GetFractionalPSD() const{
			return this->FractionalPSD;
		}

	private:
		std::vector<T> data;
		std::vector<U> data_baselinesub;
		std::vector<T> TQDCSums;
		std::pair<U,U> PreTriggerBaselineInfo;
		std::pair<U,U> PostTriggerBaselineInfo;
		std::pair<size_t,T> MaxInfo;
		std::tuple<U,U,U> FixedPSD;
		std::tuple<U,U,U> FractionalPSD;
		U BaselineSubMax;
};	

#endif
