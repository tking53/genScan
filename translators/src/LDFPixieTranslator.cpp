#include <cstdlib>
#include <stdexcept>

#include "LDFPixieTranslator.hpp"
#include "Translator.hpp"

LDFPixieTranslator::LDFPixieTranslator(const std::string& log,const std::string& translatorname) : Translator(log,translatorname){
	this->PrevTimeStamp = 0;
	this->EvtSpillCounter = std::vector<uint64_t>(3,0);
	this->CurrSpillID = 0;
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
	this->CurrPixieModBuff = {
		.numwords = 0,
		.modulenum = 9999
	};
}	

Translator::TRANSLATORSTATE LDFPixieTranslator::Parse(boost::container::devector<PhysicsData>& RawEvents){
	if( this->FinishedCurrentFile ){
		if( not this->OpenNextFile() ){
			return Translator::TRANSLATORSTATE::COMPLETE;
		}else{
			if( this->ParseDirBuffer() == -1 ){
				throw std::runtime_error("Invalid Dir Buffer when opening file : "+this->InputFiles.at(this->CurrentFileIndex));
			}
			if( this->ParseHeadBuffer() == -1 ){
				throw std::runtime_error("Invalid Head Buffer when opening file : "+this->InputFiles.at(this->CurrentFileIndex));
			}
			this->CurrDataBuff.bcount = 0;
		}
	}
	do{
		//if( not this->Leftovers.empty() ){
		//	RawEvents.push_back(this->Leftovers.back());
		//	this->Leftovers.pop_back();
		//	auto evt = RawEvents.back();
		//	auto toss = this->correlator->IsWithinCorrelationWindow(evt.GetTimeStamp(),evt.GetCrate(),evt.GetModule(),evt.GetChannel());
		//	(void) toss;
		//}
		while( this->EvtSpillCounter[0] == 0 or this->EvtSpillCounter[1] == 0 or this->EvtSpillCounter[2] == 0 ){
			if( this->ParseDataBuffer() == -1 ){
				throw std::runtime_error("Invalid Data Buffer in File : "+this->InputFiles.at(this->CurrentFileIndex));
			}	

			if( this->CurrentFile.eof() ){
				if( not this->OpenNextFile() ){
					return Translator::TRANSLATORSTATE::COMPLETE;
				}else{
					if( this->ParseDirBuffer() == -1 ){
						throw std::runtime_error("Invalid Dir Buffer when opening file : "+this->InputFiles.at(this->CurrentFileIndex));
					}
					if( this->ParseHeadBuffer() == -1 ){
						throw std::runtime_error("Invalid Head Buffer when opening file : "+this->InputFiles.at(this->CurrentFileIndex));
					}
					this->CurrDataBuff.bcount = 0;
				}
			}
		}
	}while(this->LastReadEvtWithin);
	//Clear for now
	return Translator::TRANSLATORSTATE::PARSING;
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
	this->console->info("Parsed Dir Buffer : found total buff size : {}, unknown [0-2] : {} {} {}, runnum : {}",this->CurrDirBuff.totalbuffsize,this->CurrDirBuff.unknown[0],this->CurrDirBuff.unknown[1],this->CurrDirBuff.unknown[2],this->CurrDirBuff.run_num);
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
	this->console->info("Found Head Buffer : facility : {}, format : {}, type : {}, date : {}, title : {}, run : {}",this->CurrHeadBuff.facility,this->CurrHeadBuff.format,this->CurrHeadBuff.type,this->CurrHeadBuff.date,this->CurrHeadBuff.run_title,this->CurrHeadBuff.run_num);

	return 0;
}

