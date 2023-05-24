#include "ChannelMap.hpp"


ChannelMap* ChannelMap::instance = nullptr; 

ChannelMap* ChannelMap::Get(){
	if( instance == nullptr ){
		instance = new ChannelMap();
	}
	return instance;
}

ChannelMap::ChannelMap(){
}

bool ChannelMap::AddBoardInfo(unsigned long long bid,std::string firmware,int frequency,int tracedelay){
	if( bid >= Boards.size() ){
		//board does not exist but don't want to overwrite already read in boards
		for( size_t ii = Boards.size(); ii < bid+1; ++ii ){
			Boards.push_back(nullptr);
		}
	}
	if( Boards.at(bid) == nullptr ){
		Boards.at(bid) = new BoardNode(bid,firmware,frequency,tracedelay);
		return true;
	}else{
		return false;
	}
}

ChannelNode::ChannelNode(unsigned long long board_id,unsigned long long channel_id, std::string det_type,std::string det_subtype,std::string det_group,std::vector<std::string> det_tags){
	bid = board_id;
	cid = channel_id;
	type = det_type;
	subtype = det_subtype;
	group = det_group;
	tags = det_tags;
	unique_id = type + ":" + subtype + ":" + group;
	for( auto& t : tags )
		unique_id += ":" + t;
}

double ChannelNode::GetCalibratedEnergy(double& erg){
	return calibrator->Calibrate(erg);
}

BoardNode::BoardNode(unsigned long long board_id,std::string firmware,int frequency,int tracedelay){
	id = board_id;
	Firmware = firmware;
	Frequency = frequency;
	TraceDelay = tracedelay;
}

bool BoardNode::AddChannel(unsigned long long cid,std::string t,std::string st,std::string g,std::vector<std::string> ts){
	if( cid >= Channels.size() ){
		//channel does not exist but don't want to overwrite already existing ones
		for( size_t ii = Channels.size(); ii < cid+1; ++ii ){
			Channels.push_back(nullptr);
		}
	}
	if( Channels.at(cid) != nullptr ){
		return false;
	}else{
		Channels.at(cid) = new ChannelNode(id,cid,t,st,g,ts);
		return true;
	}
}

size_t BoardNode::GetNumChannels() const{
	return Channels.size();
}

ChannelNode* BoardNode::GetSingleChannel(unsigned long long cid){
	return Channels.at(cid);
}

std::vector<ChannelNode*> BoardNode::GetChannels(){
	return Channels;
}

double BoardNode::GetCalibratedEnergy(unsigned long& cid,double& erg){
	return Channels.at(cid)->GetCalibratedEnergy(erg);
}

bool ChannelMap::AddChannel(unsigned long long bid,unsigned long long cid,std::string t,std::string st,std::string g,std::vector<std::string> ts){
	return Boards.at(bid)->AddChannel(cid,t,st,g,ts);
}

ChannelNode* ChannelMap::GetSingleChannel(unsigned long long bid,unsigned long long cid){
	return Boards.at(bid)->GetSingleChannel(cid);
}

size_t ChannelMap::GetNumBoards() const{
	return Boards.size();
}

std::vector<BoardNode*> ChannelMap::GetBoards(){
	return Boards;
}

void ChannelMap::GenerateLookupTables(){
	for( auto& b : Boards ){
		for( auto& c : b->GetChannels() ){
			if( c != nullptr ){
				TypeLookupChart[c->type].push_back(nullptr);
				TypeLookupChart[c->type].back() = c;

				SubTypeLookupChart[c->subtype].push_back(nullptr);
				SubTypeLookupChart[c->subtype].back() = c;

				GroupLookupChart[c->group].push_back(nullptr);
				GroupLookupChart[c->group].back() = c;

				for( auto& t : c->tags ){
					TagLookupChart[t].push_back(nullptr);
					TagLookupChart[t].back() = c;
				}

			}
		}
	}
}

double ChannelMap::GetCalibratedEnergy(unsigned long& bid,unsigned long& cid,double& erg){
	return Boards.at(bid)->GetCalibratedEnergy(cid,erg);
}

std::map<std::string,std::vector<ChannelNode*>> ChannelMap::GetTypeLookupChart(){
	return TypeLookupChart;
}

std::map<std::string,std::vector<ChannelNode*>> ChannelMap::GetSubTypeLookupChart(){
	return SubTypeLookupChart;
}

std::map<std::string,std::vector<ChannelNode*>> ChannelMap::GetGroupLookupChart(){
	return GroupLookupChart;
}

std::map<std::string,std::vector<ChannelNode*>> ChannelMap::GetTagLookupChart(){
	return TagLookupChart;
}
