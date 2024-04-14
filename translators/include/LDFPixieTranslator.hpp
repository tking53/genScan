#ifndef __LDF_PIXIE_TRANSLATOR_HPP__
#define __LDF_PIXIE_TRANSLATOR_HPP__

#include <string>

#include <boost/container/devector.hpp>

#include "Translator.hpp"

#include "PhysicsData.hpp"

class LDFPixieTranslator : public Translator{
	public:
		LDFPixieTranslator(const std::string&,const std::string&);
		~LDFPixieTranslator() = default;
		Translator::TRANSLATORSTATE Parse(boost::container::devector<PhysicsData>&);

		enum HRIBF_TYPES{
			HEAD = 1145128264,
			DATA = 1096040772,
			SCAL = 1279345491,
			DEAD = 1145128260,
			DIR = 542263620,
			PAC = 541278544,
			ENDFILE = 541478725,
			ENDBUFF = 0xFFFFFFFF
		};

		struct HRIBF_DIR_Buffer{
			unsigned int bufftype;
			unsigned int buffsize;
			unsigned int totalbuffsize;
			unsigned int unknown[3];
			unsigned int run_num;
		};
		//struct HRIBF_HEAD_Buffer{
		//};
		//struct HRIBF_DATA_Buffer{
		//};

	private:
		unsigned int check_bufftype;
		unsigned int check_buffsize;	
		
		HRIBF_DIR_Buffer CurrDirBuff;
		int ParseDirBuffer();

		int ParseHeadBuffer();

		int ParseDataBuffer(boost::container::devector<PhysicsData>&);

		unsigned int CurrHeaderLength;
		unsigned int CurrTraceLength;
		uint32_t firstWords[4];
		uint32_t otherWords[12];

		uint64_t PrevTimeStamp;

		std::vector<int> EvtSpillCounter;
		int CurrSpillID;
};

#endif
