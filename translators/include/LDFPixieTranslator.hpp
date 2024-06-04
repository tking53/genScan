#ifndef __LDF_PIXIE_TRANSLATOR_HPP__
#define __LDF_PIXIE_TRANSLATOR_HPP__

#include <string>
#include <map>

#include <boost/container/devector.hpp>

#include "Translator.hpp"

#include "PhysicsData.hpp"

class LDFPixieTranslator : public Translator{
	public:
		LDFPixieTranslator(const std::string&,const std::string&);
		~LDFPixieTranslator();
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

		struct HRIBF_HEAD_Buffer{
			unsigned int bufftype;
			unsigned int buffsize;
			char facility[9];
			char format[9];
			char type[17];
			char date[17];
			char run_title[81];
			unsigned int run_num;
		};

		struct HRIBF_DATA_Buffer{
			unsigned int bufftype;
			unsigned int buffsize;
			unsigned int bcount;
			unsigned int buffhead;
			unsigned int nextbuffhead;
			unsigned int nextbuffsize;
			unsigned int goodchunks;
			unsigned int missingchunks;
			unsigned int numbytes;
			unsigned int numchunks;
			unsigned int currchunknum;
			unsigned int prevchunknum;
			unsigned int buffpos;
			std::vector<unsigned int>* currbuffer;
			std::vector<unsigned int>* nextbuffer;
			std::vector<unsigned int> buffer1;
			std::vector<unsigned int> buffer2;

			unsigned int operator[](unsigned int pos){
				return this->currbuffer->at(pos);
			}
		};

	private:
		unsigned int check_bufftype;
		unsigned int check_buffsize;	
		
		HRIBF_DIR_Buffer CurrDirBuff;
		int ParseDirBuffer();

		HRIBF_HEAD_Buffer CurrHeadBuff;
		int ParseHeadBuffer();

		HRIBF_DATA_Buffer CurrDataBuff;
		int ReadNextBuffer(bool force = false);
		int ParseDataBuffer(unsigned int&,bool&,bool&);

		int UnpackData(unsigned int&,bool&,bool&);
		int CountBuffersWithData() const;

		std::vector<unsigned int> databuffer;

		unsigned int CurrHeaderLength;
		unsigned int CurrTraceLength;
		uint32_t* firstWords;
		uint32_t* otherWords;

		uint64_t PrevTimeStamp;
		bool FinishedReadingFiles;

		const int NUMCONCURRENTSPILLS = 3;
		std::vector<int> EvtSpillCounter;

		//std::map<uint64_t,int> EvtSpillCounter;
		//Increment when we find spill footer
		uint64_t CurrSpillID;
};

#endif
