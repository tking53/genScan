#include <fstream>
#include <stdexcept>
#include <sstream>
#include <limits>
#include <regex>

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "JSONConfigParser.hpp"
#include "ChannelMap.hpp"
#include "StringManipFunctions.hpp"

JSONConfigParser::JSONConfigParser(const std::string& log) : ConfigParser(log){
}

void JSONConfigParser::Parse(ChannelMap* cmap){
	if( this->ConfigName == nullptr ){
		throw std::runtime_error("JSONConfigParser::Parse() SetConfigFile() has not been called");
	}

	finput = std::ifstream(*(this->ConfigName));
	if( finput.fail() ){
		std::stringstream ss;
		ss << "JSONConfigParser::Parse() : config file named \""
		   << *(this->ConfigName) 
		   << "\" Does not exist.";
		throw std::runtime_error(ss.str());

	}
	finput >> JsonDoc;

	this->Configuration = JsonDoc["Configuration"];
	if( !this->Configuration ){
		std::stringstream ss;
		ss << "JSONConfigParser::Parse() : config file named \""
		   << *(this->ConfigName) 
		   << "\" is malformed and there is no Configuration node.";
		throw std::runtime_error(ss.str());
	}

	spdlog::get(this->LogName)->info("Parsing Description tag");
	ParseDescription();
	spdlog::get(this->LogName)->info("Parsing Author tag");
	ParseAuthor();
	spdlog::get(this->LogName)->info("Parsing Global tag");
	ParseGlobal();
	spdlog::get(this->LogName)->info("Parsing DetectorDriver tag");
	ParseDetectorDriver();
	spdlog::get(this->LogName)->info("Parsing Map tag");
	ParseMap(cmap);
}
		
void JSONConfigParser::ParseDescription(){
	this->Description = this->Configuration["Description"];
	if( this->Description ){
		this->DescriptionText.reset(new std::string(this->Description.asString()));
	}
}

void JSONConfigParser::ParseAuthor(){
	this->Author = this->Configuration["Author"];
	if( this->Author ){
		if( this->Author["Name"] )
			this->AuthorNameText.reset(new std::string(this->Author["Name"].asString()));
		if( this->Author["Email"] )
			this->AuthorEmailText.reset(new std::string(this->Author["Email"].asString()));
		if( this->Author["Date"] )
			this->AuthorDateText.reset(new std::string(this->Author["Date"].asString()));
	}
}

void JSONConfigParser::ParseGlobal(){
	this->Global = this->Configuration["Global"];
	if( this->Global ){
		GlobalEventWidthInS = this->Global.get("EventWidth",-1.0).asDouble();
		if( GlobalEventWidthInS < 0.0 ){
			std::stringstream ss;
			ss << "JSONConfigParser::ParseGlobal() : config file named \""
		   	   << *(this->ConfigName) 
		   	   << "\" is malformed and Global node is missing EventWidth attribute. expect positive number";
			throw std::runtime_error(ss.str());
		}

		std::string GlobalEventWidthUnit = this->Global.get("EventWidthUnit","x").asString();
		if( GlobalEventWidthUnit.compare("ns") == 0 ){
			GlobalEventWidthInS *= 1.0e-9;
		}else if( GlobalEventWidthUnit.compare("us") == 0 ){
			GlobalEventWidthInS *= 1.0e-6;
		}else if( GlobalEventWidthUnit.compare("ms") == 0 ){
			GlobalEventWidthInS *= 1.0e-3;
		}else if( GlobalEventWidthUnit.compare("s") == 0 ){
			GlobalEventWidthInS *= 1.0;
		}else{
				std::stringstream ss;
				ss << "JSONConfigParser::ParseGlobal() : config file named \""
		   		   << *(this->ConfigName) 
		   		   << "\" is malformed and Global node is missing EventWidthUnit attribute. (s,ns,us,ms) are valid";
				throw std::runtime_error(ss.str());
		}
	}else{
		std::stringstream ss;
		ss << "JSONConfigParser::ParseGlobal() : config file named \""
		   << *(this->ConfigName) 
		   << "\" is malformed and Global node is missing.";
		throw std::runtime_error(ss.str());
	}
}

