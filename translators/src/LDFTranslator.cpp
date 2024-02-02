#include <stdexcept>

#include "LDFTranslator.hpp"

LDFTranslator::LDFTranslator(const std::string& log,const std::string& translatorname,LDF_TYPE formattype) : Translator(log,translatorname){
	this->Format = formattype;
}	

void LDFTranslator::Parse(boost::container::devector<PhysicsData>& RawEvents){
	switch(this->Format){
		case PIXIE:
			this->ParsePixie(RawEvents);
			break;
		default:
			throw std::runtime_error("Unknown LDF File type, not PIXIE");
			break;
	}
}

void LDFTranslator::ParsePixie(boost::container::devector<PhysicsData>& RawEvents){
}
