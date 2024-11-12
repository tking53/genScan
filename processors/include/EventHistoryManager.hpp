#ifndef __EVENT_HISTORY_MANAGER_HPP__
#define __EVENT_HISTORY_MANAGER_HPP__

#include <set>
#include <string>
#include <tuple>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>


#include <boost/container/flat_map.hpp>
#include <boost/container/devector.hpp>
#include <boost/regex.hpp>
#include <boost/circular_buffer.hpp>

#include "ChannelMap.hpp"
#include "PhysicsData.hpp"
#include "EventSummary.hpp"

class ProcessorList;
class EventSummary;

class EventHistoryManager{
	public:
		EventHistoryManager(const std::string&,size_t);
		~EventHistoryManager();

		void InitMappedUIDs(const ChannelMap*,const ProcessorList*);

		unsigned long long GetEventCount() const;

		void RotateBuffer();

		EventSummary* GetCurrentEventSummary();
		bool IsCurrentEventSummaryEmpty();
		void BuildCurrentEventDetectorSummary();

		EventSummary* GetPreviousEventSummary(size_t);

		inline void IncrementUIDCacheHits(){
			++(this->UIDCacheHits);
		}

		inline void IncrementUIDCacheMisses(){
			++(this->UIDCacheMisses);
		}

		inline void IncrementCacheHits(){
			++(this->CacheHits);
		}

		inline void IncrementCacheMisses(){
			++(this->CacheMisses);
		}
		
	private:
		std::string LogName;
		std::shared_ptr<spdlog::logger> console;
		size_t MaxHistorySize;

		unsigned long long EventCount;

		boost::circular_buffer<EventSummary> History;

		boost::container::flat_map<std::string,std::vector<bool>> MappedUIDs;
		boost::regex ColonParse;
		unsigned long long UIDCacheHits;
		unsigned long long UIDCacheMisses;
		unsigned long long CacheHits;
		unsigned long long CacheMisses;

};

#endif
