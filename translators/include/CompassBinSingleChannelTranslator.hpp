#ifndef __COMPASS_BIN_SINGLE_CHANNEL_TRANSLATOR_HPP__
#define __COMPASS_BIN_SINGLE_CHANNEL_TRANSLATOR_HPP__

#include <map>
#include <string>

#include <boost/container/devector.hpp>
#include <tuple>
#include <utility>
#include <vector>

#include "BitDecoder.hpp"
#include "Translator.hpp"

#include "PhysicsData.hpp"

class CompassBinSingleChannelTranslator : public Translator {
public:
	CompassBinSingleChannelTranslator(const std::string&, const std::string&);
	~CompassBinSingleChannelTranslator();
	Translator::TRANSLATORSTATE Parse(boost::container::devector<PhysicsData>&);
	virtual void FinalizeFiles(); 

private:
	using CaenDigitizerKey = std::tuple<int,std::string,int>;
	using CaenDigitizerValue = std::pair<CaenDecoder, std::vector<std::string>>;
	std::map<CaenDigitizerKey, CaenDigitizerValue> CaenMapping;
	CaenDecoder DecodeSingleFileHeader(const std::string&) const;
};

#endif
