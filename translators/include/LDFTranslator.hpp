#ifndef __LDF_TRANSLATOR_HPP__
#define __LDF_TRANSLATOR_HPP__

#include <string>

#include "Translator.hpp"

class LDFTranslator : public Translator{
	public:
		enum LDF_TYPE{
			PIXIE
		};
		LDFTranslator(const std::string&,const std::string&,LDF_TYPE);
		~LDFTranslator() = default;
		void Parse();
	private:
		LDF_TYPE Format;
		void ParsePixie();
};

#endif
