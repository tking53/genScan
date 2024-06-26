#include "EventSummary.hpp"

#include "ProcessorList.hpp"

EventSummary::EventSummary(const std::string& log){
	this->ColonParse =  boost::regex(":");
	this->LogName = log;
	this->console = spdlog::get(this->LogName)->clone("EventSummary");
	this->CacheHits = 0;
	this->CacheMisses = 0;
	this->UIDCacheHits = 0;
	this->UIDCacheMisses = 0;
}

EventSummary::~EventSummary(){
	this->console->info("Cache Hits : {}, Cache Misses : {}",this->CacheHits,this->CacheMisses);
	this->console->info("UID Cache Hits : {}, UID Cache Misses : {}",this->UIDCacheHits,this->UIDCacheMisses);
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
		++(this->UIDCacheMisses);
		auto CacheCheck = this->Cache.find(rkey.str());
		if( CacheCheck == this->Cache.end() ){
			++(this->CacheMisses);
			boost::smatch type_match;
			//The regex cache has been missed, i.e. it isn't a default type list
			for( auto& evt : this->RawEvents ){
				if( boost::regex_match(evt.GetUniqueID(),type_match,rkey,boost::regex_constants::match_continuous) ){
					vec.push_back(&evt);
				}
			}
			this->Cache[rkey.str()] = vec;
		}else{
			++(this->CacheHits);
			vec = CacheCheck->second;
		}
	}else{
		++(this->UIDCacheHits);
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

boost::container::devector<PhysicsData>& EventSummary::GetRawEvents(){
	return this->RawEvents;
}

const std::set<std::string>& EventSummary::GetKnownTypes() const{
	return this->KnownTypes;
}

void EventSummary::InitMappedUIDs(const ChannelMap* cmap,const ProcessorList* proclist){
	auto config = cmap->GetChannelConfig();
	auto procs = proclist->GetProcessors();
	auto anals = proclist->GetAnalyzers();
	boost::smatch type_match;
	for( const auto& p : procs ){
		for( const auto& kv : p->GetAllDefaultRegex() ){
			auto def_regex = kv.second;
			this->MappedUIDs[def_regex.str()] = std::vector<bool>(cmap->GetMaxGCID(),false);
			for( const auto& kv : config ){
				if( boost::regex_match(kv.second.unique_id,type_match,def_regex,boost::regex_constants::match_continuous) ){
					this->MappedUIDs[def_regex.str()][kv.first] = true;
				}
			}
		}
	}
	for( const auto& a : anals ){
		for( const auto& kv : a->GetAllDefaultRegex() ){
			auto def_regex = kv.second;
			this->MappedUIDs[def_regex.str()] = std::vector<bool>(cmap->GetMaxGCID(),false);
			for( const auto& kv : config ){
				if( boost::regex_match(kv.second.unique_id,type_match,def_regex,boost::regex_constants::match_continuous) ){
					this->MappedUIDs[def_regex.str()][kv.first] = true;
				}
			}
		}
	}
}

void EventSummary::ClearRawEvents(){
	this->RawEvents.clear();
	this->KnownTypes.clear();
	this->Cache.clear();
}
