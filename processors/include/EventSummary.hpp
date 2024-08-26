#ifndef __EVENT_SUMMARY_HPP__
#define __EVENT_SUMMARY_HPP__

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

#include "ChannelMap.hpp"
#include "PhysicsData.hpp"

class ProcessorList;

class EventSummary{
	public:
		EventSummary(const std::string&);
		~EventSummary();
	
		void InitMappedUIDs(const ChannelMap*,const ProcessorList*);	
		void BuildDetectorSummary();
		void GetDetectorTypeSummary(const std::string&,std::vector<PhysicsData*>&);
		void GetDetectorSummary(const std::string&,std::vector<PhysicsData*>&);
		void GetDetectorSummary(const boost::regex&,std::vector<PhysicsData*>&);

		boost::container::devector<PhysicsData>& GetRawEvents();
		void ClearRawEvents();

		const std::set<std::string>& GetKnownTypes() const;

		PhysicsData* GetDetectorMaxEvent(const std::vector<PhysicsData*>&) const;

	private:
		std::string LogName;
		std::shared_ptr<spdlog::logger> console;

		boost::container::devector<PhysicsData> RawEvents;
		std::set<std::string> KnownTypes;
		boost::container::flat_map<std::string,std::vector<bool>> MappedUIDs;
		boost::regex ColonParse;
		unsigned long long UIDCacheHits;
		unsigned long long UIDCacheMisses;
		boost::container::flat_map<std::string,std::vector<PhysicsData*>> Cache;
		unsigned long long CacheHits;
		unsigned long long CacheMisses;
};

#endif
