#ifndef __EVT_TRANSLATOR_HPP__
#define __EVT_TRANSLATOR_HPP__

#include <string>

#include "Translator.hpp"

class EVTTranslator : public Translator{
	public:
		enum EVT_TYPE{
			PRESORT,
			EVTBUILT
		};
		EVTTranslator(const std::string&,const std::string&,EVT_TYPE);
		~EVTTranslator() = default;
		void Parse();
	private:
		EVT_TYPE Format;
		void ParsePresort();
		void ParseEVTBuilt();
};

#endif
