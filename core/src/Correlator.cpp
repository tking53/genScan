#include "Correlator.hpp"
#include <stdexcept>

Correlator::Correlator(double eventwidth){
	this->CorrelationType = UNKNOWN;
	this->Width = eventwidth;
}

void Correlator::SetCorrelationType(Correlator::CORRELATIONWINDOWTYPE type){
	if( type == Correlator::CORRELATIONWINDOWTYPE::UNKNOWN ){
		throw std::runtime_error("UNKNOWN CORRELATIONWINDOWTYPE, KNOWN TYPES ARE ROLLINGWIDTH,ROLLINGTRIGGER,ANALOG");
	}	
	this->CorrelationType = type;
}

void Correlator::AddTriggerChannel(int crateID,int moduleID,int channelID){
	this->Triggers.insert({.Crate = crateID, .Module = moduleID, .Channel = channelID});	
}

bool Correlator::IsWithinCorrelationWindow(double ts,int crateID,int moduleID,int channelID){
	switch( this->CorrelationType ){
		case ROLLINGTRIGGER:
			return this->RollingWindowCorrelation(ts);
			break;
		case ROLLINGWIDTH:
			return this->RollingTriggerCorrelation(ts);
			break;
		case ANALOG:
			return this->AnalogCorrelation(crateID,moduleID,channelID);
			break;
		case UNKNOWN:
		default:
			throw std::runtime_error("UNHANDLED CORRELATION TYPE");
			break;
	}
}

bool Correlator::AnalogCorrelation(int crateID,int moduleID,int channelID) const{
	AnalogTrigger trigtest = {.Crate = crateID, .Module = moduleID, .Channel = channelID};
	return (this->Triggers.find(trigtest) != this->Triggers.end());
}

bool Correlator::RollingWindowCorrelation(double ts){
	if( std::abs(ts - MinHeap.top()) < Width ){
		MinHeap.push(ts);
		return true;
	}else{
		MinHeap.clear();
		MinHeap.push(ts);
		return false;
	}
}

bool Correlator::RollingTriggerCorrelation(double ts){
	if( std::abs(ts - MaxHeap.top()) < Width ){
		MaxHeap.push(ts);
		return true;
	}else{
		MaxHeap.clear();
		MaxHeap.push(ts);
		return false;
	}
}
