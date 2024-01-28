#include <stdexcept>

#include "LDFTranslator.hpp"

LDFTranslator::LDFTranslator(const std::string& log,const std::string& translatorname,LDF_TYPE formattype) : Translator(log,translatorname){
	this->Format = formattype;
}	

void LDFTranslator::Parse(){
	switch(this->Format){
		case PIXIE:
			this->ParsePixie();
			break;
		default:
			throw std::runtime_error("Unknown LDF File type, not PIXIE");
			break;
	}
}

void LDFTranslator::ParsePixie(){
}
