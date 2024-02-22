#include <stdexcept>

#include "BitDecoder.hpp"
#include "EVTTranslator.hpp"
#include "Translator.hpp"

EVTTranslator::EVTTranslator(const std::string& log,const std::string& translatorname) : Translator(log,translatorname){
	this->CurrEVTBuiltInfo = { .rib_size = 0, .ri_size = 0, .ri_type = 0};
}	

EVTTranslator::~EVTTranslator(){
	if( not this->Leftovers.empty() ){
		this->console->error("Still have data left in the queue");
	}	
}

Translator::TRANSLATORSTATE EVTTranslator::Parse(boost::container::devector<PhysicsData>& RawEvents){
	if( this->FinishedCurrentFile ){
		if( not this->OpenNextFile() ){
			return Translator::TRANSLATORSTATE::COMPLETE;
		}
	}
	do{
		if( not this->Leftovers.empty() ){
			RawEvents.push_back(this->Leftovers.back());
			this->Leftovers.pop_back();
			auto evt = RawEvents.back();
			auto toss = this->correlator->IsWithinCorrelationWindow(evt.GetTimeStamp(),evt.GetCrate(),evt.GetModule(),evt.GetChannel());
			(void) toss;
		}
		if( this->ReadHeader(RawEvents) != -1 ){
			this->ReadFull(RawEvents);
			#ifdef TRANSLATOR_DEBUG
			this->console->debug("{}",RawEvents.back());
			#endif
			if( not this->LastReadEvtWithin ){
				this->Leftovers.push_back(RawEvents.back());
				RawEvents.pop_back();
				this->correlator->Pop();
				this->correlator->Clear();
			}
		}
		if( this->CurrentFile.eof() ){
			if( not this->OpenNextFile() ){
				return Translator::TRANSLATORSTATE::COMPLETE;
			}
		}
	}while(this->LastReadEvtWithin);
	//Clear for now
	return Translator::TRANSLATORSTATE::PARSING;
}

int EVTTranslator::ReadFull(boost::container::devector<PhysicsData>& RawEvents){
	#ifdef TRANSLATOR_DEBUG
	this->console->debug("{}:{}:{}",this->CurrHeaderLength,this->CurrTraceLength,RawEvents.size());
	this->correlator->DumpSelf();
	#endif
	if( this->CurrHeaderLength > 4 ){
		uint32_t otherWords[this->CurrHeaderLength - 4];
		if( !this->CurrentFile.read(reinterpret_cast<char*>(&otherWords),(this->CurrHeaderLength-4)*4) ){
			return -1;
		}
		this->CurrDecoder->DecodeOtherWords(otherWords,&(RawEvents.back()));
	}

	if( (RawEvents.back().GetEventLength() - this->CurrHeaderLength) != 0 ){
		RawEvents.back().SetRawTraceLength(this->CurrTraceLength);
		if( !(this->CurrentFile.read(reinterpret_cast<char*>(&(RawEvents.back().GetRawTraceData()[0])),(this->CurrTraceLength)*2)) ){
			return -1;
		}
	}

	return 0;
}

int EVTTranslator::ReadHeader(boost::container::devector<PhysicsData>& RawEvents){
	if( this->FindNextFragment() < 0 ){
		return -1;
	}
	if( !this->CurrentFile.read(reinterpret_cast<char*>(&firstWords),sizeof(int)*4) ){
		return -1;
	}
	//decode the header
	uint32_t ChannelNumber = PIXIE::ChannelNumberMask(firstWords[0]);
	uint32_t ModuleNumber = (PIXIE::ModuleNumberMask(firstWords[0]))-2;
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

	//note that this assumes that this isn't one of the weird firmwares where this is in wordzero
	//need to ask Toby if we actually still use those firmware versions anywhere we would want to scan this or if we should support
	//them anymore

	this->LastReadEvtWithin = this->correlator->IsWithinCorrelationWindow(TimeStampInNS,CrateNumber,ModuleNumber,ChannelNumber);
	RawEvents.push_back(PhysicsData(CurrHeaderLength,EventLength,CrateNumber,ModuleNumber,ChannelNumber,
					this->CMap->GetGlobalBoardID(CrateNumber,ModuleNumber),
				        this->CMap->GetGlobalChanID(CrateNumber,ModuleNumber,ChannelNumber),
					EventEnergy,TimeStamp));
	RawEvents.back().SetPileup(FinishCode);
	RawEvents.back().SetSaturation(OutOfRange);
	RawEvents.back().SetTimeStamp(TimeStampInNS);

	//word2 has CFD things
	//this->CurrDecoder->DecodeCFDParams(firstWords,RawEvents.back());

	return 0;
}