void JSONConfigParser::ParseDetectorDriver(){
	this->DetectorDriver = this->Configuration["DetectorDriver"];
	if( this->DetectorDriver ){

		Json::Value processor = this->DetectorDriver["Processor"];
		Json::Value analyzer = this->DetectorDriver["Analyzer"];
		if( (!processor) and (!analyzer) ){
			std::stringstream ss;
			ss << "JSONConfigParser::ParseDetectorDriver() : config file named \""
			   << *(this->ConfigName) 
			   << "\" is malformed. No Processors or Analyzers listed.";
			throw std::runtime_error(ss.str());
		}

		for(const auto& proc : processor ){
			std::string name = proc["name"].asString();
			this->ProcessorNames[name] = proc;
			AddProcessorName(name);
		}

		for(const auto& proc : analyzer ){
			std::string name = proc["name"].asString();
			this->AnalyzerNames[name] = proc;
			AddAnalyzerName(name);
		}
	}else{
		std::stringstream ss;
		ss << "JSONConfigParser::ParseDetectorDriver() : config file named \""
		   << *(this->ConfigName) 
		   << "\" is malformed because DetectorDriver tag is missing.";
		throw std::runtime_error(ss.str());
	}
}

void JSONConfigParser::ParseMap(ChannelMap* cmap){
	this->Map = this->Configuration["Map"];
	if( this->Map ){
		Json::Value board = this->Map["Module"];
		if( !board ){
			std::stringstream ss;
			ss << "JSONConfigParser::ParseMap() : config file named \""
			   << *(this->ConfigName) 
			   << "\" is most likely malformed, because there are no Module tags";
			throw std::runtime_error(ss.str());
		}
		for( const auto& currboard : board ){
			int bid = currboard.get("number",std::numeric_limits<int>::max()).asInt();
			if( bid == std::numeric_limits<int>::max() ){
				std::stringstream ss;
				ss << "JSONConfigParser::ParseMap() : config file named \""
				   << *(this->ConfigName) 
				   << "\" is most likely malformed, because one of the Module tags is missing the \"number\" attribute.";
				throw std::runtime_error(ss.str());
			}
			std::string firmware = currboard.get("Firmware","").asString();
			if( firmware.compare("") == 0 ){
				std::stringstream ss;
				ss << "JSONConfigParser::ParseMap() : config file named \""
				   << *(this->ConfigName) 
				   << "\" is most likely malformed, because module with number \""
				   << bid << "\" is missing firmware attribute.";
				throw std::runtime_error(ss.str());
			}
			int frequency = currboard.get("Frequency",-1).asInt();
			if( frequency < 0  ){
				std::stringstream ss;
				ss << "JSONConfigParser::ParseMap() : config file named \""
				   << *(this->ConfigName) 
				   << "\" is most likely malformed, because module with number \""
				   << bid << "\" is either missing Frequency attribute or is negative";
				throw std::runtime_error(ss.str());
			}
			int TraceDelay = currboard.get("TraceDelay",-1).asInt();
			if( TraceDelay < 0 ){
				std::stringstream ss;
				ss << "JSONConfigParser::ParseMap() : config file named \""
				   << *(this->ConfigName) 
				   << "\" is most likely malformed, because module with number \""
				   << bid << "\" is either missing TraceDelay attribute or is negative";
				throw std::runtime_error(ss.str());
			}
			cmap->SetBoardInfo(bid,firmware,frequency,TraceDelay);

			Json::Value channel = currboard["Channel"];
			if( !channel ){
				std::stringstream ss;
				ss << "JSONConfigParser::ParseMap() : config file named \""
				   << *(this->ConfigName) 
				   << "\" is most likely malformed, because There are no Channel tags in Board : "
				   << bid;
				throw std::runtime_error(ss.str());
			}
			for( const auto& currchannel : channel ){
				int cid = currchannel.get("number",std::numeric_limits<int>::max()).asInt();
				if( cid == std::numeric_limits<int>::max() ){
					std::stringstream ss;
					ss << "JSONConfigParser::ParseMap() : config file named \""
					   << *(this->ConfigName) 
					   << "\" is most likely malformed, because one of the Channel tags in Board : "
					   << bid << " is missing the \"number\" attribute.";
					throw std::runtime_error(ss.str());
				}else{
					std::string type = currchannel.get("type","").asString();
					if( type.compare("") == 0 ){
						std::stringstream ss;
						ss << "JSONConfigParser::ParseMap() : config file named \""
						   << *(this->ConfigName) 
						   << "\" is most likely malformed, because no type is listed for channel with number=\""
						   << cid << "\" in module with number=\""
						   << bid << "\"";
						throw std::runtime_error(ss.str());
					}
					std::string subtype = currchannel.get("subtype","").asString();
					if( subtype.compare("") == 0 ){
						std::stringstream ss;
						ss << "JSONConfigParser::ParseMap() : config file named \""
						   << *(this->ConfigName) 
						   << "\" is most likely malformed, because no subtype is listed for channel with number=\""
						   << cid << "\" in module with number=\""
						   << bid << "\"";
						throw std::runtime_error(ss.str());
					}
					std::string group = currchannel.get("group","").asString();
					if( group.compare("") == 0 ){
						std::stringstream ss;
						ss << "JSONConfigParser::ParseMap() : config file named \""
						   << *(this->ConfigName) 
						   << "\" is most likely malformed, because no group is listed for channel with number=\""
						   << cid << "\" in module with number=\""
						   << bid << "\"";
						throw std::runtime_error(ss.str());
					}
					std::vector<std::string> taglist = {};
					std::string tags = currchannel.get("tags","").asString();
					//regex to parse out tags
					std::regex word_regex("(\\w+)");
					auto words_begin =std::sregex_iterator(tags.begin(),tags.end(), word_regex);
					auto words_end = std::sregex_iterator();
					for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
						std::smatch match = *i;
						taglist.push_back(match.str());
					}
				
					Json::Value calibration = currchannel["Calibration"];
					if( !calibration ){
						std::stringstream ss;
						ss << "JSONConfigParser::ParseMap() : config file named \""
						   << *(this->ConfigName) 
						   << "\" is malformed. Because no Calibration tag exists for channel with number=\""
						   << cid << "\" in module with number=\""
						   << bid << "\"";
						throw std::runtime_error(ss.str());
					}else{
						std::string cal_type = calibration.get("model","").asString();
						if( cal_type.compare("") == 0 ){
							std::stringstream ss;
							ss << "JSONConfigParser::ParseMap() : config file named \""
							   << *(this->ConfigName) 
							   << "\" is malformed. Because Calibration tag for channel with number=\""
							   << cid << "\" in module with number=\""
							   << bid << "\" is missing the \"model\" attribute";
							throw std::runtime_error(ss.str());
						}
						
						std::vector<double> params;
						std::string calstring = calibration.get("value","0.0 1.0").asString();
						ChannelMap::CalType ct = ChannelMap::CalType::Unknown;
						StringManip::ParseCalString(calstring,params);
						if( params.size() == 0 ){
							std::stringstream ss;
							ss << "JSONConfigParser::ParseMap() : config file named \""
							   << *(this->ConfigName) 
							   << "\" is malformed. Because Calibration tag for channel with number=\""
							   << cid << "\" in module with number=\""
							   << bid << "\" is missing the \"text\" containing the calibration parameters";
							throw std::runtime_error(ss.str());
						}

						if (cal_type.compare("linear") == 0 ){
							if( params.size() != 2 ){
								std::stringstream ss;
								ss << "JSONConfigParser::ParseMap() : config file named \""
								   << *(this->ConfigName) 
								   << "\" is malformed. Because Calibration tag for channel with number=\""
								   << cid << "\" in module with number=\""
								   << bid << "\" is model=\"linear\" which expects 2 parameters but has "
								   << params.size() << " listed.";
								throw std::runtime_error(ss.str());
							}
							ct = ChannelMap::CalType::Linear;
						}else if (cal_type.compare("quadratic") == 0 ){
							if( params.size() != 3 ){
								std::stringstream ss;
								ss << "JSONConfigParser::ParseMap() : config file named \""
								   << *(this->ConfigName) 
								   << "\" is malformed. Because Calibration tag for channel with number=\""
								   << cid << "\" in module with number=\""
								   << bid << "\" is model=\"quadratic\" which expects 3 parameters but has "
								   << params.size() << " listed.";
								throw std::runtime_error(ss.str());
							}
							ct = ChannelMap::CalType::Quadratic;
						}else if (cal_type.compare("cubic") == 0 ){
							if( params.size() != 4 ){
								std::stringstream ss;
								ss << "JSONConfigParser::ParseMap() : config file named \""
								   << *(this->ConfigName) 
								   << "\" is malformed. Because Calibration tag for channel with number=\""
								   << cid << "\" in module with number=\""
								   << bid << "\" is model=\"cubic\" which expects 4 parameters but has "
								   << params.size() << " listed.";
								throw std::runtime_error(ss.str());
							}
							ct = ChannelMap::CalType::Cubic;
						}else if (cal_type.compare("linear_expo") == 0 ){
							if( params.size() != 2 ){
								std::stringstream ss;
								ss << "JSONConfigParser::ParseMap() : config file named \""
								   << *(this->ConfigName) 
								   << "\" is malformed. Because Calibration tag for channel with number=\""
								   << cid << "\" in module with number=\""
								   << bid << "\" is model=\"linear_expo\" which expects 2 parameters but has "
								   << params.size() << " listed.";
								throw std::runtime_error(ss.str());
							}
							ct = ChannelMap::CalType::LinearExpo;
						}else if (cal_type.compare("quadratic_expo") == 0 ){
							if( params.size() != 3 ){
								std::stringstream ss;
								ss << "JSONConfigParser::ParseMap() : config file named \""
								   << *(this->ConfigName) 
								   << "\" is malformed. Because Calibration tag for channel with number=\""
								   << cid << "\" in module with number=\""
								   << bid << "\" is model=\"quadratic_expo\" which expects 3 parameters but has "
								   << params.size() << " listed.";
								throw std::runtime_error(ss.str());
							}
							ct = ChannelMap::CalType::QuadraticExpo;
						}else if (cal_type.compare("cubic_expo") == 0 ){
							if( params.size() != 4 ){
								std::stringstream ss;
								ss << "JSONConfigParser::ParseMap() : config file named \""
								   << *(this->ConfigName) 
								   << "\" is malformed. Because Calibration tag for channel with number=\""
								   << cid << "\" in module with number=\""
								   << bid << "\" is model=\"cubic_expo\" which expects 4 parameters but has "
								   << params.size() << " listed.";
								throw std::runtime_error(ss.str());
							}
							ct = ChannelMap::CalType::CubicExpo;
						}else{
							std::stringstream ss;
							ss << "JSONConfigParser::ParseMap() : config file named \""
							   << *(this->ConfigName) 
							   << "\" is malformed. Because Calibration tag for channel with number=\""
							   << cid << "\" in module with number=\""
							   << bid << "\" has calibration type of model=\""
							   << cal_type << "\"";
							throw std::runtime_error(ss.str());
						}
						cmap->SetParams(bid,cid,type,subtype,group,taglist,ct,params); 
					}
				}
			}
		}
		spdlog::get(this->LogName)->info("Here is the ChannelMap Info we were able to parse");
		for( int ii = 0; ii < cmap->GetNumBoards(); ++ii ){
			auto freq = cmap->GetBoardFrequency(ii);
			auto firm = cmap->GetBoardFirmware(ii);
			auto tdelay = cmap->GetBoardTraceDelay(ii);
			if( freq > 0 ){
				spdlog::get(this->LogName)->info("Found Board with Info : [Board Number : {},Frequency : {}, Firmware : {}, TraceDelay : {}]",ii,freq,firm,tdelay);
				for( int jj = 0; jj < cmap->GetNumChannelsPerBoard(); ++jj ){
					auto ct = cmap->GetCalType(ii,jj);
					if( ct != ChannelMap::CalType::Unknown ){
						spdlog::get(this->LogName)->info("Found Channel With Info : [Board Number : {}, Channel Number : {},CalType : {}]",ii,jj,ct);
					}
				}
			}
		}
	}else{
		//throw error
		std::stringstream ss;
		ss << "JSONConfigParser::ParseMap() : config file named \""
		   << *(this->ConfigName) 
		   << "\" is malformed. Map node is missing.";
		throw std::runtime_error(ss.str());
	}
}

Json::Value JSONConfigParser::GetProcessorJSONInfo(const std::string& name) const{
	return this->ProcessorNames.at(name);
}

Json::Value JSONConfigParser::GetAnalyzerJSONInfo(const std::string& name) const{
	return this->AnalyzerNames.at(name);
}
