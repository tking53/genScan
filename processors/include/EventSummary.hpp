#ifndef __EVENT_SUMMARY_HPP__
#define __EVENT_SUMMARY_HPP__

#include <set>
#include <string>
#include <map>
#include <optional>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/container/flat_map.hpp>
#include <boost/container/devector.hpp>
#include <boost/regex.hpp>

#include "PhysicsData.hpp"

class EventHistoryManager;

class EventSummary{
	public:
		EventSummary(EventHistoryManager*,const boost::container::flat_map<std::string,std::vector<bool>>&);
		~EventSummary() = default;
	
		void BuildDetectorSummary();
		void GetDetectorTypeSummary(const std::string&,std::vector<PhysicsData*>&);
		void GetDetectorSummary(const std::string&,std::vector<PhysicsData*>&);
		void GetDetectorSummary(const boost::regex&,std::vector<PhysicsData*>&);

		boost::container::devector<PhysicsData>& GetRawEvents();
		void ClearRawEvents();

		void AddEventTag(const std::string&);
		bool ContainsEventTag(const std::string&) const;

		void AddEventObservable(const std::string&,const double&);
		std::optional<double> GetEventObservable(const std::string&) const;

		const std::set<std::string>& GetKnownTypes() const;

		PhysicsData* GetDetectorMaxEvent(const std::vector<PhysicsData*>&) const;

	private:
		EventHistoryManager* parent;
		std::set<std::string> EventTags;
		std::map<std::string,double> EventObservable;

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
