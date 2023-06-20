#include <exception>
#include <cmath>

#include "ChannelMap.hpp"

ChannelMap* ChannelMap::instance = nullptr;

ChannelMap* ChannelMap::Get(){
	if( instance == nullptr ){
		throw std::runtime_error("ChannelMap::Get() ChannelMap is not initialized yet");
	}
	return instance;
}

ChannelMap* ChannelMap::Get(int mc,int mbpc,int mcpb, int mcppc){
	if( instance == nullptr ){
		instance = new ChannelMap(mc,mbpc,mcpb,mcppc);
	}
	return instance;
}

ChannelMap::ChannelMap(int mc,int mbpc,int mcpb,int mcppc){
	MAX_CRATES = mc;
	MAX_BOARDS_PER_CRATE = mbpc;
	MAX_BOARDS = MAX_BOARDS_PER_CRATE*MAX_CRATES;

	MAX_CHANNELS_PER_BOARD = mcpb;
	MAX_CHANNELS = MAX_CHANNELS_PER_BOARD*MAX_BOARDS;

	if( mcppc < 4 ){
		throw std::runtime_error("ChannelMap::ChannelMap() supplied with less than 4 calibration parameters per channel");
	}
	MAX_CAL_PARAMS_PER_CHANNEL = mcppc;
	MAX_CAL_PARAMS = MAX_CAL_PARAMS_PER_CHANNEL*MAX_CHANNELS;

	Calibration = std::vector<CalType>(MAX_CHANNELS,CalType::Unknown);
	type = std::vector<std::string>(MAX_CHANNELS,"THIS IS A LONG STRING TO PREVENT OPTIMIZATION");
	subtype = std::vector<std::string>(MAX_CHANNELS,"THIS IS A LONG STRING TO PREVENT OPTIMIZATION");
	group = std::vector<std::string>(MAX_CHANNELS,"THIS IS A LONG STRING TO PREVENT OPTIMIZATION");
	tags = std::vector<std::vector<std::string>>(MAX_CHANNELS,{"THIS IS A LONG STRING TO PREVENT OPTIMIZATION","THIS IS A LONG STRING TO PREVENT OPTIMIZATION"});
	unique_id = std::vector<std::string>(MAX_CHANNELS,"THIS IS A LONG STRING TO PREVENT OPTIMIZATION");
	Params = std::vector<double>(MAX_CAL_PARAMS,0.0);

	Firmware = std::vector<std::string>(MAX_BOARDS,"THIS IS A LONG STRING TO PREVENT OPTIMIZATION");
	Frequency = std::vector<int>(MAX_BOARDS,-1);
	TraceDelay = std::vector<int>(MAX_BOARDS,-1);
}

int ChannelMap::GetFid(int& bid,int& cid) const{
	return bid*MAX_CHANNELS_PER_BOARD + cid;
}

void ChannelMap::SetParams(int& bid,int& cid,std::string t,std::string st,std::string g,std::vector<std::string> tg,CalType c,std::vector<double>& p){
	if( p.size() > MAX_CAL_PARAMS_PER_CHANNEL ){
		throw std::runtime_error("Trying to assign more calibration parameters than allowed");
	}else{
		auto fid = GetFid(bid,cid);
		Calibration.at(fid) = c;
		type.at(fid) = t;
		subtype.at(fid) = st;
		group.at(fid) = g;
		tags.at(fid) = tg;
		unique_id.at(fid) = type.at(fid) + ":" + subtype.at(fid) + ":" + group.at(fid);
		for( auto& tt : tags.at(fid) )
			unique_id.at(fid) += ":" + tt;

		for( int ii = 0; ii < p.size(); ++ii ){
			Params.at(fid*3 + ii) = p.at(ii);
		}
	}
}
double ChannelMap::GetCalibratedEnergy(int& bid,int& cid,double& erg){
	auto fid = GetFid(bid,cid);
	auto c = Calibration.at(fid);
	switch(c){
		case Linear:
			return Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL) + Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL + 1)*erg; 
			break;
		case Quadratic:
			return Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL) + Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL + 1)*erg + Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL + 2)*erg*erg; 
			break;
		case Cubic:
			return Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL) + Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL + 1)*erg + Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL + 2)*erg*erg + Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL + 3)*erg*erg*erg; 
			break;
		case LinearExpo:
			return std::exp(Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL) + Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL + 1)*erg); 
			break;
		case QuadraticExpo:
			return std::exp(Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL) + Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL + 1)*erg + Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL + 2)*erg*erg); 
			break;
		case CubicExpo:
			return std::exp(Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL) + Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL + 1)*erg + Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL + 2)*erg*erg + Params.at(fid*MAX_CAL_PARAMS_PER_CHANNEL + 3)*erg*erg*erg); 
			break;
		case Unknown:
		default:
			throw std::runtime_error("Unused channel");
			break;
	}
}

void ChannelMap::SetBoardInfo(int& bid,std::string firm,int freq,int tdelay){
	Firmware.at(bid) = firm;
	Frequency.at(bid) = freq;
	TraceDelay.at(bid) = tdelay;
}

int ChannelMap::GetNumBoards() const{
	return MAX_BOARDS;
}

int ChannelMap::GetNumChannelsPerBoard() const{
	return MAX_CHANNELS_PER_BOARD;
}

ChannelMap::CalType ChannelMap::GetCalType(int& bid,int& cid) const{
	return Calibration.at(GetFid(bid,cid));
}
		
int ChannelMap::GetBoardFrequency(int& bid) const{
	return Frequency.at(bid);
}

int ChannelMap::GetBoardTraceDelay(int& bid) const{
	return TraceDelay.at(bid);
}

std::string ChannelMap::GetBoardFirmware(int& bid) const{
	return Firmware.at(bid);
}
