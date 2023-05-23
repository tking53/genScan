#include "GenericAnalyzer.hpp"

GenericAnalyzer::GenericAnalyzer() : Analyzer("GenericAnalyzer"){
}

bool GenericAnalyzer::PreProcess(){
	return true;
}

bool GenericAnalyzer::Process(){
	return true;
}

bool GenericAnalyzer::PostProcess(){
	return true;
}
