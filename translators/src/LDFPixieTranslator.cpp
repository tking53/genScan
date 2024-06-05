#include <cstdint>
#include <cstdlib>
#include <stdexcept>

#include <boost/sort/pdqsort/pdqsort.hpp>
#include <boost/sort/spinsort/spinsort.hpp>

#include "LDFPixieTranslator.hpp"
#include "BitDecoder.hpp"
#include "PhysicsData.hpp"
#include "Translator.hpp"
#include "boost/move/iterator.hpp"

LDFPixieTranslator::LDFPixieTranslator(const std::string& log,const std::string& translatorname) : Translator(log,translatorname){
	this->PrevTimeStamp = 0;
	this->CurrSpillID = 0;
	this->EvtSpillCounter = std::vector<int>(this->NUMCONCURRENTSPILLS,0);
	this->FinishedReadingFiles = false;
	this->CurrDirBuff = { 
		.bufftype = HRIBF_TYPES::DIR, 
		.buffsize = 8192, 
		.totalbuffsize = 0, 
		.unknown = { 0, 1, 2}, 
		.run_num = 0 
	};
	this->CurrHeadBuff = { 
		.bufftype = HRIBF_TYPES::HEAD, 
		.buffsize = 64, 
		.facility = {'N','U','L','L','\0'}, 
		.format = {'N','U','L','L','\0'}, 
		.type = {'N','U','L','L','\0'},
		.date = {'N','U','L','L','\0'},
		.run_title = {'N','U','L','L','\0'},
		.run_num = 0
	}; 
	this->CurrDataBuff = {
		.bufftype = HRIBF_TYPES::DATA,
		.buffsize = 8192,
		.bcount = 0,
		.buffhead = 0,
		.nextbuffhead = 0,
		.nextbuffsize = 0,
		.goodchunks = 0,
		.missingchunks = 0,
		.numbytes = 0,
		.numchunks = 0,
		.currchunknum = 0,
		.prevchunknum = 0,
		.buffpos = 0,
		.currbuffer = nullptr,
		.nextbuffer = nullptr,
		.buffer1 = std::vector<unsigned int>(8194,0xFFFFFFFF),
		.buffer2 = std::vector<unsigned int>(8194,0xFFFFFFFF)
	};
	this->NTotalWords = 0;
}	

LDFPixieTranslator::~LDFPixieTranslator(){
	this->console->info("good chunks : {}, bad chunks : {}, spills : {}",this->CurrDataBuff.goodchunks,this->CurrDataBuff.missingchunks,this->CurrSpillID);
	if( not this->Leftovers.empty() ){
		this->console->critical("Leftover Events : {}",this->Leftovers.size());
	}
}

