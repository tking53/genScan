#include "AnalogCorrelator.hpp"

AnalogCorrelator::AnalogCorrelator(const std::string& log,double eventwidth) : Correlator(log,"Analog_Correlator",eventwidth){
}

bool AnalogCorrelator::IsWithinCorrelationWindow(const double& ts,const int& crateID,const int& moduleID,const int& channelID){
	AnalogTrigger trigtest = {.Crate = crateID, .Module = moduleID, .Channel = channelID};
	return (this->Triggers.find(trigtest) != this->Triggers.end());
}

void AnalogCorrelator::DumpSelf() const{
	this->console->debug("{}",*this);
}
