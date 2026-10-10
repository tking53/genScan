#ifndef __EVTTO_TRANSLATOR_HPP__
#define __EVTTO_TRANSLATOR_HPP__

#include <string>

#include <boost/container/devector.hpp>

#include "Translator.hpp"

#include "PhysicsData.hpp"

class EVTTOTranslator : public Translator {
public:
	EVTTOTranslator(const std::string&, const std::string&);
	~EVTTOTranslator();
	Translator::TRANSLATORSTATE Parse(boost::container::devector<PhysicsData>&);

private:
	unsigned int CurrHeaderLength;
	unsigned int CurrTraceLength;
	uint32_t firstWords[4];
	uint32_t otherWords[12];

	uint64_t PrevTimeStamp;

	int ReadHeader(boost::container::devector<PhysicsData>&);
	int ReadFull(boost::container::devector<PhysicsData>&);
};

#endif
