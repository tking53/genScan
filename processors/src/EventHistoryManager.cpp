#include "EventHistoryManager.hpp"
#include "ChannelMap.hpp"
#include "EventSummary.hpp"
#include "ProcessorList.hpp"
#include <stdexcept>

EventHistoryManager::EventHistoryManager(const std::string& log, size_t histsize) {
	this->MaxHistorySize = histsize;
	this->History = boost::circular_buffer<EventSummary>(this->MaxHistorySize);
	this->LogName = log;
	this->console = spdlog::get(this->LogName)->clone("EventHistoryManager");
	this->ColonParse = boost::regex(":");
	this->CacheHits = 0;
	this->CacheMisses = 0;
	this->UIDCacheHits = 0;
	this->UIDCacheMisses = 0;
	this->EventCount = 0;
	this->CurrHistorySize = 0;
}

unsigned long long EventHistoryManager::GetEventCount() const {
	return this->EventCount;
}

EventHistoryManager::~EventHistoryManager() {
	this->History.clear();
	this->console->info("Received {} Events", this->EventCount);
	this->console->info("Cache Hits : {}, Cache Misses : {}", this->CacheHits, this->CacheMisses);
	this->console->info("UID Cache Hits : {}, UID Cache Misses : {}", this->UIDCacheHits, this->UIDCacheMisses);
}

EventSummary* EventHistoryManager::GetCurrentEventSummary() {
	return &(this->History.at(0));
}

EventSummary* EventHistoryManager::GetPreviousEventSummary(size_t offset) {
	if (offset > this->History.size()) {
		throw std::runtime_error("Requested Previous EventSummary is larger than number of events currently stored");
	}
	return &(this->History.at(offset));
	// offset = 0 is the current event
}

EventSummary* EventHistoryManager::GetOldestEventSummary() {
	return &(this->History.back());
}

void EventHistoryManager::InitMappedUIDs(const ChannelMap* cmap, const ProcessorList* proclist) {
	auto config = cmap->GetChannelConfig();
	auto procs = proclist->GetProcessors();
	auto anals = proclist->GetAnalyzers();
	boost::smatch type_match;
	for (const auto& p : procs) {
		for (const auto& kv : p->GetAllDefaultRegex()) {
			auto def_regex = kv.second;
			this->MappedUIDs[def_regex.str()] = std::vector<bool>(cmap->GetMaxGCID(), false);
			for (const auto& kv : config) {
				if (boost::regex_match(kv.second.unique_id, type_match, def_regex, boost::regex_constants::match_continuous)) {
					this->MappedUIDs[def_regex.str()][kv.first] = true;
				}
			}
		}
	}
	for (const auto& a : anals) {
		for (const auto& kv : a->GetAllDefaultRegex()) {
			auto def_regex = kv.second;
			this->MappedUIDs[def_regex.str()] = std::vector<bool>(cmap->GetMaxGCID(), false);
			for (const auto& kv : config) {
				if (boost::regex_match(kv.second.unique_id, type_match, def_regex, boost::regex_constants::match_continuous)) {
					this->MappedUIDs[def_regex.str()][kv.first] = true;
				}
			}
		}
	}
}

void EventHistoryManager::RotateBuffer() {
	this->History.push_front(EventSummary(this, this->MappedUIDs));
	++(this->EventCount);
	this->CurrHistorySize = (this->EventCount > this->MaxHistorySize) ? this->MaxHistorySize : this->CurrHistorySize + 1;
}

bool EventHistoryManager::IsCurrentEventSummaryEmpty() {
	return this->History.at(0).GetRawEvents().empty();
}

void EventHistoryManager::BuildCurrentEventDetectorSummary() {
	this->History.at(0).BuildDetectorSummary();
	this->History.at(0).AddEventObservable("Event_idx", this->EventCount);
}

size_t EventHistoryManager::GetMaxHistorySize() const {
	return this->MaxHistorySize;
}

size_t EventHistoryManager::GetMaxHistoryID() const {
	return this->CurrHistorySize;
}

void EventHistoryManager::SetVeryFirstTime(double ts) {
	this->FirstTime = ts;
}

double EventHistoryManager::GetVeryFirstTime() const {
	return this->FirstTime;
}
