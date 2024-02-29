#include "EventSummary.hpp"
#include <regex>

EventSummary::EventSummary(){
	this->ColonParse =  boost::regex(":");
}

void EventSummary::BuildDetectorSummary(){
	for( auto& evt : this->RawEvents ){
		this->KnownTypes.insert(std::string(evt.GetType()));
	}
}

void EventSummary::GetDetectorSummary(const boost::regex& rkey,std::vector<PhysicsData*>& vec){
	vec.clear();
	//std::sregex_iterator end;
	boost::smatch type_match;
	for( auto& evt : this->RawEvents ){
		if( boost::regex_match(evt.GetUniqueID(),type_match,rkey) ){
			vec.push_back(&evt);
		}
		//std::string unique_id(evt.GetUniqueID());
		//std::sregex_iterator keysearch(unique_id.begin(),unique_id.end(),rkey);
		//if( keysearch != end ){
		//	vec.push_back(&evt);
		//}
	}
}

void EventSummary::GetDetectorSummary(const std::string& key,std::vector<PhysicsData*>& vec){
	boost::regex rkey(key);
	this->GetDetectorSummary(rkey,vec);
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
