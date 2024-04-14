#include <stdexcept>

#include "LDFPixieTranslator.hpp"
#include "Translator.hpp"

LDFPixieTranslator::LDFPixieTranslator(const std::string& log,const std::string& translatorname) : Translator(log,translatorname){
	this->PrevTimeStamp = 0;
	this->EvtSpillCounter = std::vector<int>(3,0);
	this->CurrSpillID = -1;
	this->CurrDirBuff = { .bufftype = HRIBF_TYPES::DIR, .buffsize = 8192, .totalbuffsize = 0, .unknown = { 0, 0, 0}, .run_num = 0 };
}	

Translator::TRANSLATORSTATE LDFPixieTranslator::Parse(boost::container::devector<PhysicsData>& RawEvents){
	if( this->FinishedCurrentFile ){
		if( not this->OpenNextFile() ){
			return Translator::TRANSLATORSTATE::COMPLETE;
		}else{
			if( this->ParseDirBuffer() != -1 ){
				throw std::runtime_error("Invalid Dir Buffer when opening file : "+this->InputFiles.at(this->CurrentFileIndex));
			}
		}
	}
	//do{
	//	if( not this->Leftovers.empty() ){
	//		RawEvents.push_back(this->Leftovers.back());
	//		this->Leftovers.pop_back();
	//		auto evt = RawEvents.back();
	//		auto toss = this->correlator->IsWithinCorrelationWindow(evt.GetTimeStamp(),evt.GetCrate(),evt.GetModule(),evt.GetChannel());
	//		(void) toss;
	//	}
	//	if( this->LoadNextBuffer() != -1 ){
	//	}

	//	if( this->CurrentFile.eof() ){
	//		if( not this->OpenNextFile() ){
	//			return Translator::TRANSLATORSTATE::COMPLETE;
	//		}else{
	//		}
	//	}
	//}while(this->LastReadEvtWithin);
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
	this->console->info("found total buff size : {}, unknown [0-2] : {} {} {}, runnum : {}",this->CurrDirBuff.totalbuffsize,this->CurrDirBuff.unknown[0],this->CurrDirBuff.unknown[1],this->CurrDirBuff.unknown[2],this->CurrDirBuff.run_num);
	return 0;
}

int LDFPixieTranslator::ParseHeadBuffer(){
	return 0;
}

int LDFPixieTranslator::ParseDataBuffer(boost::container::devector<PhysicsData>&){
	return 0;
}
