#include "Unpacker.hpp"

Unpacker::Unpacker(){
	f_stats = new FileStats;
	c_stats = new CoincidenceStats;
	#ifdef USE_SPDLOG 
		LogFile = spdlog::basic_logger_mt("Unpacker","Unpacker.log",true);
		LogFile->set_pattern("%v");
	#endif
}

Unpacker::~Unpacker(){
	//need to output the info from all the things
	delete f_stats;
	delete c_stats;
}

void Unpacker::SetCurrFile(std::string* cf){
	currfile = cf;
}

std::string* Unpacker::GetCurrFile(){
	return currfile;
}
