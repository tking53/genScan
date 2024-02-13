#ifndef __EVT_TRANSLATOR_HPP__
#define __EVT_TRANSLATOR_HPP__

#include <string>

#include <boost/container/devector.hpp>

#include "Translator.hpp"

#include "PhysicsData.hpp"

class EVTTranslator : public Translator{
	public:
		enum EVT_TYPE{
			PRESORT,
			EVTBUILT
		};
		EVTTranslator(const std::string&,const std::string&,EVT_TYPE);
		~EVTTranslator() = default;
		Translator::TRANSLATORSTATE Parse(boost::container::devector<PhysicsData>&);
	private:
		struct EVT_BUILT_INFO{
			int rib_size;
			int ri_size;
			int ri_type;
		};

		unsigned int CurrHeaderLength;
		unsigned int CurrTraceLength;
		uint32_t firstWords[4];
		
		EVT_TYPE Format;

		EVT_BUILT_INFO CurrEVTBuiltInfo;

		Translator::TRANSLATORSTATE ParsePresort(boost::container::devector<PhysicsData>&);
		
		Translator::TRANSLATORSTATE ParseEVTBuilt(boost::container::devector<PhysicsData>&);

		int ReadRingItemHeader();
		int ReadRingItemBodyHeader();
		int ReadRingItemBody();
		int FindNextFragment();
		int ReadNextFragment();
		int ReadHeader(boost::container::devector<PhysicsData>&);
		int ReadFull(boost::container::devector<PhysicsData>&);
};

#endif
