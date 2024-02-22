#include <stdexcept>

#include "BitDecoder.hpp"
#include "EVTPresortTranslator.hpp"
#include "Translator.hpp"

EVTPresortTranslator::EVTPresortTranslator(const std::string& log,const std::string& translatorname) : Translator(log,translatorname){
	this->CurrEVTPresortBuiltInfo = { .rib_size = 0, .ri_size = 0, .ri_type = 0};
}	

Translator::TRANSLATORSTATE EVTPresortTranslator::Parse(boost::container::devector<PhysicsData>& RawEvents){
	if( this->FinishedCurrentFile ){
		if( !this->OpenNextFile() ){
			return Translator::TRANSLATORSTATE::COMPLETE;
		}
	}
	Translator::TRANSLATORSTATE currstate = Translator::TRANSLATORSTATE::UNKNOWN;
	return currstate;
}
