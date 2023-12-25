#include "PLDInfo.hpp"
#include <cstring>
#include <fstream>

PLDHeader::PLDHeader() : DataBuffer(1145128264,0){
	run_title = nullptr;
	this->Reset();
}

unsigned int PLDHeader::GetBufferLength() const{
	//modulo  add
	//0       0    (4 - 0) -> 0  
	//1       3    (4-3)
	//2       2    (4-2)
	//3       1    (4-1)
	
	unsigned int title_len = 100 + strlen(run_title);
	unsigned int title_len_mod = title_len%4;
	return ((title_len_mod!=0)?(title_len+(4-title_len_mod)):title_len);

	//unsigned int buffer_len = 100 + strlen(run_title);
	//while( (buffer_len%4) ){
	//	++buffer_len;
	//}
	//return buffer_len;
}

const char* PLDHeader::GetFacility() const{
	return facility;
}

const char* PLDHeader::GetFormat() const{
	return format;
}

const char* PLDHeader::GetStartDate() const{
	return start_date;
}

const char* PLDHeader::GetEndDate() const{
	return end_date;
}

const char* PLDHeader::GetRunTitle() const{
	return run_title;
}

unsigned int PLDHeader::GetRunNumber() const{
	return run_num;
}

unsigned int PLDHeader::GetMaxSpillSize() const{
	return max_spill_size;
}

float PLDHeader::GetRunTime() const{
	return run_time;
}

void PLDHeader::SetStartDateTime(){
	std::time(&runStartTime);

	char* date = std::ctime(&runStartTime);
	set_char_array(std::string(date),start_date,24);
}

void PLDHeader::SetEndDateTime(){
	std::time(&runStopTime);

	run_time = static_cast<float>(std::difftime(runStopTime,runStartTime));

	char* date = std::ctime(&runStopTime);
	set_char_array(std::string(date),end_date,24);
}

void PLDHeader::SetFacility(std::string& fac){
	set_char_array(fac,facility,16);
}

void PLDHeader::SetTitle(std::string& tit){
	if( run_title != nullptr )
		delete[] run_title;
	run_title = new char[tit.size() + 1];
	for( unsigned int ii = 0; ii < tit.size(); ++ii )
		run_title[ii] = tit[ii];
	run_title[tit.size()] = '\0';
}

bool PLDHeader::Write(std::ofstream& file){
	if( !file or !file.is_open() or !file.good()){
		return false;
	}else{
		//finish writing from here 
		//using hribf_buffers to implement pld
		return true;
	}
}
