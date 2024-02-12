#include <stdexcept>

#include "BitDecoder.hpp"
#include "EVTTranslator.hpp"

EVTTranslator::EVTTranslator(const std::string& log,const std::string& translatorname,EVT_TYPE formattype) : Translator(log,translatorname){
	this->Format = formattype;
	CurrEVTBuiltInfo = { .rib_size = 0, .ri_size = 0, .ri_type = 0};
}	

void EVTTranslator::Parse(boost::container::devector<PhysicsData>& RawEvents){
	if( this->FinishedCurrentFile ){
		this->OpenNextFile();
	}
	switch(this->Format){
		case PRESORT:
			this->ParsePresort(RawEvents);
			break;
		case EVTBUILT:
			this->ParseEVTBuilt(RawEvents);
			break;
		default:
			throw std::runtime_error("Unknown EVT File type, not EVTBUILT or PRESORT");
			break;
	}
	if( this->CurrentFile.eof() ){
		this->FinishedCurrentFile = true;
	}
}

void EVTTranslator::ParsePresort(boost::container::devector<PhysicsData>& RawEvents){
}

void EVTTranslator::ParseEVTBuilt(boost::container::devector<PhysicsData>& RawEvents){
	do{
		if( !this->Leftovers.empty() ){
			RawEvents.push_back(this->Leftovers.back());
			this->Leftovers.pop_back();
			auto evt = RawEvents.back();
			auto toss = this->correlator->IsWithinCorrelationWindow(evt.GetRawTimeStamp()*10.0,evt.GetCrate(),evt.GetModule(),evt.GetChannel());
			(void) toss;
		}
		if( this->ReadHeader(RawEvents) != -1 ){
			this->ReadFull(RawEvents);
			#ifndef NDEBUG
			this->console->debug("{}",RawEvents.back());
			#endif
			if( !this->LastReadEvtWithin ){
				this->Leftovers.push_back(RawEvents.back());
				RawEvents.pop_back();
				this->correlator->Pop();
				this->correlator->Clear();
			}
		}
	}while(this->LastReadEvtWithin);
	//Clear for now
	RawEvents.clear();
}

int EVTTranslator::ReadFull(boost::container::devector<PhysicsData>& RawEvents){
	#ifndef NDEBUG
	this->console->debug("{}:{}:{}",this->CurrHeaderLength,this->CurrTraceLength,RawEvents.size());
	this->correlator->DumpSelf();
	#endif
	if( this->CurrHeaderLength > 4 ){
		uint32_t otherWords[this->CurrHeaderLength - 4];
		if( !this->CurrentFile.read(reinterpret_cast<char*>(&otherWords),(this->CurrHeaderLength-4)*4) ){
			return -1;
		}
		CurrDecoder->DecodeOtherWords(otherWords,&(RawEvents.back()));
	}

	//We have a trace
	if( (RawEvents.back().GetEventLength() - this->CurrHeaderLength) != 0 ){
		std::vector<uint16_t> trace(this->CurrTraceLength);
		if( !(this->CurrentFile.read(reinterpret_cast<char*>(&trace[0]),(this->CurrTraceLength)*2)) ){
			return -1;
		}
		RawEvents.back().SetRawTrace(std::move(trace));
	}

	return 0;
}

int EVTTranslator::ReadHeader(boost::container::devector<PhysicsData>& RawEvents){
	if( FindNextFragment() < 0 ){
		return -1;
	}
	if( !this->CurrentFile.read(reinterpret_cast<char*>(&firstWords),sizeof(int)*4) ){
		return -1;
	}
	//decode the header
	auto ChannelNumber = PIXIE::ChannelNumberMask(firstWords[0]);
	auto ModuleNumber = PIXIE::ModuleNumberMask(firstWords[0]);
	auto CrateNumber = PIXIE::CrateNumberMask(firstWords[0]);
	CurrHeaderLength = PIXIE::HeaderLengthMask(firstWords[0]);
	auto FinishCode = (PIXIE::FinishCodeMask(firstWords[0]) != 0);

	CurrDecoder = CMap->GetXiaDecoder(CrateNumber,ModuleNumber);
	uint32_t TimeStampLow;
	uint32_t TimeStampHigh;
	unsigned int EventEnergy;
	bool OutOfRange;
	int EventLength;

	CurrDecoder->DecodeFirstWords(firstWords,EventLength,TimeStampLow,TimeStampHigh,EventEnergy,CurrTraceLength,OutOfRange);

	uint64_t TimeStamp = static_cast<uint64_t>(TimeStampHigh);
	TimeStamp = TimeStamp<<32;
	TimeStamp += TimeStampLow;

	//note that this assumes that this isn't one of the weird firmwares where this is in wordzero
	//need to ask Toby if we actually still use those firmware versions anywhere we would want to scan this or if we should support
	//them anymore

	this->LastReadEvtWithin = this->correlator->IsWithinCorrelationWindow(10.0*TimeStamp,CrateNumber,ModuleNumber,ChannelNumber);
	RawEvents.push_back(PhysicsData(CurrHeaderLength,EventLength,CrateNumber,ModuleNumber,ChannelNumber,EventEnergy,TimeStamp));
	RawEvents.back().SetPileup(FinishCode);
	RawEvents.back().SetSaturation(OutOfRange);

	//word2 has CFD things
	//CurrDecoder->DecodeCFDParams(firstWords,RawEvents.back());

	return 0;
}

int EVTTranslator::ReadRingItemHeader(){
	if( !this->CurrentFile.read(reinterpret_cast<char*>(&CurrEVTBuiltInfo.ri_size),sizeof(int)) ){
		return -1;
	}
	if( !this->CurrentFile.read(reinterpret_cast<char*>(&CurrEVTBuiltInfo.ri_type),sizeof(int)) ){
		return -1;
	}
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
		return ReadNextFragment();
	}
	while(true){
		int type = ReadRingItemHeader();
		if( type == -1 ){
			return -1;
		}
		if( type != 30 ){
			this->CurrentFile.seekg(CurrEVTBuiltInfo.ri_size-8,std::ios::cur);
			continue;
		}
		int ribh_size = ReadRingItemBodyHeader();
		if( ribh_size < 0 ){
			return ribh_size;
		}
		ReadRingItemBody();
		if( CurrEVTBuiltInfo.rib_size < 0 ){
			return CurrEVTBuiltInfo.rib_size;
		}
		return ReadNextFragment();
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
			//fread(&data,static_cast<size_t>(sizeof(int)),1,file);
			this->console->info("{}",data);
			rib_tmp -= 4;
		}
	}else{
		this->console->warn("EVTTranslator::ReadNextFragment() called when CurrEVTBuiltInfo.rib_size == 0");
	}
	return CurrEVTBuiltInfo.rib_size;
}
