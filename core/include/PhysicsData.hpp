/// @file PhysicsData.hpp
/// @author T. J. Ruland
/// @brief Class that holds the raw information from the digitizers
///

#ifndef __PHYSICS_DATA_HPP__
#define __PHYSICS_DATA_HPP__

#include <string>
#include <vector>
#include <utility>
#include <set>
#include <map>

#if defined(__GNUC__) && (__GNUC__ >= 13)
#include <cstdint>
#endif

#include "TraceHelper.hpp"

/// @addtogroup Events
/// @{
/// @details Classes associated with constructing Events
/// @class PhysicsData
/// @brief PhysicsData object that all the translators convert their respective data files into
/// @details This class does all the heavy lifting, so far all the translators interact translate them directly from their data files
/// and they're loaded into an EventSummary managed by the EventHistoryManager
class PhysicsData {
public:
	/// @brief Constructs physics data object from the decoded word zero
	/// @details Several variables are calculated and defaulted during this process
	/// 	   -  HeaderLength : headerlength
	/// 	   -  EventLength : eventlength
	/// 	   -  CrateNum : cratenum
	/// 	   -  ModNum : modnum
	/// 	   -  ChanNum : channum
	/// 	   -  globalBoardID : gboard
	/// 	   -  globalChannelID : gchan
	/// 	   -  RawEnergy : rawerg
	/// 	   -  RawTimeStamp : rawts
	///        -  SpillID : 0
	///        -  CMapID : "cratenum:modnum:channum"
	///        -  Location : -1
	///        -  Energy : 0.0
	///        -  TimeStamp : -1.0
	///        -  CFDForcedBit : false
	///        -  CFDFraction : -1.0
	///        -  CFDSourceBit : -1
	///        -  Pileup : false
	///        -  Saturation : false
	///        -  Phase : -1.0
	///        -  ExternalTimestamp : std::numeric_limits<uint64_t>::max()
	///        -  Trace : TraceHelper<uint16_t,float>()
	///        -  QDCSums : {}
	///        -  Type : ""
	///        -  SubType : ""
	///        -  Group : ""
	///        -  Tags : ""
	///        -  TagList : {}
	/// @param[in] headerlength size of the pixie16 header in words (4/8/12/16)
	/// @param[in] eventlength size of the header + the tracelength in words
	/// @param[in] cratenum decoded crate number from word zero [0-4]
	/// @param[in] modnum decoded module number from word zero [0-12]
	/// @param[in] channum decoded channel number from word zero [0-15] or [0-31] if using Rev. H
	/// @param[in] gboard linearized global board number calculated from modnum + cratenum*13
	/// @param[in] gchan linearized global channel number calculated form channum + modnum*16 + cratenum*13*16
	/// @param[in] rawerg pixie16 filter energy
	/// @param[in] rawts pixie16 low resolution timestamp
	PhysicsData(int, int, int, int, int, int, int, uint32_t, uint64_t);

	/// @brief compiler default destructor
	~PhysicsData() = default;

	/// @brief custom defined copy constructor
	/// @param other object we are copying data from
	PhysicsData(const PhysicsData&);

	/// @brief custom defined move constructor
	/// @param other object we are moving data from, this leaves other in an unspecified state since we move all the stl objects too with std::move
	PhysicsData(PhysicsData&&) noexcept;

	/// @brief custom overload copy assignment operator
	/// @param other object we are copying data from
	PhysicsData& operator=(const PhysicsData&);

	/// @brief custom defined move assignment operator
	/// @param other object we are moving data from, this leaves other in an unspecified state since we move all the stl objects too with std::move
	PhysicsData& operator=(PhysicsData&&) noexcept;

	// HeaderLength, this is mostly used for pixie data
	/// @brief retrieve the HeaderLength
	/// @return HeaderLength
	int GetHeaderLength() const;

	// EventLength
	/// @brief retrieve the event length the object was constructed with
	/// @return EventLength
	int GetEventLength() const;

	// RawEnergy
	/// @brief retrieve the raw energy the object was constructed with
	/// @return RawEnergy
	uint32_t GetRawEnergy() const;

	/// @brief retrieve the raw energy the object was constructed with plus the random [0,1) that was added later
	/// @return RawEnergyWRandom
	double GetRawEnergyWRandom() const;

	// RawTimeStamp
	/// @brief retrieve the raw timestamp in pixie ticks the object was constructed with
	/// @return RawTimeStamp
	uint64_t GetRawTimeStamp() const;

	// SpillID
	/// @brief helper for decoding poll2 data since a spill can be split across 2 actual spills when there is high data rate
	/// @param[in] id tracked spill id as we translate from ldf into the PhysicsData object
	void SetSpillID(uint64_t);

	/// @brief retrieve the spill id assigned to this object (only useful when decoding poll2 data)
	/// @return SpillID
	uint64_t GetSpillID() const;

