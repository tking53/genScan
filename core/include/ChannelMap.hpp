#ifndef __CHANNEL_MAP_HPP__
#define __CHANNEL_MAP_HPP__

#include <string>
#include <vector>
#include <set>

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>

#include "PhysicsData.hpp"

class XiaDecoder;

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

		enum FirmwareVersion{
			R17562,
			R20466,
			R27361,
			R29432,
			R30474,
			R30980,
			R30981,
			R35207,
			R34688,
			PHA,
			PSD,
			UNKNOWN
		};

		struct BoardInfo{
			char Revision;
			int Frequency;
			int BoardIDInCrate;
			int CrateID;
			int GlobalBoardID;
			int TraceDelay;
			FirmwareVersion Version;
			XiaDecoder* xiadecoder;

			template<typename OStream>
			friend OStream& operator<<(OStream& os, const ChannelMap::BoardInfo& b) {
				os << "Crate: " << b.CrateID
				   << " Board: " << b.BoardIDInCrate
				   << " gBoard: " << b.GlobalBoardID
				   << " Revision: " << b.Revision
				   << " Frequency: " << b.Frequency
				   << " TraceDelay: " << b.TraceDelay;
				switch( b.Version ){
					case R17562:
						os << " Version: R17562";
						break;
					case R20466:
						os << " Version: R20466";
						break;
					case R27361:
						os << " Version: R27361";
						break;
					case R29432:
						os << " Version: R29432";
						break;
					case R30474:
						os << " Version: R30474";
						break;
					case R30980:
						os << " Version: R30980";
						break;
					case R30981:
						os << " Version: R30981";
						break;
					case R35207:
						os << " Version: R35207";
						break;
					case R34688:
						os << " Version: R34688";
						break;
					case PHA:
						os << " Version: PHA";
						break;
					case PSD:
						os << " Version: PSD";
						break;
					default:
						os << " Version: UNKNOWN";
						break;
				}
				return os;
			}
		};

		struct ChannelInfo{
			CalType cal;
			int ChannelIDInBoard;
			int BoardIDInCrate;
			int CrateID;
			int GlobalChannelID;
			int TraceDelay;
			std::string type;
			std::string subtype;
			std::string group;
			std::string tags;
			std::set<std::string> taglist;
			std::string unique_id;
			std::vector<double> Params;
			std::pair<double,double> Thresh;

			template<typename OStream>
			friend OStream& operator<<(OStream& os, const ChannelMap::ChannelInfo& c) {
				os << "Crate: " << c.CrateID
				   << " Board: " << c.BoardIDInCrate
				   << " Channel: " << c.ChannelIDInBoard
				   << " gChannel: " << c.GlobalChannelID
				   << " TraceDelay: " << c.TraceDelay
				   << " Type: " << c.type
				   << " Subtype: " << c.subtype
				   << " Group: " << c.group
				   << " Tags: " << c.tags
				   << " UniqueID: " << c.unique_id
				   << " ParsedTags: ";
				for( const auto& t : c.taglist )
					os << t << ",";
				os << " CalType: ";
				switch(c.cal){
					case Linear:
						os << "Linear";
						break;
					case Quadratic:
						os << "Quadratic";
						break;
					case Cubic:
						os << "Cubic";
						break;
					case LinearExpo:
						os << "LinearExpo";
						break;
					case QuadraticExpo:
						os << "QuadraticExpo";
						break;
					default:
						os << "UNKNOWN";
						break;
				}
				os << " CalParams: ";
				for( const auto& p : c.Params )
					os << p << ",";
				os << " Threshold: [" << c.Thresh.first << "," << c.Thresh.second << "]";
				return os;
			}

		};
		
		ChannelMap(int mc,int mbpc,int mcpb,int mcppc);

		int GetNumBoards() const;
		int GetNumCrates() const;
		int GetNumChannelsPerBoard() const;


		[[nodiscard]] double GetCalibratedEnergy(int,int,int,double);
		
		[[nodiscard]] bool SetParams(int,int,int,const std::string&,const std::string&,const std::string&,const std::string&,const std::set<std::string>&,CalType,const std::vector<double>&,int,const std::pair<double,double>&);

		[[nodiscard]] CalType GetCalType(int,int,int) const;

		[[nodiscard]] int GetBoardFrequency(int,int) const;
		[[nodiscard]] int GetBoardTraceDelay(int,int) const;
		[[nodiscard]] ChannelMap::FirmwareVersion GetBoardFirmware(int,int) const;

		[[nodiscard]] bool SetBoardInfo(int,int,const char&,const std::string&,int,int);
		[[nodiscard]] bool SetModuleClockTickMap(const char&,int,int,int);
		[[nodiscard]] int GetModuleClockTicksToNS(int,int) const;
		[[nodiscard]] int GetModuleADCClockTicksToNS(int,int) const;
		[[nodiscard]] int GetModuleFilterClockTicksToNS(int,int) const;

		[[nodiscard]] int GetGlobalChanID(int,int,int) const;
		[[nodiscard]] int GetGlobalBoardID(int,int) const;

		[[nodiscard]] FirmwareVersion CalcFirmwareEnum(const std::string&) const;

		const boost::container::flat_map<int,BoardInfo>& GetBoardConfig() const;
		const boost::container::flat_map<int,ChannelInfo>& GetChannelConfig() const;

		void SetChanConfigInfo(PhysicsData&) const;

		XiaDecoder* GetXiaDecoder(int,int) const;

		void FinalizeChannelMap();

		[[nodiscard]] int GetMaxGCID() const;

	private:
		int MAX_CRATES;
		int MAX_BOARDS_PER_CRATE;
		int MAX_BOARDS;
		int MAX_CHANNELS_PER_BOARD;
		int MAX_CHANNELS;
		int MAX_CAL_PARAMS_PER_CHANNEL;
		int MAX_CAL_PARAMS;
		int MAX_FID;
		
		boost::container::flat_set<std::string> KnownUID;

		boost::container::flat_map<int,BoardInfo> BoardConfigMap;
		boost::container::flat_map<int,ChannelInfo> ChannelConfigMap;

		boost::container::flat_map<int,double> ModuleClockTickToNS;
		boost::container::flat_map<int,double> ModuleADCClockTickToNS;
		boost::container::flat_map<int,double> ModuleFilterClockTickToNS;

};

#endif
