#include <algorithm>
#include <fstream>
#include <stdexcept>

#include "BitDecoder.hpp"
#include "CompassBinSingleFileTranslator.hpp"
#include "Translator.hpp"

CompassBinSingleFileTranslator::CompassBinSingleFileTranslator(const std::string& log, const std::string& translatorname)
	: Translator(log, translatorname) {
	this->LastReadEvtWithin = true;
}

void CompassBinSingleFileTranslator::FinalizeFiles() {
	Translator::FinalizeFiles();

	// sort the filenames, because we need to get the first one to parse the
	std::sort(this->InputFiles.begin(), this->InputFiles.end());
	this->single_file_decoder = this->DecodeSingleFileHeader(this->InputFiles.front());
}

CaenDecoder CompassBinSingleFileTranslator::DecodeSingleFileHeader(const std::string& inputfile) const {
	std::ifstream curr_input(inputfile);
	uint16_t header = 0xCAE1;
	if (not curr_input.read(reinterpret_cast<char*>(&header), sizeof(uint16_t))) {
		this->console->error("Unable to decode header for {}", inputfile);
		throw std::runtime_error("Unable to read compass header");
	}
	curr_input.close();

	CaenDecoder decoder;
	decoder.SetBits(header);
	return decoder;
}

CompassBinSingleFileTranslator::~CompassBinSingleFileTranslator() {
	if (not this->Leftovers.empty()) {
		this->console->error("Still have data left in the queue");
	}
}

Translator::TRANSLATORSTATE CompassBinSingleFileTranslator::Parse(boost::container::devector<PhysicsData>& RawEvents) {
	// if (this->FinishedCurrentFile) {
	// 	if (not this->OpenNextFile()) {
	// 		return Translator::TRANSLATORSTATE::COMPLETE;
	// 	}
	// }
	// do {
	// 	if (not this->Leftovers.empty()) {
	// 		// this->console->info("leftovers push size of rawevts {}, leftovers {}",RawEvents.size(),this->Leftovers.size());
	// 		RawEvents.push_back(this->Leftovers.back());
	// 		this->Leftovers.pop_back();
	// 		auto evt = RawEvents.back();
	// 		// make sure correlator is clear
	// 		this->correlator->Clear();
	// 		this->LastReadEvtWithin = this->correlator->IsWithinCorrelationWindow(evt.GetTimeStamp(), evt.GetCrate(), evt.GetModule(), evt.GetChannel());
	// 	}
	// 	if (this->ReadHeader(RawEvents) != -1) {
	// 		this->ReadFull(RawEvents);
	// #ifdef TRANSLATOR_DEBUG
	// 		this->console->debug("{}", RawEvents.back());
	// #endif
	// 		if (not this->LastReadEvtWithin) {
	// 			// this->console->info("raw push size of rawevts {}, leftovers {}",RawEvents.size(),this->Leftovers.size());
	// 			this->Leftovers.push_back(RawEvents.back());
	// 			RawEvents.pop_back();
	// 			// this->correlator->Pop();
	// 			this->correlator->Clear();
	// 		}
	// 	}
	// 	if (this->CurrentFile.eof() or not this->CurrentFile.good()) {
	// 		if (not this->OpenNextFile()) {
	// 			return Translator::TRANSLATORSTATE::COMPLETE;
	// 		}
	// 	}
	// } while (this->LastReadEvtWithin);
	// // Clear for now
	// this->LastReadEvtWithin = true;
	return Translator::TRANSLATORSTATE::PARSING;
}
