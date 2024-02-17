#include "PhysicsData.hpp"
#include "TraceHelper.hpp"

PhysicsData::PhysicsData(int headerlength,int eventlength,int cratenum,int modnum,int channum,int gchan,uint32_t rawerg,uint64_t rawts){
	this->HeaderLength = headerlength;
	this->EventLength = eventlength;
	this->CrateNum = cratenum;
	this->ModNum = modnum;
	this->ChanNum = channum;
	this->globalChannelID = gchan;
	this->RawEnergy = rawerg;
	this->RawTimeStamp = rawts;

	this->Location = -1;

	this->Energy = 0.0;
	this->TimeStamp = -1.0;
	this->CFDForcedBit = false;
	this->CFDFraction = -1.0;
	this->CFDSourceBit = -1;

	this->Pileup = false;
	this->Saturation = false;

	this->Phase = -1.0;

	this->Trace = TraceHelper<uint16_t,float>();
	this->QDCSums = {};

	this->Type = "";
	this->SubType = "";
	this->Group = "";
	this->Tags = "";
	this->TagList = {};
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

//RawTimeStamp
uint64_t PhysicsData::GetRawTimeStamp() const{
	return this->RawTimeStamp;
}

//Energy
void PhysicsData::SetEnergy(double value){
	this->Energy = value;
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
void PhysicsData::SetRawTrace(const std::vector<uint16_t>& value){
	Trace = TraceHelper<uint16_t,float>(value);
}
void PhysicsData::SetRawTrace(std::vector<uint16_t>&& value){
	Trace = TraceHelper<uint16_t,float>(std::move(value));
}

const std::vector<uint16_t>& PhysicsData::GetRawTrace() const{
	return Trace.GetData(); 
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

//SuperType
void PhysicsData::SetSuperType(const std::string& value){
	this->SuperType = value;
}
void PhysicsData::SetSuperType(std::string&& value){
	this->SuperType = std::move(value);
}

std::string PhysicsData::GetSuperType() const{
	return this->SuperType;
}

//Type
void PhysicsData::SetType(const std::string& value){
	this->Type = value;
}
void PhysicsData::SetType(std::string&& value){
	this->Type = std::move(value);
}

std::string PhysicsData::GetType() const{
	return this->Type;
}

//SubType
void PhysicsData::SetSubType(const std::string& value){
	this->SubType = value;
}
void PhysicsData::SetSubType(std::string&& value){
	this->SubType = std::move(value);
}

std::string PhysicsData::GetSubType() const{
	return this->SubType;
}

//Group
void PhysicsData::SetGroup(const std::string& value){
	this->Group = value;
}
void PhysicsData::SetGroup(std::string&& value){
	this->Group = std::move(value);
}

std::string PhysicsData::GetGroup() const{
	return this->Group;
}

//Tags
void PhysicsData::SetTags(const std::string& value){
	this->Tags = value;
}
void PhysicsData::SetTags(std::string&& value){
	this->Tags = std::move(value);
}

std::string PhysicsData::GetTags() const{
	return this->Tags;
}

void PhysicsData::SetUniqueID(const std::string& value){
	this->UniqueID = value;
}

void PhysicsData::SetUniqueID(std::string&& value){
	this->UniqueID = std::move(value);
}

std::string PhysicsData::GetUniqueID() const{
	return this->UniqueID;
}

//Tag List
void PhysicsData::SetTagList(const std::set<std::string>& value){
	this->TagList = value;
} 
void PhysicsData::SetTagList(std::set<std::string>&& value){
	this->TagList = std::move(value);
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
