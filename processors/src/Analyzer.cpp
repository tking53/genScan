#include <stdexcept>

#include "Analyzer.hpp"

Analyzer::Analyzer() : AnalyzerName("Generic"){
}

Analyzer::Analyzer(std::string name) : AnalyzerName(name){
}

bool Analyzer::PreProcess(){
	throw std::runtime_error("Called Analyzer::PreProcess()");
}

bool Analyzer::Process(){
	throw std::runtime_error("Called Analyzer::Process()");
}

bool Analyzer::PostProcess(){
	throw std::runtime_error("Called Analyzer::PostProcess()");
}