	// Energy
	/// @brief set the energy values that require aliasing
	/// @param[in] value1 uncalibrated raw energy with random [0,1.0)
	/// @param[in] value2 calibrated energy that was been aliased
	void SetFilterEnergy(double, double);
	void SetAliasValue(double);
	void SetInternalFilterEnergy(double, double);
	void SetInternalIntegralEnergy(double, double);

	/// @brief get the calibrated energy value
	/// @return Energy
	double GetEnergy() const;

	// TimeStamp
	/// @brief set the low resolution filter timestamp in ns
	/// @param[in] value low resolution filter timestamp in ns
	void SetTimeStamp(double);

	/// @brief get the low resolution filter timestamp in ns
	/// @return TimeStamp
	double GetTimeStamp() const;

	// CFDTimeStamp
	/// @brief set the on-board cfd timestamp in ns
	/// @param[in] value on-board cfd timestamp in ns
	void SetCFDTimeStamp(double);

	/// @brief get the on-board cfd timestamp in ns
	/// @return CFDTimeStamp
	double GetCFDTimeStamp() const;

	// CFD Forced Bit
	/// @brief set the on-board cfd forced bit
	/// @param[in] value bool of if the on-board cfd was forced
	void SetCFDForcedBit(bool);

	/// @brief get whether the on-board cfd was force-triggered
	/// @return CFDForcedBit
	bool GetCFDForcedBit() const;

	// CFD Fraction
	/// @brief set the on-board cfd fraction
	/// @param[in] value fraction of the on-board cfd (see pixie16 manual for better detail)
	void SetCFDFraction(double);

	/// @brief get the on-board cfd fraction
	/// @return CFDFraction
	double GetCFDFraction() const;

	// CFD Source Bit
	/// @brief set which adc triggered the on-board cfd
	/// @param[in] value adc number (varies with digitizer frequency, see pixie16 manual for better detail)
	void SetCFDSourceBit(int);

	/// @brief get which adc triggered the on-board cfd
	/// @return CFDSourceBit
	int GetCFDSourceBit() const;

	// Crate
	/// @brief get the word zero decoded crate number
	/// @return CrateNum
	int GetCrate() const;

	// Module
	/// @brief get the word zero decoded module number
	/// @return ModNum
	int GetModule() const;

	// Channel
	/// @brief get the word zero decoded channel number
	/// @return ChanNum
	int GetChannel() const;

	// GlobalChannelID
	/// @brief get the global channel id, derived from parsing xml and assigned by ChannelMap
	/// @return globalChannelID
	int GetGlobalChannelID() const;

	// GlobalBoardID
	/// @brief get the global board id, derived from parsing xml and assigned by ChannelMap
	/// @return globalBoardID
	int GetGlobalBoardID() const;

	// Location, typically this is crateID*(maxModPerCrate*maxChanPerMod) + modID*(maxChanPerMod) + chanID
	// but is overridable within the config file
	/// @brief set the parsed location within the config file, currently this is the same as globalChannelID
	/// @param[in] value config parsed/calculated location value
	void SetLocation(int);

	/// @brief get the location of the channel parsed/determined from the input config file
	/// @return Location
	int GetLocation() const;

	// Pileup
	/// @brief set the decoded finishcode (i.e. pileup)
	/// @param[in] value decoded finishcode from translated datastream
	void SetPileup(bool);

	/// @brief get the whether decoded detector hit had pileup
	/// @return Pileup
	bool GetPileup() const;

	// Saturation/trace out of range
	/// @brief set the decoded trace out of range information
	/// @param[in] value trace out of range flag from translated datastream
	void SetSaturation(bool);

	/// @brief get the whether decoded detector hit had the input trace saturate
	/// @return Saturation
	bool GetSaturation() const;

	/// @brief set the external timestamp that is passed along through the frontplane of xia boards
	/// @param[in] value decoded from special pixie spill
	void SetExternalTimeStamp(uint64_t);

	/// @brief get the decoded external timestamp as it is changing through the data stream
	/// @return ExternalTimestamp
	uint64_t GetExternalTimeStamp() const;

	// ESums
	/// @brief set the decoded esum value, this is the leading edge of the trap?
	/// @param[in] value decoded from translated datastream
	void SetESumLeading(unsigned int);

	/// @brief get the decoded leading edge esum
	/// @return ESumLeading
	unsigned int GetESumLeading() const;

	/// @brief set the decoded esum value, this is the trailing edge of the trap?
	/// @param[in] value decoded from translated datastream
	void SetESumTrailing(unsigned int);

	/// @brief get the decoded trailing edge esum
	/// @return ESumTrailing
	unsigned int GetESumTrailing() const;

	/// @brief set the decoded esum value, this is the gap of the trap?
	/// @param[in] value decoded from translated datastream
	void SetESumGap(unsigned int);

	/// @brief get the decoded gap esum
	/// @return ESumGap
	unsigned int GetESumGap() const;

