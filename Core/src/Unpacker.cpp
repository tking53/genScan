#include "Unpacker.hpp"


void Unpacker::SetCurrFile(std::string* cf){
	currfile = cf;
}

std::string* Unpacker::GetCurrFile(){
	return currfile;
}
