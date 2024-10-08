#ifndef __PHYSICS_DATA_HPP__
#define __PHYSICS_DATA_HPP__

#include <string>
#include <vector>
#include <set>

#include "TraceHelper.hpp"

class PhysicsData{
	public:
		PhysicsData(int,int,int,int,int,int,int,uint32_t,uint64_t);
		~PhysicsData() = default;

		PhysicsData(const PhysicsData&);
		PhysicsData(PhysicsData&&) noexcept;

		PhysicsData& operator=(const PhysicsData&);
		PhysicsData& operator=(PhysicsData&&) noexcept;

		//HeaderLength, this is mostly used for pixie data
		int GetHeaderLength() const;

		//EventLength
		int GetEventLength() const;

		//RawEnergy
		uint32_t GetRawEnergy() const;
		double GetRawEnergyWRandom() const;

		//RawTimeStamp
		uint64_t GetRawTimeStamp() const;

		//SpillID 
		void SetSpillID(uint64_t);
		uint64_t GetSpillID() const;

		//Energy
		void SetEnergy(double,double);
		double GetEnergy() const;

		//TimeStamp
		void SetTimeStamp(double);
		double GetTimeStamp() const;

		//CFDTimeStamp
		void SetCFDTimeStamp(double);
		double GetCFDTimeStamp() const;

		//CFD Forced Bit
		void SetCFDForcedBit(bool);
		bool GetCFDForcedBit() const;

		//CFD Fraction
		void SetCFDFraction(double);
		double GetCFDFraction() const;

		//CFD Source Bit
		void SetCFDSourceBit(int);
		int GetCFDSourceBit() const;

		//Crate
		int GetCrate() const;

		//Module
		int GetModule() const;

		//Channel
		int GetChannel() const;

		//GlobalChannelID 
		int GetGlobalChannelID() const;

		//GlobalBoardID 
		int GetGlobalBoardID() const;

		//Location, typically this is crateID*(maxModPerCrate*maxChanPerMod) + modID*(maxChanPerMod) + chanID
		//but is overridable within the config file
		void SetLocation(int);
		int GetLocation() const;

		//Pileup
		void SetPileup(bool);
		bool GetPileup() const;

		//Saturation/trace out of range
		void SetSaturation(bool);
		bool GetSaturation() const;

		void SetExternalTimeStamp(uint64_t);
		uint64_t GetExternalTimeStamp() const;

		//ESums
		void SetESumLeading(unsigned int);
		unsigned int GetESumLeading() const;
		void SetESumTrailing(unsigned int);
		unsigned int GetESumTrailing() const;
		void SetESumGap(unsigned int);
		unsigned int GetESumGap() const;
		void SetESumBaseline(unsigned int);
		unsigned int GetESumBaseline() const;

		//Phase, need to ask Toby what this means
		void SetPhase(double);
		double GetPhase() const;

		//Raw Raw Trace
		void SetRawTraceLength(const unsigned int);
		std::vector<uint16_t>& GetRawTraceData();
		
		//Raw Trace
		void SetRawTrace(const std::vector<uint16_t>&);
		void SetRawTrace(std::vector<uint16_t>&&);

		const std::vector<uint16_t>& GetRawTrace() const;
		const std::vector<float>& GetTraceDerivative() const;
		const std::vector<uint16_t>& GetRawTraceDerivative() const;

		//QDC Sums
		void SetQDCSums(const std::vector<unsigned int>&);
		void SetQDCSums(std::vector<unsigned int>&&);

		const std::vector<unsigned int>& GetQDCSums() const;

		//Make faster update
		void SetRawQDCSumLength(const unsigned int);
		void SetQDCValue(const unsigned int&,const unsigned int&);

		//Type
		void SetType(const std::string&);

		const std::string& GetType() const;

		//SubType
		void SetSubType(const std::string&);

		const std::string& GetSubType() const;

		//Group
		void SetGroup(const std::string&);

		const std::string& GetGroup() const;

		//Tags
		void SetTags(const std::string&);

		const std::string& GetTags() const;

		//CMapID
		//shouldn't be settable
		const std::string& GetCMapID() const;

		//SummaryID
		void SetSummaryID(const std::string&);

		const std::string& GetSummaryID() const;

		//UniqueID
		void SetUniqueID(const std::string&);

		const std::string& GetUniqueID() const;