Translator::TRANSLATORSTATE LDFPixieTranslator::Parse(boost::container::devector<PhysicsData>& RawEvents){
	if( this->FinishedCurrentFile ){
		if( this->OpenNextFile() ){
			if( this->ParseDirBuffer() == -1 ){
				throw std::runtime_error("Invalid Dir Buffer when opening file : "+this->InputFiles.at(this->CurrentFileIndex));
			}
			if( this->ParseHeadBuffer() == -1 ){
				throw std::runtime_error("Invalid Head Buffer when opening file : "+this->InputFiles.at(this->CurrentFileIndex));
			}
			this->CurrDataBuff.bcount = 0;
		}else{
			this->FinishedReadingFiles = true;
		}
	}
	bool entriesread = false;
	while( not this->FinishedReadingFiles and (this->CountBuffersWithData() < this->NUMCONCURRENTSPILLS) ){
		if( this->CurrentFile.eof() ){
			if( this->OpenNextFile() ){
				if( this->ParseDirBuffer() == -1 ){
					throw std::runtime_error("Invalid Dir Buffer when opening file : "+this->InputFiles.at(this->CurrentFileIndex));
				}
				if( this->ParseHeadBuffer() == -1 ){
					throw std::runtime_error("Invalid Head Buffer when opening file : "+this->InputFiles.at(this->CurrentFileIndex));
				}
				this->CurrDataBuff.bcount = 0;
			}else{
				this->FinishedReadingFiles = true;
			}
		}
		if( this->FinishedReadingFiles ){
			break;
		}

		entriesread = true;
		bool full_spill;
		bool bad_spill;
		unsigned int nBytes = 0;
		int retval = this->ParseDataBuffer(nBytes,full_spill,bad_spill);
		if( retval == -1 ){
			throw std::runtime_error("Invalid Data Buffer in File : "+this->InputFiles.at(this->CurrentFileIndex));
		}
		if( full_spill ){
			this->UnpackData(nBytes,full_spill,bad_spill);
		}
	}
	if( entriesread ){
		boost::sort::pdqsort(this->Leftovers.begin(),this->Leftovers.end());
		//boost::sort::spinsort(this->Leftovers.begin(),this->Leftovers.end());
	}
	if( not this->Leftovers.empty() ){
		size_t stopidx = 0;
		for( const auto& evt : this->Leftovers ){
			this->LastReadEvtWithin = this->correlator->IsWithinCorrelationWindow(evt.GetTimeStamp(),evt.GetCrate(),evt.GetModule(),evt.GetChannel());
			if( not this->LastReadEvtWithin ){
				this->correlator->Pop();
				this->correlator->Clear();
				break;
			}else{
				this->EvtSpillCounter[evt.GetSpillID()%this->NUMCONCURRENTSPILLS] -= 1;
				++stopidx;
			}
		}
		RawEvents.insert(RawEvents.end(),boost::make_move_iterator(this->Leftovers.begin()),boost::make_move_iterator(this->Leftovers.begin()+stopidx));
		this->Leftovers.erase(this->Leftovers.begin(),this->Leftovers.begin()+stopidx);
		if( not this->Leftovers.empty() ){
			return Translator::TRANSLATORSTATE::PARSING;
		}else{
			return Translator::TRANSLATORSTATE::COMPLETE;
		}
	}else{
		return Translator::TRANSLATORSTATE::COMPLETE;
	}
}

int LDFPixieTranslator::ParseDirBuffer(){
	this->CurrentFile.read(reinterpret_cast<char*>(&(this->check_bufftype)),sizeof(unsigned int));
	this->CurrentFile.read(reinterpret_cast<char*>(&(this->check_buffsize)),sizeof(unsigned int));
	if( this->check_bufftype != this->CurrDirBuff.bufftype or this->check_buffsize != this->CurrDirBuff.buffsize ){
		this->console->warn("Invalid DIR buffer");
		this->CurrentFile.seekg(-8,this->CurrentFile.cur);
		return -1;
	}	
	this->CurrentFile.read(reinterpret_cast<char*>(&(this->CurrDirBuff.totalbuffsize)),sizeof(unsigned int));
	this->CurrentFile.read(reinterpret_cast<char*>(&(this->CurrDirBuff.totalbuffsize)),sizeof(unsigned int));
	this->CurrentFile.read(reinterpret_cast<char*>(&(this->CurrDirBuff.unknown)),2*(sizeof(unsigned int)));
	this->CurrentFile.read(reinterpret_cast<char*>(&(this->CurrDirBuff.run_num)),sizeof(unsigned int));
	this->CurrentFile.seekg(this->CurrDirBuff.buffsize*4 - 20,this->CurrentFile.cur);
	this->console->info("Parsed Dir Buffer");
	this->console->info("found total buff size : {}",this->CurrDirBuff.totalbuffsize);
	this->console->info("unknown [0-2] : {} {} {}",this->CurrDirBuff.unknown[0],this->CurrDirBuff.unknown[1],this->CurrDirBuff.unknown[2]);
	this->console->info("runnum : {}",this->CurrDirBuff.run_num);
	return 0;
}

