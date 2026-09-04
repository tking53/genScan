#ifndef __COMPASS_BIN_TIME_SORTED_TRANSLATOR_HPP__
#define __COMPASS_BIN_TIME_SORTED_TRANSLATOR_HPP__

#include <memory>
#include <string>

#include <boost/container/devector.hpp>

#include "BitDecoder.hpp"
#include "Translator.hpp"

#include "PhysicsData.hpp"

class CompassBinTimeSortedTranslator : public Translator {
public:
	CompassBinTimeSortedTranslator(const std::string&, const std::string&);
	~CompassBinTimeSortedTranslator();
	Translator::TRANSLATORSTATE Parse(boost::container::devector<PhysicsData>&);
	virtual void FinalizeFiles(); 

private:
	std::unique_ptr<CaenDecoder> single_file_decoder;
	int ReadNext(boost::container::devector<PhysicsData>&);
	uint16_t firstWords[6];
	uint16_t secondWords[6]; // this is the most we can read based on the 0, 1, 2 bits
	uint8_t thirdWords[5]; // this is what we read based on 3 bit, need to merge back into values we want
	uint64_t PrevTimeStamp;
};

#endif
