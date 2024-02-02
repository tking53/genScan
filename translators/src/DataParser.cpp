#include <stdexcept>
#include <string>
#include <vector>

#include "DataParser.hpp"

#include "EVTTranslator.hpp"
#include "LDFTranslator.hpp"

DataParser::DataParser(DataParser::DataFileType dft,const std::string& log){
	DataType = dft;
	this->LogName = log;
	switch(DataType){
		case EVT_BUILT:
			this->ParserName = "EVT_BUILT";
			break;
		case CAEN_ROOT:
			this->ParserName = "CAEN_ROOT";
			break;
		case CAEN_BIN:
			this->ParserName = "CAEN_BIN";
			break;
		case LDF:
			this->ParserName = "LDF";
			break;
		case PLD:
			this->ParserName = "PLD";
			break;
		case EVT_PRESORT:
			this->ParserName = "EVT_Presort";
			break;
		case Unknown:
		default:
			this->ParserName = "UNKNOWN";
			break;
	}

	switch(DataType){
		case EVT_BUILT:
			console = spdlog::get(this->LogName)->clone("EVT_BUILT_Parser");
			DataTranslator.reset(new EVTTranslator(this->LogName,this->ParserName,EVTTranslator::EVT_TYPE::EVTBUILT));
			break;
		case CAEN_ROOT:
		case CAEN_BIN:
		case LDF:
			console = spdlog::get(this->LogName)->clone("LDF_PIXIE");
			DataTranslator.reset(new LDFTranslator(this->LogName,this->ParserName,LDFTranslator::LDF_TYPE::PIXIE));
			break;
		case PLD:
		case EVT_PRESORT:
			console = spdlog::get(this->LogName)->clone("EVT_Presort_Parser");
			DataTranslator.reset(new EVTTranslator(this->LogName,this->ParserName,EVTTranslator::EVT_TYPE::PRESORT));
			break;
		case Unknown:
		default:
			throw std::runtime_error("UNKNOWN PARSER OF TYPE "+this->ParserName);
	}
}

void DataParser::SetInputFiles(std::vector<std::string>& filelist){
	for(const auto& file : filelist){
		if( not DataTranslator->AddFile(file) ){
			throw std::runtime_error("Unable to Add File : "+file+" to the Translator");
		}
	}
	DataTranslator->FinalizeFiles();
}

void DataParser::Parse(boost::container::devector<PhysicsData>& RawEvents){
	DataTranslator->Parse(RawEvents);
}
