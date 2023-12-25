#include <stdexcept>

#include "DataParser.hpp"

DataParser* DataParser::instance = nullptr;

DataParser* DataParser::Get(){
	return instance;
}

DataParser* DataParser::Get(DataParser::DataFileType dft){
	if( instance == nullptr ){
		instance = new DataParser(dft);
	}else{
		throw std::runtime_error("Shouldn't call this method");
	}
	return instance;
}


DataParser::DataParser(){
	DataType = DataFileType::Unknown;
}

DataParser::DataParser(DataParser::DataFileType dft){
	DataType = dft;
}
