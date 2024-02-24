#include "RollingTriggerCorrelator.hpp"

RollingTriggerCorrelator::RollingTriggerCorrelator(const std::string& log,double eventwidth) : Correlator(log,"RollingTrigger_Correlator",eventwidth){
}


void RollingTriggerCorrelator::Pop(){
	this->MinHeap.pop();
	#ifdef CORRELATOR_DEBUG
	if( !this->gMaxHeap.empty() ){
		this->gMaxHeap.pop();
	}
	if( !this->gMinHeap.empty() ){
		this->gMinHeap.pop();
	}
	#endif
}

void RollingTriggerCorrelator::Clear(){
	this->MinHeap.clear();
	#ifdef CORRELATOR_DEBUG
	if( !this->gMinHeap.empty() ){
		this->gMinHeap.clear();
	}
	if( !this->gMaxHeap.empty() ){
		this->gMaxHeap.clear();
	}
	#endif
}

bool RollingTriggerCorrelator::IsWithinCorrelationWindow(const double& ts,const int& crateID,const int& moduleID,const int& channelID){
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
				this->console->debug("Found TS: {:.1f} which is outside the correlation window of {} ns, top before clearing is {:.1f}, delta is {} ns",ts,this->Width,this->MinHeap.top(),std::abs(ts - this->MinHeap.top()));
			#endif
			this->MinHeap.push(ts);
			return false;
		}
	}
}

void RollingTriggerCorrelator::DumpSelf() const{
	this->console->debug("{}",*this);
}
