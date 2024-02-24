#include "Correlator.hpp"
#include <stdexcept>

Correlator::Correlator(const std::string& log,const std::string& corname,double eventwidth){
	this->Width = eventwidth;
	this->LogName = log;
	this->CorrelatorName = corname;
	this->console = spdlog::get(this->LogName)->clone(this->CorrelatorName);
}

[[noreturn]] void Correlator::DumpSelf() const{
	this->console->error("Called Correlator::DumpSelf, not the overload");
	throw std::runtime_error("Called Correlator::DumpSelf, not the overload");
}

void Correlator::AddTriggerChannel(const int& crateID,const int& moduleID,const int& channelID){
	this->Triggers.insert({.Crate = crateID, .Module = moduleID, .Channel = channelID});	
}

[[noreturn]] bool Correlator::IsWithinCorrelationWindow(const double& ts,const int& crateID,const int& moduleID,const int& channelID){
	this->console->error("Called Correlator::IsWithinCorrelationWindow, not the overload");
	throw std::runtime_error("Called Correlator::IsWithinCorrelationWindow, not the overload");
}

[[noreturn]] void Correlator::Pop(){
	this->console->error("Called Correlator::Pop, not the overload");
	throw std::runtime_error("Called Correlator::Pop, not the overload");
}

[[noreturn]] void Correlator::Clear(){
	this->console->error("Called Correlator::Clear, not the overload");
	throw std::runtime_error("Called Correlator::Clear, not the overload");
}
