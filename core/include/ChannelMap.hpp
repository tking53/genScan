#ifndef __CHANNEL_MAP_HPP__
#define __CHANNEL_MAP_HPP__

#include <string>
#include <vector>
#include <map>

#include "Calibration.hpp"

class ChannelNode{
	public:
		ChannelNode(unsigned long long,unsigned long long,std::string,std::string,std::string,std::vector<std::string>);
		double GetCalibratedEnergy(double&);

		template<typename OStream>
		friend OStream& operator<<(OStream& os, const ChannelNode& c){
			os << c.bid << " " << c.cid << " " << c.type << " " << c.subtype << " "  << c.group << " ";
			for( auto& t : c.tags){
				os << t << " ";
			}
			os << " " << c.unique_id;
			return os;
		}

		unsigned long long bid;
		unsigned long long cid;
		std::string type;
		std::string subtype;
		std::string group;
		std::vector<std::string> tags;
		std::string unique_id;
		Calibration* calibrator;
		unsigned long fid;
};

class BoardNode{
	public:
		BoardNode(unsigned long long,std::string,int,int);
		bool AddChannel(unsigned long long,std::string,std::string,std::string,std::vector<std::string>);
		size_t GetNumChannels() const;
		ChannelNode* GetSingleChannel(unsigned long long);
		std::vector<ChannelNode*> GetChannels();
		
		double GetCalibratedEnergy(unsigned long&,double&);
		unsigned long long GetFlatID(unsigned long long&);

		template<typename OStream>
		friend OStream& operator<<(OStream& os,const BoardNode& b){
			os << b.id << " " << b.Firmware << " " << b.Frequency << " " << b.TraceDelay;
			return os;
		}
	private:
		unsigned long long id;
		std::string Firmware;
		int Frequency;
		int TraceDelay;
		std::vector<ChannelNode*> Channels;
};

class ChannelMap{
	public:
		bool AddBoardInfo(unsigned long long,std::string,int,int);
		bool AddChannel(unsigned long long,unsigned long long,std::string,std::string,std::string,std::vector<std::string>);
		ChannelNode* GetSingleChannel(unsigned long long bid,unsigned long long cid);
		std::vector<BoardNode*> GetBoards();
		size_t GetNumBoards() const;

		double GetCalibratedEnergy(unsigned long&,unsigned long&,double&);

		unsigned long long GenerateLookupTables();
		std::map<std::string,std::vector<ChannelNode*>> GetTypeLookupChart();
		std::map<std::string,std::vector<ChannelNode*>> GetSubTypeLookupChart();
		std::map<std::string,std::vector<ChannelNode*>> GetGroupLookupChart();
		std::map<std::string,std::vector<ChannelNode*>> GetTagLookupChart();
		static ChannelMap* Get();

		void InitializeRawHistograms();
		unsigned long long GetFlatID(unsigned long long&,unsigned long long&);
	private: 
		static ChannelMap* instance;
		ChannelMap();
		std::vector<BoardNode*> Boards;
		std::map<std::string,std::vector<ChannelNode*>> TypeLookupChart;
		std::map<std::string,std::vector<ChannelNode*>> SubTypeLookupChart;
		std::map<std::string,std::vector<ChannelNode*>> GroupLookupChart;
		std::map<std::string,std::vector<ChannelNode*>> TagLookupChart;
		unsigned long long max_flatid;
};


#endif
