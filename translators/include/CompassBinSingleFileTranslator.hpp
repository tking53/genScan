#ifndef __COMPASS_BIN_SINGLE_FILE_TRANSLATOR_HPP__
#define __COMPASS_BIN_SINGLE_FILE_TRANSLATOR_HPP__

#include <string>

#include <boost/container/devector.hpp>

#include "BitDecoder.hpp"
#include "Translator.hpp"

#include "PhysicsData.hpp"

class CompassBinSingleFileTranslator : public Translator {
public:
	CompassBinSingleFileTranslator(const std::string&, const std::string&);
	~CompassBinSingleFileTranslator();
	Translator::TRANSLATORSTATE Parse(boost::container::devector<PhysicsData>&);
	virtual void FinalizeFiles(); 

private:
	CaenDecoder single_file_decoder;
	CaenDecoder DecodeSingleFileHeader(const std::string&) const;
};

#endif
