#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "ChannelMap.hpp"
#include "Correlator.hpp"
#include "DataParser.hpp"

#include "EVTTranslator.hpp"
#include "EVTTOTranslator.hpp"
#include "EVTPresortTranslator.hpp"
#include "LDFPixieTranslator.hpp"
#include "PacmanLDFPixieTranslator.hpp"
#include "CompassBinSingleFileTranslator.hpp"
#include "CompassBinTimeSortedTranslator.hpp"
#include "CompassBinSingleChannelTranslator.hpp"

DataParser::DataParser(DataParser::DataFileType dft, const std::string& log) {
	this->DataType = dft;
	this->LogName = log;
	switch (this->DataType) {
	case EVT_BUILT:
		this->ParserName = "EVT_BUILT";
		break;
	case CAEN_ROOT:
		this->ParserName = "CAEN_ROOT";
		break;
	case CAEN_BIN_SINGLE_FILE:
		this->ParserName = "CAEN_BIN_SINGLE_FILE";
		break;
	case CAEN_BIN_SINGLE_CHANNEL:
		this->ParserName = "CAEN_BIN_SINGLE_CHANNEL";
		break;
	case CAEN_BIN_TIME_SORTED:
		this->ParserName = "CAEN_BIN_TIME_SORTED";
		break;
	case LDF_PIXIE:
		this->ParserName = "LDF_PIXIE";
		break;
	case PACMAN_LDF_PIXIE:
		this->ParserName = "PACMAN_LDF_PIXIE";
		break;
	case PLD:
		this->ParserName = "PLD";
		break;
	case EVT_PRESORT:
		this->ParserName = "EVT_Presort";
		break;
	case EVT_TO:
		this->ParserName = "EVT_TO";
		break;
	case Unknown:
	default:
		this->ParserName = "UNKNOWN";
		break;
	}

	switch (this->DataType) {
	case EVT_BUILT:
		this->console = spdlog::get(this->LogName)->clone("EVT_BUILT_Parser");
		this->DataTranslator.reset(new EVTTranslator(this->LogName, this->ParserName));
		break;
	case LDF_PIXIE:
		this->console = spdlog::get(this->LogName)->clone("LDF_PIXIE");
		this->DataTranslator.reset(new LDFPixieTranslator(this->LogName, this->ParserName));
		break;
	case PACMAN_LDF_PIXIE:
		this->console = spdlog::get(this->LogName)->clone("PACMAN_LDF_PIXIE");
		this->DataTranslator.reset(new PacmanLDFPixieTranslator(this->LogName, this->ParserName));
		break;
	case EVT_PRESORT:
		this->console = spdlog::get(this->LogName)->clone("EVT_Presort_Parser");
		this->DataTranslator.reset(new EVTPresortTranslator(this->LogName, this->ParserName));
		break;
	case EVT_TO:
		this->console = spdlog::get(this->LogName)->clone("EVT_TO_Parser");
		this->DataTranslator.reset(new EVTTOTranslator(this->LogName, this->ParserName));
		break;
	case CAEN_BIN_SINGLE_FILE:
		this->console = spdlog::get(this->LogName)->clone("CAEN_BIN_SINGLE_FILE_Parser");
		this->DataTranslator.reset(new CompassBinSingleFileTranslator(this->LogName, this->ParserName));
		break;
	case CAEN_BIN_TIME_SORTED:
		this->console = spdlog::get(this->LogName)->clone("CAEN_BIN_TIME_SORTED_Parser");
		this->DataTranslator.reset(new CompassBinTimeSortedTranslator(this->LogName, this->ParserName));
		break;
	case CAEN_BIN_SINGLE_CHANNEL:
		this->console = spdlog::get(this->LogName)->clone("CAEN_BIN_SINGLE_CHANNEL_Parser");
		this->DataTranslator.reset(new CompassBinSingleChannelTranslator(this->LogName, this->ParserName));
		break;
	case CAEN_ROOT:
	case PLD:
	case Unknown:
	default:
		throw std::runtime_error("UNKNOWN PARSER OF TYPE " + this->ParserName);
	}
}

void DataParser::SetInputFiles(std::vector<std::string>& filelist) {
	for (const auto& file : filelist) {
		if (not this->DataTranslator->AddFile(file)) {
			throw std::runtime_error("Unable to Add File : " + file + " to the Translator");
		}
	}
	this->DataTranslator->FinalizeFiles();
}

Translator::TRANSLATORSTATE DataParser::Parse(boost::container::devector<PhysicsData>& RawEvents) {
	return this->DataTranslator->Parse(RawEvents);
}

void DataParser::SetChannelMap(const std::shared_ptr<ChannelMap>& cmap) {
	this->CMap = cmap;
	this->DataTranslator->SetChannelMap(cmap);
}

void DataParser::SetCorrelator(const std::shared_ptr<Correlator>& corr) {
	this->correlator = corr;
	this->DataTranslator->SetCorrelator(corr);
}
