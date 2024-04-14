#include <stdexcept>

#include "LDFTranslator.hpp"
#include "Translator.hpp"

LDFTranslator::LDFTranslator(const std::string& log,const std::string& translatorname,LDF_TYPE formattype) : Translator(log,translatorname){
	this->Format = formattype;
}	

Translator::TRANSLATORSTATE LDFTranslator::Parse(boost::container::devector<PhysicsData>& RawEvents){
	switch(this->Format){
		case PIXIE:
			this->ParsePixie(RawEvents);
			break;
		default:
			throw std::runtime_error("Unknown LDF File type, not PIXIE");
			break;
	}
	return Translator::TRANSLATORSTATE::PARSING;
}

Translator::TRANSLATORSTATE LDFTranslator::ParsePixie(boost::container::devector<PhysicsData>& RawEvents){
	return Translator::TRANSLATORSTATE::PARSING;
}
