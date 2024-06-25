#include <limits>
#include <tuple>

#include "PhysicsData.hpp"
#include "TraceHelper.hpp"

PhysicsData::PhysicsData(int headerlength,int eventlength,int cratenum,int modnum,int channum,int gboard,int gchan,uint32_t rawerg,uint64_t rawts){
	this->HeaderLength = headerlength;
	this->EventLength = eventlength;
	this->CrateNum = cratenum;
	this->ModNum = modnum;
	this->ChanNum = channum;
	this->globalBoardID = gboard;
	this->globalChannelID = gchan;
	this->RawEnergy = rawerg;
	this->RawTimeStamp = rawts;
	this->SpillID = 0;

	this->Location = -1;

	this->Energy = 0.0;
	this->TimeStamp = -1.0;
	this->CFDForcedBit = false;
	this->CFDFraction = -1.0;
	this->CFDSourceBit = -1;

	this->Pileup = false;
	this->Saturation = false;

	this->Phase = -1.0;

	this->ExternalTimestamp = std::numeric_limits<uint64_t>::max();

	this->Trace = TraceHelper<uint16_t,float>();
	this->QDCSums = {};

	this->Type = "";
	this->SubType = "";
	this->Group = "";
	this->Tags = "";
	this->TagList = {};
}

PhysicsData::PhysicsData(const PhysicsData& other) : 
	HeaderLength(other.HeaderLength),
	EventLength(other.EventLength),
	RawEnergy(other.RawEnergy),
	RawEnergyWRandom(other.RawEnergyWRandom),
	RawTimeStamp(other.RawTimeStamp),
	ExternalTimestamp(other.ExternalTimestamp),
	SpillID(other.SpillID),
	Energy(other.Energy),
	TimeStamp(other.TimeStamp),
	CFDTimeStamp(other.CFDTimeStamp),
	CFDForcedBit(other.CFDForcedBit),
	CFDFraction(other.CFDFraction),
	CFDSourceBit(other.CFDSourceBit),
	CrateNum(other.CrateNum),
	ModNum(other.ModNum),
	ChanNum(other.ChanNum),
	Location(other.Location),
	globalChannelID(other.globalChannelID),
	globalBoardID(other.globalBoardID),
	Pileup(other.Pileup),
	Saturation(other.Saturation),
	Phase(other.Phase),
	Trace(other.Trace),  // Assuming TraceHelper has a proper copy constructor
	QDCSums(other.QDCSums),
	ESumTrailing(other.ESumTrailing),
	ESumLeading(other.ESumLeading),
	ESumGap(other.ESumGap),
	ESumBaseLine(other.ESumBaseLine),
	Type(other.Type),
	SubType(other.SubType),
	Group(other.Group),
	Tags(other.Tags),
	SummaryID(other.SummaryID),
	UniqueID(other.UniqueID),
	TagList(other.TagList)
{
}

PhysicsData::PhysicsData(PhysicsData&& other) noexcept :
	HeaderLength(other.HeaderLength),
	EventLength(other.EventLength),
	RawEnergy(other.RawEnergy),
	RawEnergyWRandom(other.RawEnergyWRandom),
	RawTimeStamp(other.RawTimeStamp),
	ExternalTimestamp(other.ExternalTimestamp),
	SpillID(other.SpillID),
	Energy(other.Energy),
	TimeStamp(other.TimeStamp),
	CFDTimeStamp(other.CFDTimeStamp),
	CFDForcedBit(other.CFDForcedBit),
	CFDFraction(other.CFDFraction),
	CFDSourceBit(other.CFDSourceBit),
	CrateNum(other.CrateNum),
	ModNum(other.ModNum),
	ChanNum(other.ChanNum),
	Location(other.Location),
	globalChannelID(other.globalChannelID),
	globalBoardID(other.globalBoardID),
	Pileup(other.Pileup),
	Saturation(other.Saturation),
	Phase(other.Phase),
	Trace(std::move(other.Trace)),  // Assuming TraceHelper has a proper move constructor
	QDCSums(std::move(other.QDCSums)),
	ESumTrailing(other.ESumTrailing),
	ESumLeading(other.ESumLeading),
	ESumGap(other.ESumGap),
	ESumBaseLine(other.ESumBaseLine),
	Type(std::move(other.Type)),
	SubType(std::move(other.SubType)),
	Group(std::move(other.Group)),
	Tags(std::move(other.Tags)),
	SummaryID(std::move(other.SummaryID)),
	UniqueID(std::move(other.UniqueID)),
	TagList(std::move(other.TagList))
{
}

