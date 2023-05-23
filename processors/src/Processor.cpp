#include <stdexcept>

#include "Processor.hpp"

Processor::Processor() : ProcessorName("Generic"){
}

Processor::Processor(std::string name) : ProcessorName(name){
}

bool Processor::PreProcess(){
	throw std::runtime_error("Called Processor::PreProcess()");
}

bool Processor::Process(){
	throw std::runtime_error("Called Processor::Process()");
}

bool Processor::PostProcess(){
	throw std::runtime_error("Called Processor::PostProcess()");
}
