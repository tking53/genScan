#ifndef __GENERAL_STATS_HPP__
#define __GENERAL_STATS_HPP__

#include <map>
#include <chrono>

struct ChannelStats{
	unsigned int NumHits;
	unsigned int NumTraceSaturate;
	unsigned int NumFilterSaturate;
	unsigned int NumPileup;
};

struct FileStats{
	unsigned int NumHits;
	std::chrono::time_point<std::chrono::high_resolution_clock> StartTime;	
	std::chrono::time_point<std::chrono::high_resolution_clock> StopTime;	
};

struct CoincidenceStats{
	unsigned int NumBuilt;
	double CoincidenceTime;
};

#endif
