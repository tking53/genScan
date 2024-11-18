#include "EventSummary.hpp"
#include "EventHistoryManager.hpp"

EventSummary::EventSummary(EventHistoryManager* hismanager,const boost::container::flat_map<std::string,std::vector<bool>>& table){
	this->ColonParse =  boost::regex(":");
	this->parent = hismanager;
	this->MappedUIDs = table; 
}

void EventSummary::BuildDetectorSummary(){
	for( auto& evt : this->RawEvents ){
		this->KnownTypes.insert(std::string(evt.GetType()));
	}
}

void EventSummary::GetDetectorSummary(const boost::regex& rkey,std::vector<PhysicsData*>& vec){
	vec.clear();
	auto UIDCacheCheck = this->MappedUIDs.find(rkey.str());
	if( UIDCacheCheck == this->MappedUIDs.end() ){
		this->parent->IncrementUIDCacheMisses();
		auto CacheCheck = this->Cache.find(rkey.str());
		if( CacheCheck == this->Cache.end() ){
			this->parent->IncrementCacheMisses();
			boost::smatch type_match;
			//The regex cache has been missed, i.e. it isn't a default type list
			for( auto& evt : this->RawEvents ){
				if( boost::regex_match(evt.GetUniqueID(),type_match,rkey,boost::regex_constants::match_continuous) ){
					vec.push_back(&evt);
				}
			}
			this->Cache[rkey.str()] = vec;
		}else{
			this->parent->IncrementCacheHits();
			vec = CacheCheck->second;
		}
	}else{
		this->parent->IncrementUIDCacheHits();
		for( auto& evt : this->RawEvents ){
			if( UIDCacheCheck->second[evt.GetGlobalChannelID()] ){
				vec.push_back(&evt);
			}
		}
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

PhysicsData* EventSummary::GetDetectorMaxEvent(const std::vector<PhysicsData*>& evtlist) const{
	PhysicsData* maxevt = nullptr;
	double maxerg = 0.0;
	for( const auto& evt : evtlist ){
		if( evt->GetEnergy() > maxerg ){
			maxevt = evt;
			maxerg = evt->GetEnergy();
		}
	}
	return maxevt;
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

void EventSummary::AddEventTag(const std::string& tag){
	this->EventTags.insert(tag);
}

bool EventSummary::ContainsEventTag(const std::string& tag) const{
	return this->EventTags.find(tag) != this->EventTags.end();
}

void EventSummary::AddEventObservable(const std::string& name,const double& obs){
	this->EventObservable[name] = obs;
}

std::optional<double> EventSummary::GetEventObservable(const std::string& name) const{
	auto search = this->EventObservable.find(name);
	if( search == this->EventObservable.end() ){
		return std::nullopt;
	}else{
		return std::optional<double>(search->second);
	}
}
