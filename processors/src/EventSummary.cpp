#include "EventSummary.hpp"

EventSummary::EventSummary(const std::string& log){
	this->ColonParse =  boost::regex(":");
	this->LogName = log;
	this->console = spdlog::get(this->LogName)->clone("EventSummary");
	this->CacheHits = 0;
	this->CacheMisses = 0;
}

EventSummary::~EventSummary(){
	this->console->info("Cache Hits : {}, Cache Misses : {}",this->CacheHits,this->CacheMisses);
}

void EventSummary::BuildDetectorSummary(){
	for( auto& evt : this->RawEvents ){
		this->KnownTypes.insert(std::string(evt.GetType()));
	}
}

void EventSummary::GetDetectorSummary(const boost::regex& rkey,std::vector<PhysicsData*>& vec){
	auto CacheCheck = this->Cache.find(rkey.str());
	if( CacheCheck == this->Cache.end() ) [[likely]] {
		//this->console->info("No Cached info for : {}",rkey.str());
		++(this->CacheMisses);
		vec.clear();
		//std::sregex_iterator end;
		boost::smatch type_match;
		for( auto& evt : this->RawEvents ){
			if( boost::regex_match(evt.GetUniqueID(),type_match,rkey,boost::regex_constants::match_continuous) ){
				vec.push_back(&evt);
			}
			//std::string unique_id(evt.GetUniqueID());
			//std::sregex_iterator keysearch(unique_id.begin(),unique_id.end(),rkey);
			//if( keysearch != end ){
			//	vec.push_back(&evt);
			//}
		}
		this->Cache[rkey.str()] = vec;
	}else{
		//this->console->info("Cached info found : {}",rkey.str());
		++(this->CacheHits);
		vec = CacheCheck->second;
	}
}

void EventSummary::GetDetectorSummary(const std::string& key,std::vector<PhysicsData*>& vec){
	//need to assume the regex is correct
	boost::regex rkey(key);
	this->GetDetectorSummary(rkey,vec);
}

void EventSummary::GetDetectorTypeSummary(const std::string& key,std::vector<PhysicsData*>& vec){
	//need to assume the regex is correct
	boost::regex rkey(key+":.*");
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
	this->Cache.clear();
}
