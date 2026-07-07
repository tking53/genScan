#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>

#include <boost/sort/pdqsort/pdqsort.hpp>

#include "BitDecoder.hpp"
#include "CompassBinSingleFileTranslator.hpp"
#include "Translator.hpp"

CompassBinSingleFileTranslator::CompassBinSingleFileTranslator(const std::string& log, const std::string& translatorname)
	: Translator(log, translatorname) {
	this->LastReadEvtWithin = true;
	this->FinishedReadingFiles = false;
	this->ReadCurrentFile = false;
	this->PrevTimeStamp = std::numeric_limits<uint64_t>::max();
	this->single_file_decoder.reset(nullptr);
}

void CompassBinSingleFileTranslator::FinalizeFiles() {
	this->console->info("Sorting Files for Compass");
	std::sort(this->InputFiles.begin(), this->InputFiles.end(), [](const auto& a, const auto& b) {
		auto stem_a = std::filesystem::path(a).stem().string();
		auto stem_b = std::filesystem::path(b).stem().string();
		if (stem_a.length() < stem_b.length()) {
			return true;
		} else if (stem_a.length() == stem_b.length()) {
			auto a_ = stem_a.rfind("_");
			auto numa = std::stoi(stem_a.substr(a_ + 1));
			auto b_ = stem_b.rfind("_");
			auto numb = std::stoi(stem_b.substr(b_ + 1));
			return numa < numb;
		} else {
			return false;
		}
	});
	this->console->info("Here is the order we will process the files");
	for (const auto& f : this->InputFiles) {
		this->console->info("{}", f);
	}
	Translator::FinalizeFiles();
}

CompassBinSingleFileTranslator::~CompassBinSingleFileTranslator() {
	if (not this->Leftovers.empty()) {
		this->console->error("Still have data left in the queue");
	}
}

int CompassBinSingleFileTranslator::ReadNext() {
	if (!this->CurrentFile.read(reinterpret_cast<char*>(&firstWords), sizeof(uint16_t) * 6)) {
		return -1;
	}
	uint16_t board = firstWords[0];
	uint16_t channel = firstWords[1];
	uint64_t timestamp = static_cast<uint64_t>(firstWords[5]) << 48 |
			     static_cast<uint64_t>(firstWords[4]) << 32 |
			     static_cast<uint64_t>(firstWords[3]) << 16 |
			     static_cast<uint64_t>(firstWords[2]);

	uint16_t raw_energy = 0;
	uint64_t cal_energy_bits = 0;
	double cal_energy = 0.0;
	uint16_t energy_short = 0;

	if (this->single_file_decoder->HasBit0() and this->single_file_decoder->HasBit1() and this->single_file_decoder->HasBit2()) {
		if (!this->CurrentFile.read(reinterpret_cast<char*>(&secondWords), sizeof(uint16_t) * 6)) {
			return -1;
		}
		raw_energy = secondWords[0];
		cal_energy_bits = static_cast<uint64_t>(secondWords[4]) << 48 |
				  static_cast<uint64_t>(secondWords[3]) << 32 |
				  static_cast<uint64_t>(secondWords[2]) << 16 |
				  static_cast<uint64_t>(secondWords[1]);
		std::memcpy(&cal_energy, &cal_energy_bits, sizeof(cal_energy));
		energy_short = secondWords[5];
	} else if (this->single_file_decoder->HasBit0() and this->single_file_decoder->HasBit1()) {
		if (!this->CurrentFile.read(reinterpret_cast<char*>(&secondWords), sizeof(uint16_t) * 5)) {
			return -1;
		}
		raw_energy = secondWords[0];
		cal_energy_bits = static_cast<uint64_t>(secondWords[4]) << 48 |
				  static_cast<uint64_t>(secondWords[3]) << 32 |
				  static_cast<uint64_t>(secondWords[2]) << 16 |
				  static_cast<uint64_t>(secondWords[1]);
		std::memcpy(&cal_energy, &cal_energy_bits, sizeof(cal_energy));
	} else if (this->single_file_decoder->HasBit0() and this->single_file_decoder->HasBit2()) {
		if (!this->CurrentFile.read(reinterpret_cast<char*>(&secondWords), sizeof(uint16_t) * 2)) {
			return -1;
		}
		raw_energy = secondWords[0];
		energy_short = secondWords[5];
	} else if (this->single_file_decoder->HasBit1() and this->single_file_decoder->HasBit2()) {
		if (!this->CurrentFile.read(reinterpret_cast<char*>(&secondWords), sizeof(uint16_t) * 5)) {
			return -1;
		}
		cal_energy_bits = static_cast<uint64_t>(secondWords[3]) << 48 |
				  static_cast<uint64_t>(secondWords[2]) << 32 |
				  static_cast<uint64_t>(secondWords[1]) << 16 |
				  static_cast<uint64_t>(secondWords[0]);
		std::memcpy(&cal_energy, &cal_energy_bits, sizeof(cal_energy));
		energy_short = secondWords[4];
	} else if (this->single_file_decoder->HasBit0()) {
		if (!this->CurrentFile.read(reinterpret_cast<char*>(&secondWords), sizeof(uint16_t))) {
			return -1;
		}
		raw_energy = secondWords[0];
	} else if (this->single_file_decoder->HasBit1()) {
		if (!this->CurrentFile.read(reinterpret_cast<char*>(&secondWords), sizeof(uint16_t) * 4)) {
			return -1;
		}
		cal_energy_bits = static_cast<uint64_t>(secondWords[3]) << 48 |
				  static_cast<uint64_t>(secondWords[2]) << 32 |
				  static_cast<uint64_t>(secondWords[1]) << 16 |
				  static_cast<uint64_t>(secondWords[0]);
		std::memcpy(&cal_energy, &cal_energy_bits, sizeof(cal_energy));
	} else if (this->single_file_decoder->HasBit2()) {
		if (!this->CurrentFile.read(reinterpret_cast<char*>(&secondWords), sizeof(uint16_t))) {
			return -1;
		}
		energy_short = secondWords[0];
	} else {
		// no-op
	}

	double TimeStampInNS = timestamp * 1.0e-3;
	this->Leftovers.push_back(PhysicsData(0, 0, 0, board, channel,
					      this->CMap->GetGlobalBoardID(0, board),
					      this->CMap->GetGlobalChanID(0, board, channel),
					      raw_energy, timestamp / 1000));

	this->Leftovers.back().SetTimeStamp(TimeStampInNS);
	this->Leftovers.back().SetCFDTimeStamp(TimeStampInNS);

	if (this->single_file_decoder->HasBit2()) {
		// store energy_short within the QDC sum
		this->Leftovers.back().SetRawQDCSumLength(1);
		this->Leftovers.back().SetQDCValue(0, energy_short);
	}

	uint32_t event_flags = 0;
	if (!this->CurrentFile.read(reinterpret_cast<char*>(&event_flags), sizeof(uint32_t))) {
		return -1;
	}
	this->Leftovers.back().SetPileup((event_flags & 0x8000) != 0);
	// first one is similar to pixie, the input from the analog stage is saturating (trace out of range)
	// the second one is the calculated value within the filter is saturating which pixie does not have
	this->Leftovers.back().SetSaturation((event_flags & 0x400) != 0 or (event_flags & 0x80) != 0);

	// the waveform_code doesn't appear to be used, in the data file???
	[[maybe_unused]] uint8_t waveform_code = 0;
	uint32_t num_samples = 0;
	if (this->single_file_decoder->HasBit3()) {
		if (!this->CurrentFile.read(reinterpret_cast<char*>(&thirdWords), sizeof(uint8_t) * 5)) {
			return -1;
		}
		waveform_code = thirdWords[0];
		if (waveform_code != 1) {
			this->console->info("waveform_code({}) != 1", waveform_code);
		}
		num_samples = static_cast<uint32_t>(thirdWords[4]) << 24 |
			      static_cast<uint32_t>(thirdWords[3]) << 16 |
			      static_cast<uint32_t>(thirdWords[2]) << 8 |
			      static_cast<uint32_t>(thirdWords[1]);
		this->Leftovers.back().SetRawTraceLength(num_samples);
		if (!this->CurrentFile.read(reinterpret_cast<char*>(&(this->Leftovers.back().GetRawTraceData()[0])), sizeof(uint16_t) * num_samples)) {
			return -1;
		}
	}

	return 0;
}

