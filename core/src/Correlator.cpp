#include "Correlator.hpp"
#include <stdexcept>
#include <iostream>

Correlator::Correlator(const std::string& log,double eventwidth){
	this->CorrelationType = UNKNOWN;
	this->Width = eventwidth;
	this->LogName = log;
}

void Correlator::SetCorrelationType(Correlator::CORRELATIONWINDOWTYPE type){
	if( type == Correlator::CORRELATIONWINDOWTYPE::UNKNOWN ){
		throw std::runtime_error("UNKNOWN CORRELATIONWINDOWTYPE, KNOWN TYPES ARE ROLLINGWIDTH,ROLLINGTRIGGER,ANALOG");
	}	
	this->CorrelationType = type;
	switch( this->CorrelationType ){
		case Correlator::CORRELATIONWINDOWTYPE::ANALOG:
			this->CorrelatorName = "Correlator_ANALOG";
			break;
		case Correlator::CORRELATIONWINDOWTYPE::ROLLINGTRIGGER:
			this->CorrelatorName = "Correlator_ROLLING_TRIGGER";
			break;
		case Correlator::CORRELATIONWINDOWTYPE::ROLLINGWIDTH:
			this->CorrelatorName = "Correlator_ROLLING_WINDOW";
			break;
		default:
			this->CorrelatorName = "Correlator_UNKNOWN";
			break;
	}

	this->console = spdlog::get(this->LogName)->clone(this->CorrelatorName);
	this->console->info("Created Correlator [{}]",this->CorrelatorName);
}

void Correlator::DumpSelf() const{
	this->console->debug("{}",*this);
}

void Correlator::AddTriggerChannel(int crateID,int moduleID,int channelID){
	this->Triggers.insert({.Crate = crateID, .Module = moduleID, .Channel = channelID});	
}

bool Correlator::IsWithinCorrelationWindow(double ts,int crateID,int moduleID,int channelID){
	switch( this->CorrelationType ){
		case ROLLINGWIDTH:
			return this->RollingWindowCorrelation(ts);
			break;
		case ROLLINGTRIGGER:
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

void Correlator::Pop(){
	if( this->CorrelationType == Correlator::CORRELATIONWINDOWTYPE::ROLLINGTRIGGER ){
		this->MinHeap.pop();
	}
	if( this->CorrelationType == Correlator::CORRELATIONWINDOWTYPE::ROLLINGWIDTH ){
		this->MaxHeap.pop();
	}
	#ifdef CORRELATOR_DEBUG
	if( !this->gMaxHeap.empty() ){
		this->gMaxHeap.pop();
	}
	if( !this->gMinHeap.empty() ){
		this->gMinHeap.pop();
	}
	#endif
}

void Correlator::Clear(){
	if( this->CorrelationType == Correlator::CORRELATIONWINDOWTYPE::ROLLINGTRIGGER ){
		this->MinHeap.clear();
	}
	if( this->CorrelationType == Correlator::CORRELATIONWINDOWTYPE::ROLLINGWIDTH ){
		this->MaxHeap.clear();
	}
	#ifdef CORRELATOR_DEBUG
	if( !this->gMinHeap.empty() ){
		this->gMinHeap.clear();
	}
	if( !this->gMaxHeap.empty() ){
		this->gMaxHeap.clear();
	}
	#endif
}

bool Correlator::RollingTriggerCorrelation(double ts){
	#ifdef CORRELATOR_DEBUG
	this->gMinHeap.push(ts);
	this->gMaxHeap.push(ts);
	#endif
	if( this->MinHeap.empty() ){
		this->MinHeap.push(ts);
		return true;
	}else{
		if( std::abs(ts - this->MinHeap.top()) < this->Width ){
			this->MinHeap.push(ts);
			return true;
		}else{
			#ifdef CORRELATOR_DEBUG
				this->console->debug("Found TS: {:.15f} which is outside the correlation window of {} ns, top before clearing is {:.1f}, delta is {} ns",ts,this->Width,this->MinHeap.top(),std::abs(ts - this->MinHeap.top()));
			#endif
			this->MinHeap.push(ts);
			return false;
		}
	}
}

bool Correlator::RollingWindowCorrelation(double ts){
	#ifdef CORRELATOR_DEBUG
	this->gMinHeap.push(ts);
	this->gMaxHeap.push(ts);
	#endif
	if( this->MaxHeap.empty() ){
		this->MaxHeap.push(ts);
		return true;
	}else{
		if( std::abs(ts - this->MaxHeap.top()) < this->Width ){
			this->MaxHeap.push(ts);
			return true;
		}else{
			#ifdef CORRELATOR_DEBUG
				this->console->debug("Found TS: {:.15f} which is outside the correlation window of {} ns, top before clearing is {:.1f}, delta is {} ns",ts,this->Width,this->MaxHeap.top(),std::abs(ts - this->MaxHeap.top()));
			#endif
			this->MaxHeap.push(ts);
			return false;
		}
	}
}
