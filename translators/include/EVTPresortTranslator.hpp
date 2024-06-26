#ifndef __EVTPresort_TRANSLATOR_HPP__
#define __EVTPresort_TRANSLATOR_HPP__

#include <string>

#include <boost/container/devector.hpp>

#include "Translator.hpp"

#include "PhysicsData.hpp"

class EVTPresortTranslator : public Translator{
	public:
		EVTPresortTranslator(const std::string&,const std::string&);
		~EVTPresortTranslator() = default;
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
		
		EVT_BUILT_INFO CurrEVTPresortBuiltInfo;

		int ReadRingItemHeader();
		int ReadRingItemBodyHeader();
		int ReadRingItemBody();
		int FindNextFragment();
		int ReadNextFragment();
		int ReadHeader(boost::container::devector<PhysicsData>&);
		int ReadFull(boost::container::devector<PhysicsData>&);

		int ReadPresortHelper(boost::container::devector<PhysicsData>&);
};

#endif