int LDFPixieTranslator::ParseHeadBuffer(){
	this->CurrentFile.read(reinterpret_cast<char*>(&(this->check_bufftype)),sizeof(unsigned int));
	this->CurrentFile.read(reinterpret_cast<char*>(&(this->check_buffsize)),sizeof(unsigned int));
	if( this->check_bufftype != this->CurrHeadBuff.bufftype or this->check_buffsize != this->CurrHeadBuff.buffsize ){
		this->console->warn("Invalid HEAD buffer");
		this->CurrentFile.seekg(-8,this->CurrentFile.cur);
		return -1;
	}	
	this->CurrentFile.read(reinterpret_cast<char*>(&(this->CurrHeadBuff.facility)),8);
	this->CurrHeadBuff.facility[8] = '\0';
	this->CurrentFile.read(reinterpret_cast<char*>(&(this->CurrHeadBuff.format)),8);
	this->CurrHeadBuff.format[8] = '\0';
	this->CurrentFile.read(reinterpret_cast<char*>(&(this->CurrHeadBuff.type)),16);
	this->CurrHeadBuff.type[16] = '\0';
	this->CurrentFile.read(reinterpret_cast<char*>(&(this->CurrHeadBuff.date)),16);
	this->CurrHeadBuff.date[16] = '\0';
	this->CurrentFile.read(reinterpret_cast<char*>(&(this->CurrHeadBuff.run_title)),80);
	this->CurrHeadBuff.run_title[80] = '\0';
	this->CurrentFile.read(reinterpret_cast<char*>(&(this->CurrHeadBuff.run_num)),sizeof(unsigned int));
	this->CurrentFile.seekg(8194*4 - 140,this->CurrentFile.cur);
	this->console->info("Found Head Buffer");
	this->console->info("facility : {}",this->CurrHeadBuff.facility);
	this->console->info("format : {}",this->CurrHeadBuff.format);
	this->console->info("type : {}",this->CurrHeadBuff.type);
	this->console->info("date : {}",this->CurrHeadBuff.date);
	this->console->info("title : {}",this->CurrHeadBuff.run_title);
	this->console->info("run : {}",this->CurrHeadBuff.run_num);

	return 0;
}

