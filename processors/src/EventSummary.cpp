#include "EventSummary.hpp"
#include <regex>

EventSummary::EventSummary(){
	this->ColonParse =  std::regex(":");
}

void EventSummary::BuildDetectorSummary(){
	for( auto& evt : this->RawEvents ){
		this->KnownTypes.insert(std::string(evt.GetType()));
	}
}

void EventSummary::GetDetectorSummary(const std::string& key,std::vector<PhysicsData*>& vec){
	vec.clear();
	std::regex rkey(key);
	std::sregex_iterator end;
	for( auto& evt : this->RawEvents ){
		std::string unique_id(evt.GetUniqueID());
		std::sregex_iterator keysearch(unique_id.begin(),unique_id.end(),rkey);
		if( keysearch != end ){
			vec.push_back(&evt);
		}
	}
}

boost::container::devector<PhysicsData>& EventSummary::GetRawEvents(){
	return this->RawEvents;
}

const std::set<std::string>& EventSummary::GetKnownTypes() const{
	return this->KnownTypes;
}

void EventSummary::ClearRawEvents(){
	this->RawEvents.clear();
	this->KnownTypes.clear();
}
