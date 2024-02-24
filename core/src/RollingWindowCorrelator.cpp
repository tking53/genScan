#include "RollingWindowCorrelator.hpp"
#include "Correlator.hpp"
#include <stdexcept>
#include <iostream>

RollingWindowCorrelator::RollingWindowCorrelator(const std::string& log,double eventwidth) : Correlator(log,"RollingWindow_Correlator",eventwidth) {
}


void RollingWindowCorrelator::Pop(){
	this->MaxHeap.pop();
	#ifdef CORRELATOR_DEBUG
	if( !this->gMaxHeap.empty() ){
		this->gMaxHeap.pop();
	}
	if( !this->gMinHeap.empty() ){
		this->gMinHeap.pop();
	}
	#endif
}

void RollingWindowCorrelator::Clear(){
	this->MaxHeap.clear();
	#ifdef CORRELATOR_DEBUG
	if( !this->gMinHeap.empty() ){
		this->gMinHeap.clear();
	}
	if( !this->gMaxHeap.empty() ){
		this->gMaxHeap.clear();
	}
	#endif
}

bool RollingWindowCorrelator::IsWithinCorrelationWindow(const double& ts,const int& crateID,const int& moduleID,const int& channelID){
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
				this->console->debug("Found TS: {:.1f} which is outside the correlation window of {} ns, top before clearing is {:.1f}, delta is {} ns",ts,this->Width,this->MaxHeap.top(),std::abs(ts - this->MaxHeap.top()));
			#endif
			this->MaxHeap.push(ts);
			return false;
		}
	}
}

void RollingWindowCorrelator::DumpSelf() const{
	this->console->debug("{}",*this);
}