int LDFPixieTranslator::ParseDataBuffer(unsigned int& nBytes,bool& full_spill,bool& bad_spill){
	bool first_chunk = true;
	bad_spill = false;
	unsigned int this_chunk_sizeB;
	unsigned int total_num_chunks = 0;
	unsigned int current_chunk_num = 0;
	unsigned int prev_chunk_num;
	unsigned int prev_num_chunks;
	nBytes = 0;

	while( true ){
		if( this->ReadNextBuffer() == -1 and (this->CurrDataBuff.buffhead != HRIBF_TYPES::ENDFILE) ){
			this->console->critical("Failed to read from input data file");
			return 6;
		}
		//this->ReadNextBuffer(true);
		if( this->CurrDataBuff.buffhead == HRIBF_TYPES::ENDFILE ){
			if( this->CurrDataBuff.nextbuffhead == HRIBF_TYPES::ENDFILE ){
				this->console->info("Read double EOF");
				return 2;
			}else{
				this->console->info("Reached single EOF, force reading next");
				this->ReadNextBuffer(true);
				return 1;
			}
		}else if( this->CurrDataBuff.buffhead == HRIBF_TYPES::DATA ){
			prev_chunk_num = current_chunk_num;
			prev_num_chunks = total_num_chunks;
			this_chunk_sizeB = this->CurrDataBuff.currbuffer->at(this->CurrDataBuff.buffpos++);
			total_num_chunks = this->CurrDataBuff.currbuffer->at(this->CurrDataBuff.buffpos++);
			current_chunk_num = this->CurrDataBuff.currbuffer->at(this->CurrDataBuff.buffpos++);

			if( first_chunk ){
				if( current_chunk_num != 0 ){
					this->console->critical("first chunk {} isn't chunk 0 at spill {}",current_chunk_num,this->CurrSpillID);
					this->CurrDataBuff.missingchunks += current_chunk_num;
					full_spill = false;
				}else{
					full_spill = true;
				}
				first_chunk = false;
			}else if( total_num_chunks != prev_num_chunks ){
				this->console->critical("Gotten out of order parsing spill {}",this->CurrSpillID);
				this->ReadNextBuffer(true);
				this->CurrDataBuff.missingchunks += (prev_num_chunks - 1) - prev_chunk_num;
				return 4; 
			}else if( current_chunk_num != prev_chunk_num+1 ){
				full_spill = false;
				if( current_chunk_num == prev_chunk_num+2 ){
					this->console->critical("Missing single spill chunk {} at spill {}",prev_chunk_num+1,this->CurrSpillID);
				}else{
					this->console->critical("Missing multiple spill chunks from {} to {} at spill {}",prev_chunk_num+1,current_chunk_num-1,this->CurrSpillID);
				}
				this->ReadNextBuffer(true);
				this->CurrDataBuff.missingchunks += std::abs(static_cast<double>(static_cast<double>(current_chunk_num - 1) - prev_chunk_num));
				return 4;
			}

			if( current_chunk_num == total_num_chunks - 1) {//spill footer
				if( this_chunk_sizeB != 20 ){
					this->console->critical("spill footer (chunk {} of {}) has size {} != 5 at spill {}",current_chunk_num,total_num_chunks,this_chunk_sizeB,this->CurrSpillID);
					this->ReadNextBuffer(true);
					return 5;
				}
				//memcpy(&data_[nBytes],&curr_buffer[buff_pos],8)
				unsigned int nWords = 2;
				for( unsigned int ii = 0; ii < nWords; ++ii ){
					this->databuffer.push_back(this->CurrDataBuff[this->CurrDataBuff.buffpos+ii]);
				}
				nBytes += 8;
				this->CurrDataBuff.buffpos += 2;
				return 0;
			}else{
				unsigned int copied_bytes = 0;
				if( this_chunk_sizeB <= 12 ){
					this->console->critical("invalid number of bytes in chunk {} of {}, {} bytes at spill {}",current_chunk_num+1,total_num_chunks,this_chunk_sizeB,this->CurrSpillID);
					++this->CurrDataBuff.missingchunks;
					return 4;
				}
				++this->CurrDataBuff.goodchunks;
				copied_bytes = this_chunk_sizeB - 12;
				//memcpy(&data_[nBytes],&curr_buffer[buff_pos],copied_bytes);
				unsigned int nWords = copied_bytes/4;
				for( unsigned int ii = 0; ii < nWords; ++ii ){
					this->databuffer.push_back(this->CurrDataBuff[this->CurrDataBuff.buffpos+ii]);
				}
				nBytes += copied_bytes;
				this->CurrDataBuff.buffpos += copied_bytes/4;
			}
		}else{
			this->console->critical("found non data/non eof buffer 0x{:x}",this->CurrDataBuff.buffhead);
			this->ReadNextBuffer(true);
		}
	}
	return 0;
}

int LDFPixieTranslator::ReadNextBuffer(bool force){
	if( this->CurrDataBuff.bcount == 0 ){
		this->CurrentFile.read(reinterpret_cast<char*>(&(this->CurrDataBuff.buffer1[0])),8194*4);
	}else if( this->CurrDataBuff.buffpos + 3 <= 8193 and not force ){
		while( this->CurrDataBuff.currbuffer->at(this->CurrDataBuff.buffpos) == HRIBF_TYPES::ENDBUFF and  this->CurrDataBuff.buffpos < 8193 ){
			++(this->CurrDataBuff.buffpos);
		}
		if( this->CurrDataBuff.buffpos + 3 < 8193 ){
			return 0;
		}
	}
	if( this->CurrDataBuff.bcount % 2 == 0 ){
		this->CurrentFile.read(reinterpret_cast<char*>(&(this->CurrDataBuff.buffer2[0])),8194*4);
		this->CurrDataBuff.currbuffer = &(this->CurrDataBuff.buffer1);
		this->CurrDataBuff.nextbuffer = &(this->CurrDataBuff.buffer2);
	}else{
		this->CurrentFile.read(reinterpret_cast<char*>(&(this->CurrDataBuff.buffer1[0])),8194*4);
		this->CurrDataBuff.currbuffer = &(this->CurrDataBuff.buffer2);
		this->CurrDataBuff.nextbuffer = &(this->CurrDataBuff.buffer1);
	}
	++(this->CurrDataBuff.bcount);
	this->CurrDataBuff.buffpos = 0;
	this->CurrDataBuff.buffhead = this->CurrDataBuff.currbuffer->at((this->CurrDataBuff.buffpos)++);
	this->CurrDataBuff.buffsize = this->CurrDataBuff.currbuffer->at((this->CurrDataBuff.buffpos)++);
	//for( size_t ii = 0; ii < 8194; ++ii ){
	//	this->console->info("{} {:x}",ii,this->CurrDataBuff.currbuffer->at(ii));
	//}
	
	this->CurrDataBuff.nextbuffhead = this->CurrDataBuff.nextbuffer->at(0);
	this->CurrDataBuff.nextbuffsize = this->CurrDataBuff.nextbuffer->at(1);
	if( not this->CurrentFile.good() ){
		return -1;
	}else if( this->CurrentFile.eof() ){
		return 2;
	}

	return 0;
}

