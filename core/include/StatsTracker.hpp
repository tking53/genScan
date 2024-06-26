#ifndef __STATS_TRACKER_HPP__
#define __STATS_TRACKER_HPP__

#include <string>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <boost/container/flat_map.hpp>
#include <boost/container/devector.hpp>

#include "ChannelMap.hpp"
#include "PhysicsData.hpp"

class StatsTracker{
	public:
		StatsTracker(const std::string&);
		~StatsTracker();

		void Init(ChannelMap*);
		void IncrementStats(const boost::container::devector<PhysicsData>&);

		struct StatsObject{
			unsigned long long Hits = 0;
			unsigned long long PileupHits = 0;
			unsigned long long SaturationHits = 0;

			void IncrementHit(){
				++Hits;
			}

			void IncrementPileup(){
				++PileupHits;
			}

			void IncrementSaturation(){
				++SaturationHits;
			}
		};
	private:
		std::string LogName;

		std::shared_ptr<spdlog::logger> console;
		
		boost::container::flat_map<int,StatsObject> StatsInfo;
};

#endif
