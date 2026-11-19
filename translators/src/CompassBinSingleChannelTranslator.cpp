#include <algorithm>
#include <filesystem>
#include <fstream>
#include <regex>
#include <stdexcept>

#include "BitDecoder.hpp"
#include "CompassBinSingleChannelTranslator.hpp"
#include "Translator.hpp"

CompassBinSingleChannelTranslator::CompassBinSingleChannelTranslator(const std::string& log, const std::string& translatorname)
	: Translator(log, translatorname) {
	this->LastReadEvtWithin = true;
}

void CompassBinSingleChannelTranslator::FinalizeFiles() {
	Translator::FinalizeFiles();
	
	std::sort(this->InputFiles.begin(), this->InputFiles.end());
	std::regex re("DataR_CH(\\d+)@((?:DT|V)\\d+(?:S|X)?)_(\\d+)_.*");
	for (const auto& f : this->InputFiles) {
		std::smatch match;
		auto file_stem = std::filesystem::path(f).stem().string();
		if (std::regex_match(f, match, re)) {
			auto chan_num = std::stoi(match[1]);
			auto digitizer = match[2].str();
			auto pid = std::stoi(match[3]);
			CaenDigitizerKey key{chan_num, digitizer, pid};
			if (this->CaenMapping.find(key) == this->CaenMapping.end()) {
				this->CaenMapping[key] = {CaenDecoder{}, {f}};
			} else {
				this->CaenMapping[key].second.push_back(f);
			}
		}
	}
	for (auto& kv : this->CaenMapping) {
		std::sort(kv.second.second.begin(), kv.second.second.end());
		kv.second.first = this->DecodeSingleFileHeader(kv.second.second.front());
	}
}

CaenDecoder CompassBinSingleChannelTranslator::DecodeSingleFileHeader(const std::string& inputfile) const {
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

CompassBinSingleChannelTranslator::~CompassBinSingleChannelTranslator() {
	if (not this->Leftovers.empty()) {
		this->console->error("Still have data left in the queue");
	}
}

Translator::TRANSLATORSTATE CompassBinSingleChannelTranslator::Parse(boost::container::devector<PhysicsData>& RawEvents) {
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