PhysicsData& PhysicsData::operator=(const PhysicsData& other){
	if( this != &other ){
		HeaderLength = other.HeaderLength;
		EventLength = other.EventLength;
		RawEnergy = other.RawEnergy;
		RawEnergyWRandom = other.RawEnergyWRandom;
		RawTimeStamp = other.RawTimeStamp;
		ExternalTimestamp = other.ExternalTimestamp;
		SpillID = other.SpillID;
		Energy = other.Energy;
		TimeStamp = other.TimeStamp;
		CFDTimeStamp = other.CFDTimeStamp;
		CFDForcedBit = other.CFDForcedBit;
		CFDFraction = other.CFDFraction;
		CFDSourceBit = other.CFDSourceBit;
		CrateNum = other.CrateNum;
		ModNum = other.ModNum;
		ChanNum = other.ChanNum;
		Location = other.Location;
		globalChannelID = other.globalChannelID;
		globalBoardID = other.globalBoardID;
		Pileup = other.Pileup;
		Saturation = other.Saturation;
		Phase = other.Phase;
		Trace = other.Trace;  // Assuming TraceHelper has a proper copy constructor
		QDCSums = other.QDCSums;
		ESumTrailing = other.ESumTrailing;
		ESumLeading = other.ESumLeading;
		ESumGap = other.ESumGap;
		ESumBaseLine = other.ESumBaseLine;
		Type = other.Type;
		SubType = other.SubType;
		Group = other.Group;
		Tags = other.Tags;
		SummaryID = other.SummaryID;
		UniqueID = other.UniqueID;
		TagList = other.TagList;
	}
	return *this;
}

PhysicsData& PhysicsData::operator=(PhysicsData&& other) noexcept{
	if( this != &other ){
		HeaderLength = other.HeaderLength;
		EventLength = other.EventLength;
		RawEnergy = other.RawEnergy;
		RawEnergyWRandom = other.RawEnergyWRandom;
		RawTimeStamp = other.RawTimeStamp;
		ExternalTimestamp = other.ExternalTimestamp;
		SpillID = other.SpillID;
		Energy = other.Energy;
		TimeStamp = other.TimeStamp;
		CFDTimeStamp = other.CFDTimeStamp;
		CFDForcedBit = other.CFDForcedBit;
		CFDFraction = other.CFDFraction;
		CFDSourceBit = other.CFDSourceBit;
		CrateNum = other.CrateNum;
		ModNum = other.ModNum;
		ChanNum = other.ChanNum;
		Location = other.Location;
		globalChannelID = other.globalChannelID;
		globalBoardID = other.globalBoardID;
		Pileup = other.Pileup;
		Saturation = other.Saturation;
		Phase = other.Phase;
		Trace = std::move(other.Trace);  // Assuming TraceHelper has a proper move constructor
		QDCSums = std::move(other.QDCSums);
		ESumTrailing = other.ESumTrailing;
		ESumLeading = other.ESumLeading;
		ESumGap = other.ESumGap;
		ESumBaseLine = other.ESumBaseLine;
		Type = std::move(other.Type);
		SubType = std::move(other.SubType);
		Group = std::move(other.Group);
		Tags = std::move(other.Tags);
		SummaryID = std::move(other.SummaryID);
		UniqueID = std::move(other.UniqueID);
		TagList = std::move(other.TagList);
	}
	return *this;
}


//HeaderLength, this is mostly used for pixie data
int PhysicsData::GetHeaderLength() const{
	return this->HeaderLength;
}

int PhysicsData::GetEventLength() const{
	return this->EventLength;
}

