#ifndef __PHYSICS_DATA_HPP__
#define __PHYSICS_DATA_HPP__

#include <string>
#include <string_view>
#include <vector>
#include <set>

#include "TraceHelper.hpp"

class PhysicsData{
	public:
		PhysicsData(int,int,int,int,int,int,uint32_t,uint64_t);
		~PhysicsData() = default;

		//HeaderLength, this is mostly used for pixie data
		int GetHeaderLength() const;

		//EventLength
		int GetEventLength() const;

		//RawEnergy
		uint32_t GetRawEnergy() const;

		//RawTimeStamp
		uint64_t GetRawTimeStamp() const;

		//Energy
		void SetEnergy(double);
		double GetEnergy() const;

		//TimeStamp
		void SetTimeStamp(double);
		double GetTimeStamp() const;

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

		//QDC Sums
		void SetQDCSums(const std::vector<unsigned int>&);
		void SetQDCSums(std::vector<unsigned int>&&);

		const std::vector<unsigned int>& GetQDCSums() const;

		//Make faster update
		void SetRawQDCSumLength(const unsigned int);
		void SetQDCValue(const unsigned int&,const unsigned int&);

		//SuperType
		void SetSuperType(const std::string&);

		std::string_view GetSuperType() const;

		//Type
		void SetType(const std::string&);

		std::string_view GetType() const;

		//SubType
		void SetSubType(const std::string&);

		std::string_view GetSubType() const;

		//Group
		void SetGroup(const std::string&);

		std::string_view GetGroup() const;

		//Tags
		void SetTags(const std::string&);

		std::string_view GetTags() const;

		//UniqueID
		void SetUniqueID(const std::string&);

		std::string_view GetUniqueID() const;

		//Tag List
		void SetTagList(const std::set<std::string>&); 

		std::set<std::string> GetTagList() const;

		template<typename OStream>
		friend OStream& operator<<(OStream& os, const PhysicsData& pd) {
			os << "PhysicsData( Crate: " << pd.CrateNum 
			   << " Module: " << pd.ModNum
			   << " Channel: " << pd.ChanNum
			   << " Location: " << pd.Location
			   << " RawEnergy: " << pd.RawEnergy
			   << " Energy: " << pd.Energy
			   << " RawTimeStamp: " << pd.RawTimeStamp 
			   << " TimeStamp: " << pd.TimeStamp
			   << " Saturation: " << pd.Saturation
			   << " Pileup: " << pd.Pileup
			   << " TraceLength: " << pd.Trace.GetSize() 
			   << " )";
			return os;
		}
	private:
		//this is info decoded from the data files
		int HeaderLength;	
		int EventLength;
		uint32_t RawEnergy;
		uint64_t RawTimeStamp;

		double Energy;	
		double TimeStamp;

		bool CFDForcedBit;
		double CFDFraction;
		int CFDSourceBit;

		int CrateNum;
		int ModNum;
		int ChanNum;
		int Location;
		int globalChannelID;

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
		std::string_view SuperType;
		std::string_view Type;
		std::string_view SubType;
		std::string_view Group;
		std::string_view Tags;
		std::string_view UniqueID;
		std::set<std::string> TagList;

		//Trace Helper, should probably hide this from end user though
		//and only expose what it can determine
		void SetTraceHelper(TraceHelper<uint16_t, float>);
		void SetTraceHelper(const TraceHelper<uint16_t, float>&);
		void SetTraceHelper(TraceHelper<uint16_t, float>&&);

		TraceHelper<uint16_t,float> GetTraceHelper() const;
};

#endif
