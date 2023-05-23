#include "GenericProcessor.hpp"

GenericProcessor::GenericProcessor() : Processor("GenericProcessor"){
}

bool GenericProcessor::PreProcess(){
	return true;
}

bool GenericProcessor::Process(){
	return true;
}

bool GenericProcessor::PostProcess(){
	return true;
}
