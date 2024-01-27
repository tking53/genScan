#include <stdexcept>

#include "EVTTranslator.hpp"

EVTTranslator::EVTTranslator(const std::string& log,const std::string& translatorname,EVT_TYPE formattype) : Translator(log,translatorname){
	this->Format = formattype;
}	

void EVTTranslator::Parse(){
	switch(this->Format){
		case PRESORT:
			this->ParsePresort();
			break;
		case EVTBUILT:
			this->ParseEVTBuilt();
			break;
		default:
			throw std::runtime_error("Unknown EVT File type, not EVTBUILT or PRESORT");
			break;
	}
}

void EVTTranslator::ParsePresort(){
}

void EVTTranslator::ParseEVTBuilt(){
}
