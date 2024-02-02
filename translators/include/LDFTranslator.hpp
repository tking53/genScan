#ifndef __LDF_TRANSLATOR_HPP__
#define __LDF_TRANSLATOR_HPP__

#include <string>

#include <boost/container/devector.hpp>

#include "Translator.hpp"

#include "PhysicsData.hpp"

class LDFTranslator : public Translator{
	public:
		enum LDF_TYPE{
			PIXIE
		};
		LDFTranslator(const std::string&,const std::string&,LDF_TYPE);
		~LDFTranslator() = default;
		void Parse(boost::container::devector<PhysicsData>&);
	private:
		LDF_TYPE Format;
		void ParsePixie(boost::container::devector<PhysicsData>&);
};

#endif
