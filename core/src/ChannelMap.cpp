#include <exception>
#include <cmath>
#include <stdexcept>
#include <string>

#include "ChannelMap.hpp"

#include "BitDecoder.hpp"

ChannelMap::ChannelMap(int mc,int mbpc,int mcpb,int mcppc){
	MAX_CRATES = mc;
	MAX_BOARDS_PER_CRATE = mbpc;
	MAX_BOARDS = MAX_BOARDS_PER_CRATE*MAX_CRATES;

	MAX_CHANNELS_PER_BOARD = mcpb;
	MAX_CHANNELS = MAX_CHANNELS_PER_BOARD*MAX_BOARDS;
	MAX_FID = this->GetGlobalChanID(MAX_CRATES,MAX_BOARDS,MAX_CHANNELS_PER_BOARD);

	if( mcppc < 4 ){
		throw std::runtime_error("ChannelMap::ChannelMap() supplied with less than 4 calibration parameters per channel");
	}
	MAX_CAL_PARAMS_PER_CHANNEL = mcppc;
	MAX_CAL_PARAMS = MAX_CAL_PARAMS_PER_CHANNEL*MAX_CHANNELS;
}

ChannelMap::FirmwareVersion ChannelMap::CalcFirmwareEnum(const std::string& type) const{
	auto VERSION = ChannelMap::FirmwareVersion::UNKNOWN;
	unsigned int firmwareNumber = 0;

	if( type.compare("PHA") == 0 ){
		VERSION = PHA;
	}else if( type.compare("PSD") == 0 ){
		VERSION = PSD;
	}else{

		if( type.find("R") == 0 || type.find("r") == 0 ){
			std::string tmp(type.begin() + 1, type.end());
			firmwareNumber = (unsigned int)atoi(tmp.c_str());
		}else{
			firmwareNumber = (unsigned int)atoi(type.c_str());
		}

		if( firmwareNumber >= 17562 && firmwareNumber < 20466 ){
			VERSION = R17562;
		}else if( firmwareNumber >= 20466 && firmwareNumber < 27361 ){
			VERSION = R20466;
		}else if( firmwareNumber >= 27361 && firmwareNumber < 29432 ){
			VERSION = R27361;
		}else if( firmwareNumber >= 29432 && firmwareNumber < 30474 ){
			VERSION = R29432;
		}else if( firmwareNumber >= 30474 && firmwareNumber < 30980 ){
			VERSION = R30474;
		}else if( firmwareNumber >= 30980 && firmwareNumber < 30981 ){
			VERSION = R30980;
		}else if( firmwareNumber >= 30981 && firmwareNumber < 34688 ){
			VERSION = R30981;
		}else if( firmwareNumber == 35207 ){  
			//14b 500MHz RevF
			//TODO we are hard matching this one because we are unsure of its structure. once we have some time we should see if its any different than the 34688->42950 structure (it shouldnt be)
			VERSION = R35207; 
		}else if( firmwareNumber >= 34688 && firmwareNumber <= 42950 ){  
			//42950 is is the newest 16b:250 VERSION, received at the end of August 2019
			VERSION = R34688;
		}else{
			throw std::runtime_error("Unknown Firmware Version : "+type);
		}
	}
	return VERSION;
}

[[nodiscard]] int ChannelMap::GetGlobalChanID(int CrateNum,int ModNum,int ChanNum) const{
	return this->MAX_BOARDS_PER_CRATE*this->MAX_CHANNELS_PER_BOARD*CrateNum + this->MAX_CHANNELS_PER_BOARD*ModNum + ChanNum;
}

[[nodiscard]] int ChannelMap::GetGlobalBoardID(int CrateNum,int ModNum) const{
	return this->MAX_BOARDS_PER_CRATE*CrateNum + ModNum;
}