	/// @brief set the decoded esum value, this is the baseline of the trap?
	/// @param[in] value decoded from translated datastream
	void SetESumBaseline(unsigned int);

	/// @brief get the decoded baseline esum
	/// @return ESumBaseLine
	unsigned int GetESumBaseline() const;

	// Phase, need to ask Toby what this means
	/// @brief what the fuck is this?
	/// @param[in] decoded phase value from the datastream?
	void SetPhase(double);
	double GetPhase() const;

	// Raw Raw Trace
	void SetRawTraceLength(const unsigned int);
	std::vector<uint16_t>& GetRawTraceData();

	// Raw Trace
	void SetRawTrace(const std::vector<uint16_t>&);
	void SetRawTrace(std::vector<uint16_t>&&);

	const std::vector<uint16_t>& GetRawTrace() const;
	const std::vector<float>& GetTraceDerivative() const;
	const std::vector<uint16_t>& GetRawTraceDerivative() const;

	// QDC Sums
	void SetQDCSums(const std::vector<unsigned int>&);
	void SetQDCSums(std::vector<unsigned int>&&);

	const std::vector<unsigned int>& GetQDCSums() const;
	unsigned int GetQDC(size_t) const;

	// Make faster update
	void SetRawQDCSumLength(const unsigned int);
	void SetQDCValue(const unsigned int&, const unsigned int&);

	// Type
	void SetType(const std::string&);

	const std::string& GetType() const;

	// SubType
	void SetSubType(const std::string&);

	const std::string& GetSubType() const;

	// Group
	void SetGroup(const std::string&);

	const std::string& GetGroup() const;

	// Tags
	void SetTags(const std::string&);

	const std::string& GetTags() const;

	// CMapID
	// shouldn't be settable
	const std::string& GetCMapID() const;

	// SummaryID
	void SetSummaryID(const std::string&);

	const std::string& GetSummaryID() const;

	// UniqueID
	void SetUniqueID(const std::string&);

	const std::string& GetUniqueID() const;

	// Tag List
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

	void AnalyzeWaveform(const std::pair<size_t, size_t>&, const std::pair<size_t, size_t>&, const std::vector<size_t>&, const bool&);
	const std::pair<float, float>& GetTracePreTriggerBaseline() const;
	const std::pair<float, float>& GetTracePostTriggerBaseline() const;
	const std::pair<size_t, uint16_t>& GetTraceMaxInfo() const;
	const std::pair<size_t, uint16_t>& GetPSDBoundedTraceMaxInfo() const;
	const float& GetBaselineSubtractedMaxValue() const;
	const float& GetBaselineSubtractedPSDBoundedMaxValue() const;
	float InegrateRawTrace(const std::pair<size_t, size_t>&) const;
	float AverageRawTrace(const std::pair<size_t, size_t>&) const;
	float IntegrateBaselineSubtractedTrace(const std::pair<size_t, size_t>&) const;
	float AverageBaselineSubtractedTrace(const std::pair<size_t, size_t>&) const;
	void CalcTraceFixedPSD(const size_t&, const size_t&, const size_t&, const bool&);
	const std::tuple<float, float, float>& GetTraceFixedPSD() const;
	void CalcTraceFractionalPSD(const size_t&, const size_t&, const float&);
	const std::tuple<float, float, float>& GetTraceFractionalPSD() const;
	void CalculateTraceDerivatives();

	void AddTraceFitInfo(const std::string&, double, double);
	bool DoesTraceFitValueExist(const std::string&) const;
	std::pair<double, double> GetTraceFitValue(const std::string&) const;
	double GetInternalFilterRaw() const;
	double GetInternalFilterEnergy() const;
	double GetInternalIntegralRaw() const;
	double GetInternalIntegralEnergy() const;
	double GetAliasValue() const;

private:
	// this is info decoded from the data files
	int HeaderLength; /**< decoded length of event header (4/8/12/16) */
	int EventLength; /**< decoded legth of the event HeaderLength+TraceLength */
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

	// this is info derived from the channel map
	std::string Type;
	std::string SubType;
	std::string Group;
	std::string Tags;
	std::string SummaryID;
	std::string UniqueID;
	std::string CMapID;
	std::set<std::string> TagList;

	// From trace fitting info
	std::map<std::string, std::pair<double, double>> TraceFitInfo;

	// From internal trap filter by default is just the FilterEnergy
	double InternalFilterRaw;
	double InternalFilterEnergy;

	// From internal integration by default is just the FilterEnergy
	double InternalIntegralRaw;
	double InternalIntegralEnergy;

	double AliasValue;

	// Trace Helper, should probably hide this from end user though
	// and only expose what it can determine
	void SetTraceHelper(TraceHelper<uint16_t, float>);
	void SetTraceHelper(const TraceHelper<uint16_t, float>&);
	void SetTraceHelper(TraceHelper<uint16_t, float>&&);

	TraceHelper<uint16_t, float> GetTraceHelper() const;
};
/// @}

#endif