void CompassBinSingleFileTranslator::ReadEntireFile() {
	while (this->CurrentFile.good() and not this->CurrentFile.eof()) {
		this->ReadNext();
	}
	boost::sort::pdqsort(this->Leftovers.begin(), this->Leftovers.end(), [](const auto& a, const auto& b) {
		return b < a;
	});
	this->ReadCurrentFile = true;
}

Translator::TRANSLATORSTATE CompassBinSingleFileTranslator::Parse(boost::container::devector<PhysicsData>& RawEvents) {
	if (this->FinishedCurrentFile) {
		if (not this->OpenNextFile()) {
			return Translator::TRANSLATORSTATE::COMPLETE;
		}
	}
	if (this->single_file_decoder == nullptr) {
		uint16_t header;
		if (!this->CurrentFile.read(reinterpret_cast<char*>(&header), sizeof(uint16_t))) {
			this->console->error("Unable to decode header");
			throw std::runtime_error("Unable to read compass header");
		}
		this->single_file_decoder.reset(new CaenDecoder());
		this->single_file_decoder->SetBits(header);
	}

	if (not this->ReadCurrentFile) {
		this->ReadEntireFile();
	}

	// clang-format off
	this->LastReadEvtWithin = this->correlator->IsWithinCorrelationWindow(this->Leftovers.back().GetTimeStamp(),
		      this->Leftovers.back().GetCrate(), this->Leftovers.back().GetModule(), this->Leftovers.back().GetChannel());

	do {
		RawEvents.push_back(this->Leftovers.back());
		this->Leftovers.pop_back();
		if ( this->Leftovers.size() > 0 ) {
			this->LastReadEvtWithin = this->correlator->IsWithinCorrelationWindow(this->Leftovers.back().GetTimeStamp(),
		 	     this->Leftovers.back().GetCrate(), this->Leftovers.back().GetModule(), this->Leftovers.back().GetChannel());
		} else {
			if (not this->OpenNextFile()) {
				return Translator::TRANSLATORSTATE::COMPLETE;
			} else {
				this->ReadEntireFile();
				this->LastReadEvtWithin = this->correlator->IsWithinCorrelationWindow(this->Leftovers.back().GetTimeStamp(),
		      			this->Leftovers.back().GetCrate(), this->Leftovers.back().GetModule(), this->Leftovers.back().GetChannel());
			}
		}

	} while (this->LastReadEvtWithin);
	this->correlator->Clear();
	// clang-format on
	return Translator::TRANSLATORSTATE::PARSING;
}