[[nodiscard]] bool ChannelMap::SetModuleClockTickMap(const char& Revision,int Frequency,int CrateNum,int ModNum){
	auto GlobalModNum = this->GetGlobalBoardID(CrateNum,ModNum);
	if( Revision == 'F' ){
		if( Frequency == 100 ){
			auto retval1 = this->ModuleClockTickToNS.insert_or_assign(GlobalModNum,10);
			auto retval2 = this->ModuleADCClockTickToNS.insert_or_assign(GlobalModNum,10);
			auto retval3 = this->ModuleFilterClockTickToNS.insert_or_assign(GlobalModNum,10);
			return !(retval1.second and retval2.second and retval3.second);
		}else if( Frequency == 500 ){
			auto retval1 = this->ModuleClockTickToNS.insert_or_assign(GlobalModNum,10);
			auto retval2 = this->ModuleADCClockTickToNS.insert_or_assign(GlobalModNum,2);
			auto retval3 = this->ModuleFilterClockTickToNS.insert_or_assign(GlobalModNum,10);
			return !(retval1.second and retval2.second and retval3.second);
		}else{
			auto retval1 = this->ModuleClockTickToNS.insert_or_assign(GlobalModNum,8);
			auto retval2 = this->ModuleADCClockTickToNS.insert_or_assign(GlobalModNum,4);
			auto retval3 = this->ModuleFilterClockTickToNS.insert_or_assign(GlobalModNum,8);
			return !(retval1.second and retval2.second and retval3.second);
		}
	}else if( Revision == 'H' ){
		if( Frequency == 500 ){
			auto retval1 = this->ModuleClockTickToNS.insert_or_assign(GlobalModNum,8);
			auto retval2 = this->ModuleADCClockTickToNS.insert_or_assign(GlobalModNum,2);
			auto retval3 = this->ModuleFilterClockTickToNS.insert_or_assign(GlobalModNum,8);
			return !(retval1.second and retval2.second and retval3.second);
		}else{
			auto retval1 = this->ModuleClockTickToNS.insert_or_assign(GlobalModNum,8);
			auto retval2 = this->ModuleADCClockTickToNS.insert_or_assign(GlobalModNum,8);
			auto retval3 = this->ModuleFilterClockTickToNS.insert_or_assign(GlobalModNum,8);
			return !(retval1.second and retval2.second and retval3.second);
		}
	}else{
		auto retval1 = this->ModuleClockTickToNS.insert_or_assign(GlobalModNum,10);
		auto retval2 = this->ModuleADCClockTickToNS.insert_or_assign(GlobalModNum,10);
		auto retval3 = this->ModuleFilterClockTickToNS.insert_or_assign(GlobalModNum,10);
		return !(retval1.second and retval2.second and retval3.second);
	}
}

[[nodiscard]] int ChannelMap::GetModuleClockTicksToNS(int CrateNum,int ModNum) const{
	return this->ModuleClockTickToNS.at(this->GetGlobalBoardID(CrateNum,ModNum));
}

[[nodiscard]] int ChannelMap::GetModuleADCClockTicksToNS(int CrateNum,int ModNum) const{
	return this->ModuleADCClockTickToNS.at(this->GetGlobalBoardID(CrateNum,ModNum));
}

[[nodiscard]] int ChannelMap::GetModuleFilterClockTicksToNS(int CrateNum,int ModNum) const{
	return this->ModuleFilterClockTickToNS.at(this->GetGlobalBoardID(CrateNum,ModNum));
}