int LDFPixieTranslator::UnpackData(unsigned int& nBytes,bool& full_spill,bool& bad_spill){
	unsigned int nWords = nBytes/4;
	unsigned int nWords_read = 0;
	unsigned int lenrec = 0xFFFFFFFF;
	unsigned int vsn = 0xFFFFFFFF;
	auto currsize = this->Leftovers.size();
	time_t theTime = 0;
	this->NTotalWords += nWords;
	while( nWords_read <= nWords ){
		while( this->databuffer[nWords_read] == 0xFFFFFFFF ){
			++nWords_read;
		}
		lenrec = this->databuffer[nWords_read];
		vsn = this->databuffer[nWords_read + 1];
		//this->console->info("lenrec : {} vsn : {}",lenrec,vsn);

		if( lenrec == 6 ){
			nWords_read += lenrec;
			continue;
		}

		if( vsn < 14 ){
			//this->console->info("{} {}",lenrec,vsn);
			if( lenrec == 2 ){
				nWords_read += lenrec;
				continue;
			}else{
				//good module readout
				unsigned int buffpos = nWords_read+2; 
				while( buffpos < (nWords_read + lenrec) ){
					firstWords = &(this->databuffer[buffpos]);

					uint32_t ChannelNumber = PIXIE::ChannelNumberMask(firstWords[0]);
					uint32_t ModuleNumber = (PIXIE::ModuleNumberMask(firstWords[0]))-2;
					//if( vsn > 0 ){
					//	this->console->info("{} {} {}",lenrec,vsn,ModuleNumber);
					//}
					uint32_t CrateNumber = PIXIE::CrateNumberMask(firstWords[0]);
					this->CurrHeaderLength = PIXIE::HeaderLengthMask(firstWords[0]);
					uint32_t FinishCode = (PIXIE::FinishCodeMask(firstWords[0]) != 0);

					try{
						this->CurrDecoder = CMap->GetXiaDecoder(CrateNumber,ModuleNumber);
					}catch(const boost::container::out_of_range& e){
						this->console->error("Ill formed config file, Crate : {} Board : {} does not exist. The next message is what boost reports",CrateNumber,ModuleNumber);
						throw std::runtime_error(e.what());
					}
					uint32_t TimeStampLow;
					uint32_t TimeStampHigh;
					uint32_t EventEnergy;
					bool OutOfRange;
					uint32_t EventLength;

					this->CurrDecoder->DecodeFirstWords(firstWords,EventLength,TimeStampLow,TimeStampHigh,EventEnergy,CurrTraceLength,OutOfRange);

					uint64_t TimeStamp = static_cast<uint64_t>(TimeStampHigh);
					TimeStamp = TimeStamp<<32;
					TimeStamp += TimeStampLow;
					double TimeStampInNS = TimeStamp*(this->CMap->GetModuleClockTicksToNS(CrateNumber,ModuleNumber));
					this->Leftovers.push_back(PhysicsData(CurrHeaderLength,EventLength,CrateNumber,ModuleNumber,ChannelNumber,
								this->CMap->GetGlobalBoardID(CrateNumber,ModuleNumber),
								this->CMap->GetGlobalChanID(CrateNumber,ModuleNumber,ChannelNumber),
								EventEnergy,TimeStamp));
					this->Leftovers.back().SetPileup(FinishCode);
					this->Leftovers.back().SetSaturation(OutOfRange);

					//word2 has CFD things
					uint64_t CFDTimeStamp = this->CurrDecoder->DecodeCFDParams(firstWords,TimeStamp,this->Leftovers.back());
					double CFDTimeStampInNS = CFDTimeStamp*(this->CMap->GetModuleADCClockTicksToNS(CrateNumber,ModuleNumber));

#ifdef TRANSLATOR_DEBUG
					this->console->debug("TS : {}, TS(ns) : {}, CFDTS : {}, CFDTS(ns) : {}",TimeStamp,TimeStampInNS,CFDTimeStamp,CFDTimeStampInNS);
#endif
					//always use the cfd based TimeStampInNS to event build, it is the same other if nothing is set
					this->Leftovers.back().SetTimeStamp(CFDTimeStampInNS);

					if( this->CurrHeaderLength > 4 ){
						otherWords = &(this->databuffer[buffpos+4]);
						this->CurrDecoder->DecodeOtherWords(otherWords,&(this->Leftovers.back()));
					}
					buffpos += this->CurrHeaderLength;

					if( this->CurrTraceLength > 0 ){
						this->Leftovers.back().SetRawTraceLength(this->CurrTraceLength);
						otherWords = &(this->databuffer[buffpos]);
						buffpos += this->CurrTraceLength/2;
						uint16_t* sbuf = reinterpret_cast<uint16_t*>(otherWords);
						std::vector<uint16_t> tmp;
						for( unsigned int ii = 0; ii < this->CurrTraceLength; ++ii ){
							tmp.push_back(sbuf[ii]);	
						}
						this->Leftovers.back().SetRawTrace(tmp);
					}
					this->Leftovers.back().SetSpillID(this->CurrSpillID);
				}
				nWords_read += lenrec;
			}
		}else if( vsn == 1000 ){
			//this is for superheavy
			memcpy(&theTime,&(this->databuffer[nWords_read + 2]),sizeof(time_t));
			this->console->info("ctime : {}",ctime(&theTime));
			nWords_read += lenrec;
			continue;
		}else if( vsn == 9999 ){
			//end of readout
			auto finalsize = this->Leftovers.size();
			this->EvtSpillCounter[this->CurrSpillID%this->NUMCONCURRENTSPILLS] = (finalsize - currsize);
			//this->console->info("evts added {}",(finalsize-currsize));
			this->console->info("spill : {} words : {} Total words : {}",this->CurrSpillID,nWords,this->NTotalWords);
			++(this->CurrSpillID);
			this->databuffer.clear();
			break;
		}else{
			auto finalsize = this->Leftovers.size();
			this->EvtSpillCounter[this->CurrSpillID%this->NUMCONCURRENTSPILLS] = (finalsize - currsize);
			++(this->CurrSpillID);
			this->databuffer.clear();
			this->console->critical("UNEXPECTED VSN : {}",vsn);
			break;
		}
	}
	return 0;
}

int LDFPixieTranslator::CountBuffersWithData() const{
	int numspill = 0;
	for( const auto& itr : this->EvtSpillCounter ){
		if( itr > 0 ){
			++numspill;
		}
	}
	return numspill;
	//if( this->EvtSpillCounter.size() == 0 ){
	//	return 0;
	//}else{
	//	int numspill = 0;
	//	for( const auto& kv : this->EvtSpillCounter ){
	//		if( kv.second > 0 ){
	//			++numspill;
	//			//this->console->info("spill {} : entries {}",kv.first,kv.second);
	//		}
	//	}
	//	//this->console->info("done");
	//	return numspill;
	//}
}