int LDFPixieTranslator::ParseDataBuffer(){
	this->ReadNextBuffer(true);
	if( this->CurrDataBuff.buffhead == HRIBF_TYPES::ENDFILE ){
		if( this->CurrDataBuff.nextbuffhead == HRIBF_TYPES::ENDFILE ){
			this->console->info("Read double EOF");
		}else{
			this->console->info("Reached single EOF, force reading next");
			this->ReadNextBuffer(true);
		}
	}else if( this->CurrDataBuff.buffhead == HRIBF_TYPES::DATA ){
		//decode the spill
		//need to advance the buffpos identifier when we move things around
		//this misses the edge case of a spill fitting within an 8194
		if( this->CurrDataBuff.currchunknum <= this->CurrDataBuff.prevchunknum ){
			++this->CurrSpillID;
		}
		//this will not output any chunk numbers for 8194 buffers that have multiple chunks
		this->console->info("spill : {} chunk: {}/{}",this->CurrSpillID,this->CurrDataBuff.currchunknum,this->CurrDataBuff.numchunks);
	}else{
		this->console->warn("found non data/non eof buffer 0x{:x}",this->CurrDataBuff.buffhead);
		this->ReadNextBuffer(true);
	}
	return 0;
}

int LDFPixieTranslator::ReadNextBuffer(bool force){
	if( this->CurrDataBuff.bcount == 0 ){
		this->CurrentFile.read(reinterpret_cast<char*>(&(this->CurrDataBuff.buffer1[0])),8194*sizeof(unsigned int));
	}else if( this->CurrDataBuff.buffpos + 3 <= 8193 and not force ){
		while( this->CurrDataBuff.currbuffer->at(this->CurrDataBuff.buffpos) == HRIBF_TYPES::ENDBUFF and  this->CurrDataBuff.buffpos < 8193 ){
			++(this->CurrDataBuff.buffpos);
		}
		if( this->CurrDataBuff.buffpos + 3 < 8193 ){
			return 0;
		}
	}
	if( this->CurrDataBuff.bcount % 2 == 0 ){
		this->CurrentFile.read(reinterpret_cast<char*>(&(this->CurrDataBuff.buffer2[0])),8194*sizeof(unsigned int));
		this->CurrDataBuff.currbuffer = &(this->CurrDataBuff.buffer1);
		this->CurrDataBuff.nextbuffer = &(this->CurrDataBuff.buffer2);
	}else{
		this->CurrentFile.read(reinterpret_cast<char*>(&(this->CurrDataBuff.buffer1[0])),8194*sizeof(unsigned int));
		this->CurrDataBuff.currbuffer = &(this->CurrDataBuff.buffer2);
		this->CurrDataBuff.nextbuffer = &(this->CurrDataBuff.buffer1);
	}
	++(this->CurrDataBuff.bcount);
	this->CurrDataBuff.buffhead = this->CurrDataBuff.currbuffer->at(0);
	this->CurrDataBuff.buffsize = this->CurrDataBuff.currbuffer->at(1);
	this->CurrDataBuff.numbytes = this->CurrDataBuff.currbuffer->at(2);
	this->CurrDataBuff.numchunks = this->CurrDataBuff.currbuffer->at(3);
	this->CurrDataBuff.prevchunknum = this->CurrDataBuff.currchunknum;
	this->CurrDataBuff.currchunknum = this->CurrDataBuff.currbuffer->at(4);
	this->CurrDataBuff.buffpos = 5;
	//for( size_t ii = 0; ii < 8194; ++ii ){
	//	this->console->info("{} {:x}",ii,this->CurrDataBuff.currbuffer->at(ii));
	//}
	
	this->CurrDataBuff.nextbuffhead = this->CurrDataBuff.nextbuffer->at(0);
	this->CurrDataBuff.nextbuffsize = this->CurrDataBuff.nextbuffer->at(1);

	return 0;
}

int LDFPixieTranslator::DecodeNextModuleDump(){
	this->CurrPixieModBuff.numwords = this->CurrDataBuff.currbuffer->at(this->CurrDataBuff.buffpos);
	++this->CurrDataBuff.buffpos;
	this->CurrPixieModBuff.modulenum = this->CurrDataBuff.currbuffer->at(this->CurrDataBuff.buffpos);
	++this->CurrDataBuff.buffpos;
	if( this->CurrPixieModBuff.numwords == 2 ){
		return -1;
	}else{
		//read through the module data
		//it has numwords-2 items to read
	}
	return 0;
}