		//Tag List
		void SetTagList(const std::set<std::string>&); 

		std::set<std::string> GetTagList() const;

		bool HasTag(const std::string&) const;

		template<typename OStream>
		friend OStream& operator<<(OStream& os, const PhysicsData& pd) {
			os << "PhysicsData( \nCrate: " << pd.CrateNum 
			   << "\nModule: " << pd.ModNum
			   << "\nChannel: " << pd.ChanNum
			   << "\nLocation: " << pd.Location
			   << "\ngChanID: " << pd.globalChannelID
			   << "\ngBoardID: " << pd.globalBoardID
			   << "\nRawEnergy: " << pd.RawEnergy
			   << "\nEnergy: " << pd.Energy
			   << "\nRawTimeStamp: " << pd.RawTimeStamp 
			   << "\nTimeStamp: " << pd.TimeStamp
			   << "\nSaturation: " << pd.Saturation
			   << "\nPileup: " << pd.Pileup
			   << "\nTraceLength: " << pd.Trace.GetSize() 
			   << "\nType: " << pd.Type 
			   << "\nSubtype: " << pd.SubType 
			   << "\nGroup:" << pd.Group 
			   << "\nTag: " << pd.Tags
			   << "\n)";
			return os;
		}

		bool operator<(const PhysicsData&) const;
		bool operator>(const PhysicsData&) const;
		bool operator<=(const PhysicsData&) const;
		bool operator>=(const PhysicsData&) const;
		bool operator==(const PhysicsData&) const;
		bool operator!=(const PhysicsData&) const;

		void AnalyzeWaveform(const std::pair<size_t,size_t>&,const std::pair<size_t,size_t>&,const std::vector<size_t>&);
		const std::pair<float,float>& GetTracePreTriggerBaseline() const;
		const std::pair<float,float>& GetTracePostTriggerBaseline() const;
		const std::pair<size_t,uint16_t>& GetTraceMaxInfo() const;
		const std::pair<size_t,uint16_t>& GetPSDBoundedTraceMaxInfo() const;
		const float& GetBaselineSubtractedMaxValue() const;
		const float& GetBaselineSubtractedPSDBoundedMaxValue() const;
		float InegrateRawTrace(const std::pair<size_t,size_t>&) const;
		float AverageRawTrace(const std::pair<size_t,size_t>&) const;
		float IntegrateBaselineSubtractedTrace(const std::pair<size_t,size_t>&) const;
		float AverageBaselineSubtractedTrace(const std::pair<size_t,size_t>&) const;
		void CalcTraceFixedPSD(const size_t&,const size_t&, const size_t&);
		const std::tuple<float,float,float>& GetTraceFixedPSD() const;
		void CalcTraceFractionalPSD(const size_t&,const size_t&, const float&);
		const std::tuple<float,float,float>& GetTraceFractionalPSD() const;
		void CalculateTraceDerivatives();
	private:
		//this is info decoded from the data files
		int HeaderLength;	
		int EventLength;
		uint32_t RawEnergy;
		double RawEnergyWRandom;
		uint64_t RawTimeStamp;

		uint64_t ExternalTimestamp;

		uint64_t SpillID;

		double Energy;	
		double TimeStamp;
		double CFDTimeStamp;

		bool CFDForcedBit;
		double CFDFraction;
		int CFDSourceBit;

		int CrateNum;
		int ModNum;
		int ChanNum;
		int Location;
		int globalChannelID;
		int globalBoardID;

		bool Pileup;
		bool Saturation;
		double Phase;
		TraceHelper<uint16_t, float> Trace;
		std::vector<unsigned int> QDCSums;

		unsigned int ESumTrailing;
		unsigned int ESumLeading;
		unsigned int ESumGap;
		unsigned int ESumBaseLine;

		//this is info derived from the channel map
		std::string Type;
		std::string SubType;
		std::string Group;
		std::string Tags;
		std::string SummaryID;
		std::string UniqueID;
		std::string CMapID;
		std::set<std::string> TagList;

		//Trace Helper, should probably hide this from end user though
		//and only expose what it can determine
		void SetTraceHelper(TraceHelper<uint16_t, float>);
		void SetTraceHelper(const TraceHelper<uint16_t, float>&);
		void SetTraceHelper(TraceHelper<uint16_t, float>&&);

		TraceHelper<uint16_t,float> GetTraceHelper() const;
};

#endif