//RawEnergy
uint32_t PhysicsData::GetRawEnergy() const{
	return this->RawEnergy;
}

//RawEnergyWRandom
double PhysicsData::GetRawEnergyWRandom() const{
	return this->RawEnergyWRandom;
}

//RawTimeStamp
uint64_t PhysicsData::GetRawTimeStamp() const{
	return this->RawTimeStamp;
}
		
//SpillID
void PhysicsData::SetSpillID(uint64_t id){
	this->SpillID = id;
}

uint64_t PhysicsData::GetSpillID() const{
	return this->SpillID;
}

//Energy
void PhysicsData::SetEnergy(double value1,double value2){
	this->RawEnergyWRandom = value1;
	this->Energy = value2;
}

double PhysicsData::GetEnergy() const{
	return this->Energy;
}

//TimeStamp
void PhysicsData::SetTimeStamp(double value){
	this->TimeStamp = value;
}

double PhysicsData::GetTimeStamp() const{
	return this->TimeStamp;
}

//CFDTimeStamp
void PhysicsData::SetCFDTimeStamp(double value){
	this->CFDTimeStamp = value;
}

double PhysicsData::GetCFDTimeStamp() const{
	return this->CFDTimeStamp;
}

//CFD Forced Bit
void PhysicsData::SetCFDForcedBit(bool value){
	this->CFDForcedBit = value;
}

bool PhysicsData::GetCFDForcedBit() const{
	return this->CFDForcedBit;
}

//CFD Fraction
void PhysicsData::SetCFDFraction(double value){
	this->CFDFraction = value;
}

double PhysicsData::GetCFDFraction() const{
	return this->CFDFraction;
}

//CFD Source Bit
void PhysicsData::SetCFDSourceBit(int value){
	this->CFDSourceBit = value;
}

int PhysicsData::GetCFDSourceBit() const{
	return this->CFDSourceBit;
}

//Crate
int PhysicsData::GetCrate() const{
	return this->CrateNum;
}

//Module
int PhysicsData::GetModule() const{
	return this->ModNum;
}

//Channel
int PhysicsData::GetChannel() const{
	return this->ChanNum;
}

//Location, typically this is crateID*(maxModPerCrate*maxChanPerMod) + modID*(maxChanPerMod) + chanID
//but is overridable within the config file
void PhysicsData::SetLocation(int value){
	this->Location = value;
}

int PhysicsData::GetLocation() const{
	return this->Location;
}

//Pileup
void PhysicsData::SetPileup(bool value){
	this->Pileup = value;
}

bool PhysicsData::GetPileup() const{
	return this->Pileup;
}

//Saturation/trace out of range
void PhysicsData::SetSaturation(bool value){
	this->Saturation = value;
}

bool PhysicsData::GetSaturation() const{
	return this->Saturation;
}

//Phase, need to ask Toby what this means
void PhysicsData::SetPhase(double value){
	this->Phase = value;
}

double PhysicsData::GetPhase() const{
	return this->Phase;
}

//Raw Trace
void PhysicsData::SetRawTraceLength(const unsigned int value){
	this->Trace = TraceHelper<uint16_t,float>(value);
}

void PhysicsData::SetRawTrace(const std::vector<uint16_t>& value){
	this->Trace = TraceHelper<uint16_t,float>(value);
}
void PhysicsData::SetRawTrace(std::vector<uint16_t>&& value){
	this->Trace = TraceHelper<uint16_t,float>(std::move(value));
}

const std::vector<uint16_t>& PhysicsData::GetRawTrace() const{
	return this->Trace.GetData(); 
}
		

std::vector<uint16_t>& PhysicsData::GetRawTraceData(){
	return this->Trace.GetRawData();
}

void PhysicsData::SetRawQDCSumLength(const unsigned int value){
	this->QDCSums = std::vector<unsigned int>(value);
}

void PhysicsData::SetQDCValue(const unsigned int& idx,const unsigned int& value){
	this->QDCSums[idx] = value;
}

//QDC Sums
void PhysicsData::SetQDCSums(const std::vector<unsigned int>& value){
	this->QDCSums = value;
}
void PhysicsData::SetQDCSums(std::vector<unsigned int>&& value){
	this->QDCSums = std::move(value);
}