[[nodiscard]] bool ChannelMap::SetParams(int crid,int bid,int cid,const std::string& t,const std::string& st,const std::string& g,const std::string& tt,const std::set<std::string>& tg,CalType c,const std::vector<double>& p,int tdelay,const std::pair<double,double>& thresh){
	if( static_cast<int>(p.size()) > MAX_CAL_PARAMS_PER_CHANNEL ){
		throw std::runtime_error("Trying to assign more calibration parameters than allowed");
	}else{
		auto gcid = this->GetGlobalChanID(crid,bid,cid);
		auto gbid = this->GetGlobalBoardID(crid,bid);
		if( (gcid >= MAX_FID) or (gbid >= MAX_BOARDS) or (cid >= MAX_CHANNELS_PER_BOARD) ){
			std::string mess = "Invalid config file, Crate : "+std::to_string(crid)+"Board : "+std::to_string(bid)+" Channel : "+std::to_string(cid)+" Is Invalid";
			throw std::runtime_error(mess);
		}
		std::string currunique_id = t + ":" + st + ":" + g;
		for( auto& currtag : tg )
			currunique_id += ":" + currtag;
		auto result = this->KnownUID.insert_unique(currunique_id);
		if( not result.second ){
			throw std::runtime_error("Channel : "+std::to_string(cid)+
					         " Board : "+std::to_string(bid)+
						 " Crate : "+std::to_string(crid)+
						 " type:subtype:group:(tags) : "+currunique_id+
						 " has duplicate type:subtype:group:(tags) as another");
		}

		ChannelInfo CurrChannelInfo = {
			.cal = c,
			.ChannelIDInBoard = cid,
			.BoardIDInCrate = bid,
			.CrateID = crid,
			.GlobalChannelID = gcid,
			.TraceDelay = tdelay, 
			.type = t,
			.subtype = st,
			.group = g,
			.tags = tt,
			.taglist = tg,
			.unique_id = currunique_id,
		        .Params = p,
			.Thresh = thresh	
		};
		auto retval = this->ChannelConfigMap.insert_or_assign(gcid,CurrChannelInfo);
		return !retval.second; 
	}
}

double ChannelMap::GetCalibratedEnergy(int crid,int bid,int cid,double erg){
	auto gcid = this->GetGlobalChanID(crid,bid,cid);
	auto gbid = this->GetGlobalBoardID(crid,bid);
	if( (gcid >= MAX_FID) or (gbid >= MAX_BOARDS) or (cid >= MAX_CHANNELS_PER_BOARD) ){
		std::string mess = "Invalid config file, Crate : "+std::to_string(crid)+"Board : "+std::to_string(bid)+" Channel : "+std::to_string(cid)+" Is Invalid";
		throw std::runtime_error(mess);
	}
	auto c = ChannelConfigMap.at(gcid);
	//if( erg < c.Thresh.first or erg > c.Thresh.second ){
	//	return 0.0;
	//}
	switch(c.cal){
		case Linear:
			return c.Params.at(0) + c.Params.at(1)*erg; 
			break;
		case Quadratic:
			return c.Params.at(0) + c.Params.at(1)*erg + c.Params.at(2)*erg*erg; 
			break;
		case Cubic:
			return c.Params.at(0) + c.Params.at(1)*erg + c.Params.at(2)*erg*erg + c.Params.at(3)*erg*erg*erg; 
			break;
		case LinearExpo:
			return std::exp(c.Params.at(0) + c.Params.at(1)*erg); 
			break;
		case QuadraticExpo:
			return std::exp(c.Params.at(0) + c.Params.at(1)*erg + c.Params.at(2)*erg*erg); 
			break;
		case CubicExpo:
			return std::exp(c.Params.at(0) + c.Params.at(1)*erg + c.Params.at(2)*erg*erg + c.Params.at(3)*erg*erg*erg); 
			break;
		case Unknown:
		default:
			throw std::runtime_error("Unused channel");
			break;
	}
}

bool ChannelMap::SetBoardInfo(int crid,int bid,const char& rev,const std::string& firm,int freq,int tdelay){
	if( bid >= MAX_BOARDS ){
		std::string mess = "Board ID : "+std::to_string(bid)+" exceeds the maximum number of boards : "+std::to_string(MAX_BOARDS);
		throw std::runtime_error(mess);
	}else{
		BoardInfo CurrInfo = {
			.Revision = rev,
		        .Frequency = freq,
			.BoardIDInCrate = bid,
			.CrateID = crid,
			.GlobalBoardID = this->GetGlobalBoardID(crid,bid),
			.TraceDelay = tdelay,
			.Version = this->CalcFirmwareEnum(firm),
			.xiadecoder = new XiaDecoder(this->CalcFirmwareEnum(firm),freq)
		};
		auto retval = this->BoardConfigMap.insert_or_assign(CurrInfo.GlobalBoardID,CurrInfo); 
		return !retval.second;
	}
}

int ChannelMap::GetNumBoards() const{
	return MAX_BOARDS;
}

