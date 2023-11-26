#ifndef __CHANNEL_MAP_HPP__
#define __CHANNEL_MAP_HPP__

#include <string>
#include <vector>
#include <map>

class ChannelMap{
	public:
		enum CalType{
			Unknown,
			Linear,
			Quadratic,
			Cubic,
			LinearExpo,
			QuadraticExpo,
			CubicExpo
		};
		ChannelMap(int mc,int mbpc,int mcpb,int mcppc);

		int GetFid(int&,int&) const;
		int GetNumBoards() const;
		int GetNumChannelsPerBoard() const;
		int GetBoardFrequency(int&) const;
		int GetBoardTraceDelay(int&) const;
		std::string GetBoardFirmware(int&) const;
		CalType GetCalType(int&,int&) const;
		void SetParams(int&,int&,std::string,std::string,std::string,std::vector<std::string>,CalType,std::vector<double>&);
		void SetBoardInfo(int&,std::string,int,int);
		double GetCalibratedEnergy(int&,int&,double&);
	private:

		std::vector<CalType> Calibration;
		std::vector<double> Params;
		int MAX_CRATES;
		int MAX_BOARDS_PER_CRATE;
		int MAX_BOARDS;
		int MAX_CHANNELS_PER_BOARD;
		int MAX_CHANNELS;
		int MAX_CAL_PARAMS_PER_CHANNEL;
		int MAX_CAL_PARAMS;
		int MAX_FID;
		std::vector<std::string> type;
		std::vector<std::string> subtype;
		std::vector<std::string> group;
		std::vector<std::vector<std::string>> tags;
		std::vector<std::string> unique_id;
		
		std::vector<std::string> Firmware;
		std::vector<int> Frequency;
		std::vector<int> TraceDelay;
};

#endif