int EVTTranslator::ReadRingItemHeader(){
	if( !this->CurrentFile.read(reinterpret_cast<char*>(&CurrEVTBuiltInfo.ri_size),sizeof(int)) ){
		return -1;
	}
	if( !this->CurrentFile.read(reinterpret_cast<char*>(&CurrEVTBuiltInfo.ri_type),sizeof(int)) ){
		return -1;
	}
	#ifdef TRANSLATOR_DEBUG
	if( CurrEVTBuiltInfo.ri_type != 30 ){
		switch(CurrEVTBuiltInfo.ri_type){
			case 1:
				this->console->info("====================BEGIN RUN====================");
				break;
			case 2:
				this->console->info("=====================END RUN=====================");
				break;
			case 3:
				this->console->info("====================PAUSE RUN====================");
				break;
			case 4:
				this->console->info("===================RESUME RUN====================");
				break;
			case 11:
				this->console->info("==============MONITORED VARIABLES================");
				break;
			case 12:
				this->console->info("===================RING FORMAT===================");
				break;
			case 20:
				this->console->info("==================SCALARS FOUND==================");
				break;
			case 31:
				this->console->info("===============PHYSICS EVENT COUNT===============");
				break;
			case 40:
				this->console->info("===================EVB FRAGMENT==================");
				break;
			case 41:
				this->console->info("===============EVB UNKNOWN PAYLOAD===============");
				break;
			case 42:
				this->console->info("==================EVB GLOM INFO==================");
				break;
			case 32768:
				this->console->info("===============FIRST USER ITEM CODE==============");
				break;
			default:
				this->console->critical("UNHANDLED RING ITEM TYPE : {}",CurrEVTBuiltInfo.ri_type);
				break;
		}
	}		
	#endif
	return CurrEVTBuiltInfo.ri_type;
}

int EVTTranslator::ReadRingItemBodyHeader(){
	uint32_t ribh_size;
	uint64_t ribh_ts;
	uint32_t ribh_sid;
	uint32_t ribh_bt;
	if( !this->CurrentFile.read(reinterpret_cast<char*>(&ribh_size),sizeof(int)) ){
		return -1;
	}
	if( !this->CurrentFile.read(reinterpret_cast<char*>(&ribh_ts),sizeof(uint64_t)) ){
		return -1;
	}
	if( !this->CurrentFile.read(reinterpret_cast<char*>(&ribh_sid),sizeof(int)) ){
		return -1;
	}
	if( !this->CurrentFile.read(reinterpret_cast<char*>(&ribh_bt),sizeof(int)) ){
		return -1;
	}
	return ribh_size;
}

int EVTTranslator::ReadRingItemBody(){
	if( !this->CurrentFile.read(reinterpret_cast<char*>(&CurrEVTBuiltInfo.rib_size),sizeof(int)) ){
		return -1;
	}
	CurrEVTBuiltInfo.rib_size -= 4;
	return CurrEVTBuiltInfo.rib_size;
}

int EVTTranslator::FindNextFragment(){
	if( CurrEVTBuiltInfo.rib_size > 0 ){
		return this->ReadNextFragment();
	}
	while(true){
		int type = this->ReadRingItemHeader();
		if( type == -1 ){
			return -1;
		}
		if( type != 30 ){
			this->CurrentFile.seekg(CurrEVTBuiltInfo.ri_size-8,std::ios::cur);
			continue;
		}
		int ribh_size = this->ReadRingItemBodyHeader();
		if( ribh_size < 0 ){
			return ribh_size;
		}
		this->ReadRingItemBody();
		if( CurrEVTBuiltInfo.rib_size < 0 ){
			return CurrEVTBuiltInfo.rib_size;
		}
		return this->ReadNextFragment();
	}
}

int EVTTranslator::ReadNextFragment(){
	if( CurrEVTBuiltInfo.rib_size > 48 ){
		this->CurrentFile.seekg(48,std::ios::cur);
		uint32_t frag_size;
		uint32_t dev_info;
		if( !this->CurrentFile.read(reinterpret_cast<char*>(&frag_size),sizeof(int)) ){
			return -1;
		}
		if( !this->CurrentFile.read(reinterpret_cast<char*>(&dev_info),sizeof(int)) ){
			return -1;
		}
		CurrEVTBuiltInfo.rib_size -= (48 + frag_size*2);
		if( CurrEVTBuiltInfo.rib_size < 0 ){
			this->console->error("NAVIGATION THROUGH RING BUFFER BROKEN");
			return -1;
		}
	}else if( CurrEVTBuiltInfo.rib_size < 48 ){
		this->console->info("Remaining Data : ");
		int rib_tmp = CurrEVTBuiltInfo.rib_size;
		int data;
		while(rib_tmp > 0 ){
			this->CurrentFile.read(reinterpret_cast<char*>(&data),sizeof(int));
			this->console->info("{}",data);
			rib_tmp -= 4;
		}
	}else{
		this->console->warn("EVTTranslator::ReadNextFragment() called when CurrEVTBuiltInfo.rib_size == 0");
	}
	return CurrEVTBuiltInfo.rib_size;
}