int ChannelMap::GetNumChannelsPerBoard() const{
	return MAX_CHANNELS_PER_BOARD;
}

int ChannelMap::GetNumCrates() const{
	return MAX_CRATES;
}

[[nodiscard]] ChannelMap::CalType ChannelMap::GetCalType(int crid,int bid,int cid) const{
	auto gcid = this->GetGlobalChanID(crid,bid,cid);
	auto gbid = this->GetGlobalBoardID(crid,bid);
	if( (gcid >= MAX_FID) or (gbid >= MAX_BOARDS) or (cid >= MAX_CHANNELS_PER_BOARD) ){
		std::string mess = "Invalid config file, Crate : "+std::to_string(crid)+"Board : "+std::to_string(bid)+" Channel : "+std::to_string(cid)+" Is Invalid";
		throw std::runtime_error(mess);
	}
	return ChannelConfigMap.at(gcid).cal;
}
		
[[nodiscard]] int ChannelMap::GetBoardFrequency(int crid,int bid) const{
	auto gbid = this->GetGlobalBoardID(crid,bid);
	if( gbid >= MAX_BOARDS ){
		std::string mess = "Board ID : "+std::to_string(gbid)+" exceeds the maximum number of boards : "+std::to_string(MAX_BOARDS);
		throw std::runtime_error(mess);
	}
	return BoardConfigMap.at(gbid).Frequency;
}

[[nodiscard]] int ChannelMap::GetBoardTraceDelay(int crid,int bid) const{
	auto gbid = this->GetGlobalBoardID(crid,bid);
	if( gbid >= MAX_BOARDS ){
		std::string mess = "Board ID : "+std::to_string(gbid)+" exceeds the maximum number of boards : "+std::to_string(MAX_BOARDS);
		throw std::runtime_error(mess);
	}
	return BoardConfigMap.at(gbid).TraceDelay;
}

[[nodiscard]] ChannelMap::FirmwareVersion ChannelMap::GetBoardFirmware(int crid,int bid) const{
	auto gbid = this->GetGlobalBoardID(crid,bid);
	if( gbid >= MAX_BOARDS ){
		std::string mess = "Board ID : "+std::to_string(gbid)+" exceeds the maximum number of boards : "+std::to_string(MAX_BOARDS);
		throw std::runtime_error(mess);
	}
	return BoardConfigMap.at(gbid).Version;
}
		
const boost::container::flat_map<int,ChannelMap::BoardInfo>& ChannelMap::GetBoardConfig() const{
	return this->BoardConfigMap;
}

const boost::container::flat_map<int,ChannelMap::ChannelInfo>& ChannelMap::GetChannelConfig() const{
	return this->ChannelConfigMap;
}

void ChannelMap::FinalizeChannelMap(){
	for( const auto& currboard : this->BoardConfigMap ){
		auto duplicate = this->SetModuleClockTickMap(currboard.second.Revision,currboard.second.Frequency,currboard.second.CrateID,currboard.second.BoardIDInCrate);
		if( duplicate ){
			throw std::runtime_error("Unable to finalize the Channel Map due to duplicate board parameters found in Crate "+std::to_string(currboard.second.CrateID)+" and board "+std::to_string(currboard.second.BoardIDInCrate));
		}
	}
	//this->KnownUID.clear();
}

XiaDecoder* ChannelMap::GetXiaDecoder(int crid,int bid) const{
	return this->BoardConfigMap.at(this->GetGlobalBoardID(crid,bid)).xiadecoder;
}
		
[[nodiscard]] int ChannelMap::GetMaxGCID() const{
	return this->MAX_FID;
}

void ChannelMap::SetChanConfigInfo(PhysicsData& evt) const{
	auto currmap = this->ChannelConfigMap.at(evt.GetGlobalChannelID());
	evt.SetType(currmap.type);
	evt.SetSubType(currmap.subtype);
	evt.SetGroup(currmap.group);
	evt.SetTags(currmap.tags);
	evt.SetTagList(currmap.taglist);
	evt.SetUniqueID(currmap.unique_id);
}
