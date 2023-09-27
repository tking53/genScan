#include "DataBuffer.hpp"
#include <fstream>

DataBuffer::DataBuffer(unsigned int bt,unsigned int bs){
	bufftype = bt;
	buffsize = bs;
	buffend = 0xFFFFFFFF;
	zero = 0;
	debug_mode = false;
	Reset();
}

DataBuffer::DataBuffer(unsigned int bt,unsigned int bs,unsigned int be){
	bufftype = bt;
	buffsize = bs;
	buffend = be;
	zero = 0;
	debug_mode = false;
	Reset();
}

bool DataBuffer::Write(std::ofstream& out){
	return false;
}

bool DataBuffer::Read(std::ifstream& in){
	return false;
}

void DataBuffer::Reset(){
}

unsigned int DataBuffer::GetBufferType() const{
	return bufftype;
}

unsigned int DataBuffer::GetBufferSize() const{
	return buffsize;
}

unsigned int DataBuffer::GetBufferEndFlag() const{
	return buffend;
}

bool DataBuffer::IsDebugMode() const{
	return debug_mode;
}

void DataBuffer::SetDebugMode(bool d){
	debug_mode = d;
}

bool DataBuffer::ReadHeader(std::ifstream& in) const{
	unsigned int check_bufftype;
	in.read((char*)&check_bufftype,4);
	return (check_bufftype == bufftype);
}
