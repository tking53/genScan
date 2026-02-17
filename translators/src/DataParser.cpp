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
	case CAEN_BIN:
		this->ParserName = "CAEN_BIN";
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
	case CAEN_ROOT:
	case CAEN_BIN:
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
