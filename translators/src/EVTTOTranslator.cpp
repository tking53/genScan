#include <stdexcept>

#include "BitDecoder.hpp"
#include "EVTTOTranslator.hpp"
#include "Translator.hpp"

EVTTOTranslator::EVTTOTranslator(const std::string& log, const std::string& translatorname)
	: Translator(log, translatorname) {
	this->PrevTimeStamp = 0;
	this->LastReadEvtWithin = true;
}

EVTTOTranslator::~EVTTOTranslator() {
	if (not this->Leftovers.empty()) {
		this->console->error("Still have data left in the queue");
	}
}

Translator::TRANSLATORSTATE EVTTOTranslator::Parse(boost::container::devector<PhysicsData>& RawEvents) {
	if (this->FinishedCurrentFile) {
		if (not this->OpenNextFile()) {
			return Translator::TRANSLATORSTATE::COMPLETE;
		}
	}
	do {
		if (not this->Leftovers.empty()) {
			// this->console->info("leftovers push size of rawevts {}, leftovers {}",RawEvents.size(),this->Leftovers.size());
			RawEvents.push_back(this->Leftovers.back());
			this->Leftovers.pop_back();
			auto evt = RawEvents.back();
			// make sure correlator is clear
			this->correlator->Clear();
			this->LastReadEvtWithin = this->correlator->IsWithinCorrelationWindow(evt.GetTimeStamp(), evt.GetCrate(), evt.GetModule(), evt.GetChannel());
		}
		if (this->ReadHeader(RawEvents) != -1) {
			this->ReadFull(RawEvents);
#ifdef TRANSLATOR_DEBUG
			this->console->debug("{}", RawEvents.back());
#endif
			if (not this->LastReadEvtWithin) {
				// this->console->info("raw push size of rawevts {}, leftovers {}",RawEvents.size(),this->Leftovers.size());
				this->Leftovers.push_back(RawEvents.back());
				RawEvents.pop_back();
				// this->correlator->Pop();
				this->correlator->Clear();
			}
		}
		if (this->CurrentFile.eof()) {
			if (not this->OpenNextFile()) {
				return Translator::TRANSLATORSTATE::COMPLETE;
			}
		}
	} while (this->LastReadEvtWithin);
	// Clear for now
	this->LastReadEvtWithin = true;
	return Translator::TRANSLATORSTATE::PARSING;
}

int EVTTOTranslator::ReadFull(boost::container::devector<PhysicsData>& RawEvents) {
#ifdef TRANSLATOR_DEBUG
	this->console->debug("{}:{}:{}", this->CurrHeaderLength, this->CurrTraceLength, RawEvents.size());
	this->correlator->DumpSelf();
#endif
	if (this->CurrHeaderLength > 4) {
		if (!this->CurrentFile.read(reinterpret_cast<char*>(&otherWords), (this->CurrHeaderLength - 4) * 4)) {
			return -1;
		}
		this->CurrDecoder->DecodeOtherWords(otherWords, &(RawEvents.back()));
	}

	if ((RawEvents.back().GetEventLength() - this->CurrHeaderLength) != 0) {
		RawEvents.back().SetRawTraceLength(this->CurrTraceLength);
		if (!(this->CurrentFile.read(reinterpret_cast<char*>(&(RawEvents.back().GetRawTraceData()[0])), (this->CurrTraceLength) * 2))) {
			return -1;
		}
	}

	return 0;
}

int EVTTOTranslator::ReadHeader(boost::container::devector<PhysicsData>& RawEvents) {
	if (!this->CurrentFile.read(reinterpret_cast<char*>(&firstWords), sizeof(int) * 4)) {
		return -1;
	}
	// decode the header
	uint32_t ChannelNumber = PIXIE::ChannelNumberMask(firstWords[0]);
	uint32_t ModuleNumber = (PIXIE::ModuleNumberMask(firstWords[0])) - 2;
	uint32_t CrateNumber = PIXIE::CrateNumberMask(firstWords[0]);
	this->CurrHeaderLength = PIXIE::HeaderLengthMask(firstWords[0]);
	uint32_t FinishCode = (PIXIE::FinishCodeMask(firstWords[0]) != 0);

	try {
		this->CurrDecoder = CMap->GetXiaDecoder(CrateNumber, ModuleNumber);
	} catch (const boost::container::out_of_range& e) {
		this->console->error("Ill formed config file, Crate : {} Board : {} does not exist. The next message is what boost reports", CrateNumber, ModuleNumber);
		throw std::runtime_error(e.what());
	}
	uint32_t TimeStampLow;
	uint32_t TimeStampHigh;
	uint32_t EventEnergy;
	bool OutOfRange;
	uint32_t EventLength;

	this->CurrDecoder->DecodeFirstWords(firstWords, EventLength, TimeStampLow, TimeStampHigh, EventEnergy, CurrTraceLength, OutOfRange);

	uint64_t TimeStamp = static_cast<uint64_t>(TimeStampHigh);
	TimeStamp = TimeStamp << 32;
	TimeStamp += TimeStampLow;
	double TimeStampInNS = TimeStamp * (this->CMap->GetModuleClockTicksToNS(CrateNumber, ModuleNumber));

	// note that this assumes that this isn't one of the weird firmwares where this is in wordzero
	// need to ask Toby if we actually still use those firmware versions anywhere we would want to scan this or if we should support
	// them anymore

	// this->LastReadEvtWithin = this->correlator->IsWithinCorrelationWindow(TimeStampInNS,CrateNumber,ModuleNumber,ChannelNumber);
	RawEvents.push_back(PhysicsData(CurrHeaderLength, EventLength, CrateNumber, ModuleNumber, ChannelNumber,
					this->CMap->GetGlobalBoardID(CrateNumber, ModuleNumber),
					this->CMap->GetGlobalChanID(CrateNumber, ModuleNumber, ChannelNumber),
					EventEnergy, TimeStamp));
	RawEvents.back().SetPileup(FinishCode);
	RawEvents.back().SetSaturation(OutOfRange);

	// word2 has CFD things
	double CFDTimeStampInNS = this->CurrDecoder->DecodeCFDParams(firstWords, TimeStamp, RawEvents.back());

#ifdef TRANSLATOR_DEBUG
	this->console->debug("TS : {}, TS(ns) : {}, CFDTS(ns) : {}", TimeStamp, TimeStampInNS, CFDTimeStampInNS);
#endif
	// always use the cfd based TimeStampInNS to event build, it is the same other if nothing is set
	RawEvents.back().SetTimeStamp(TimeStampInNS);
	RawEvents.back().SetCFDTimeStamp(CFDTimeStampInNS);

	if (TimeStamp < this->PrevTimeStamp) {
		this->console->critical("Timestamp out of order current : {}, previous : {}", TimeStamp, this->PrevTimeStamp);
	}
	this->PrevTimeStamp = TimeStamp;

	this->LastReadEvtWithin = this->correlator->IsWithinCorrelationWindow(RawEvents.back().GetTimeStamp(), CrateNumber, ModuleNumber, ChannelNumber);

	return 0;
}
