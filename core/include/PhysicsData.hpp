#ifndef __PHYSICS_DATA_HPP__
#define __PHYSICS_DATA_HPP__

#include <string>
#include <vector>
#include <set>

#include "TraceHelper.hpp"

class PhysicsData{
	public:
		PhysicsData(int,int,int,int,uint32_t,uint64_t);
		~PhysicsData() = default;

		//HeaderLength, this is mostly used for pixie data
		int GetHeaderLength() const;

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

		//Phase, need to ask Toby what this means
		void SetPhase(double);
		double GetPhase() const;

		//Raw Trace
		void SetRawTrace(const std::vector<unsigned int>&);
		void SetRawTrace(std::vector<unsigned int>&&);

		const std::vector<unsigned int>& GetRawTrace() const;

		//QDC Sums
		void SetQDCSums(const std::vector<unsigned int>&);
		void SetQDCSums(std::vector<unsigned int>&&);

		const std::vector<unsigned int>& GetQDCSums() const;

		//Type
		void SetType(const std::string&);
		void SetType(std::string&&);

		std::string GetType() const;

		//SubType
		void SetSubType(const std::string&);
		void SetSubType(std::string&&);

		std::string GetSubType() const;

		//Group
		void SetGroup(const std::string&);
		void SetGroup(std::string&&);

		std::string GetGroup() const;

		//Tags
		void SetTags(const std::string&);
		void SetTags(std::string&&);

		std::string GetTags() const;

		//Tag List
		void SetTagList(const std::set<std::string>&); 
		void SetTagList(std::set<std::string>&&);

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

		bool Pileup;
		bool Saturation;
		double Phase;
		TraceHelper<unsigned int, float> Trace;
		std::vector<unsigned int> QDCSums;

		//this is info derived from the channel map
		std::string Type;
		std::string SubType;
		std::string Group;
		std::string Tags;
		std::set<std::string> TagList;

		//Trace Helper, should probably hide this from end user though
		//and only expose what it can determine
		void SetTraceHelper(TraceHelper<unsigned int, float>);
		void SetTraceHelper(const TraceHelper<unsigned int, float>&);
		void SetTraceHelper(TraceHelper<unsigned int, float>&&);

		TraceHelper<unsigned int,float> GetTraceHelper() const;
};

#endif