const std::vector<unsigned int>& PhysicsData::GetQDCSums() const{
	return this->QDCSums;
}

//Type
void PhysicsData::SetType(const std::string& value){
	this->Type = value;
}

const std::string& PhysicsData::GetType() const{
	return this->Type;
}

//SubType
void PhysicsData::SetSubType(const std::string& value){
	this->SubType = value;
}

const std::string& PhysicsData::GetSubType() const{
	return this->SubType;
}

//Group
void PhysicsData::SetGroup(const std::string& value){
	this->Group = value;
}

const std::string& PhysicsData::GetGroup() const{
	return this->Group;
}

//Tags
void PhysicsData::SetTags(const std::string& value){
	this->Tags = value;
}

const std::string& PhysicsData::GetTags() const{
	return this->Tags;
}

void PhysicsData::SetSummaryID(const std::string& value){
	this->SummaryID = value; 
}

const std::string& PhysicsData::GetSummaryID() const{
	return this->SummaryID;
}

void PhysicsData::SetUniqueID(const std::string& value){
	this->UniqueID = value;
}

const std::string& PhysicsData::GetUniqueID() const{
	return this->UniqueID;
}

//Tag List
void PhysicsData::SetTagList(const std::set<std::string>& value){
	this->TagList = value;
} 

std::set<std::string> PhysicsData::GetTagList() const{
	return this->TagList;
}

void PhysicsData::SetESumLeading(unsigned int val){
	this->ESumLeading = val;
}

unsigned int PhysicsData::GetESumLeading() const{
	return this->ESumLeading;
}

void PhysicsData::SetESumTrailing(unsigned int val){
	this->ESumTrailing = val;
}

unsigned int PhysicsData::GetESumTrailing() const{
	return this->ESumTrailing;
}
void PhysicsData::SetESumGap(unsigned int val){
	this->ESumGap = val;
}

unsigned int PhysicsData::GetESumGap() const{
	return this->ESumGap;
}
void PhysicsData::SetESumBaseline(unsigned int val){
	this->ESumBaseLine = val;
}

unsigned int PhysicsData::GetESumBaseline() const{
	return this->ESumBaseLine;
}

int PhysicsData::GetGlobalChannelID() const{
	return this->globalChannelID;
}

int PhysicsData::GetGlobalBoardID() const{
	return this->globalBoardID;
}

bool PhysicsData::operator<(const PhysicsData& rhs) const{
	return this->RawTimeStamp < rhs.RawTimeStamp;
	//return this->TimeStamp < rhs.TimeStamp;
	//return std::tie(this->TimeStamp,this->CrateNum,this->ModNum,this->ChanNum,this->RawEnergy) < std::tie(rhs.TimeStamp,rhs.CrateNum,rhs.ModNum,rhs.ChanNum,rhs.RawEnergy);
}

bool PhysicsData::operator>(const PhysicsData& rhs) const{
	return rhs < (*this);
}

bool PhysicsData::operator<=(const PhysicsData& rhs) const{
	return !((*this) > rhs);
}

bool PhysicsData::operator>=(const PhysicsData& rhs) const{
	return !((*this) < rhs);
}

bool PhysicsData::operator==(const PhysicsData& rhs) const{
	return (this->CrateNum == rhs.CrateNum) and (this->ModNum == rhs.ModNum) and (this->ChanNum == rhs.ChanNum) and (this->RawTimeStamp == rhs.RawTimeStamp) and (this->RawEnergy == rhs.RawEnergy);
}

bool PhysicsData::operator!=(const PhysicsData& rhs) const{
	return !((*this) == rhs);
}

bool PhysicsData::HasTag(const std::string& tagid) const{
	return (this->TagList.find(tagid) != this->TagList.end());
}

void PhysicsData::SetExternalTimeStamp(uint64_t val){
	this->ExternalTimestamp = val;
}

uint64_t PhysicsData::GetExternalTimeStamp() const{
	return this->ExternalTimestamp;
}
