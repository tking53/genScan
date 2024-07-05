
#include "StatsTracker.hpp"
#include "ChannelMap.hpp"
#include "PhysicsData.hpp"

StatsTracker::StatsTracker(const std::string& logname){
	this->LogName = logname;
	this->console = spdlog::get(this->LogName)->clone("StatsTracker");
	this->console->info("Created StatsTracker");
}

StatsTracker::~StatsTracker(){
	this->console->debug("Stats for global channel id (gChanID) | Hits | Saturate | Pileup ");
	for( const auto& it : this->StatsInfo ){
		this->console->debug("{} | {} | {} | {}",it.first,it.second.Hits,it.second.SaturationHits,it.second.PileupHits);
	}
}

void StatsTracker::Init(ChannelMap* cmap){
	auto chaninfo  = cmap->GetChannelConfig();
	for( const auto& it : chaninfo ){
		StatsObject curr;
		this->StatsInfo[it.first] = curr;
	}
}

void StatsTracker::IncrementStats(const boost::container::devector<PhysicsData>& data){
	for( const auto& d : data ){
		this->StatsInfo[d.GetGlobalChannelID()].IncrementHit();
		if( d.GetSaturation() )
			this->StatsInfo[d.GetGlobalChannelID()].IncrementSaturation();
		if( d.GetPileup() )
			this->StatsInfo[d.GetGlobalChannelID()].IncrementPileup();
	}
}
