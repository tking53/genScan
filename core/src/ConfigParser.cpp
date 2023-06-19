#include <stdexcept>
#include <sstream>
#include <limits>
#include <regex>

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "ConfigParser.hpp"
#include "ChannelMap.hpp"

#include "Calibration.hpp"
#include "LinearCalibration.hpp"
#include "QuadraticCalibration.hpp"
#include "CubicCalibration.hpp"
#include "PolyCalibration.hpp"
#include "LinearExpoCalibration.hpp"
#include "QuadraticExpoCalibration.hpp"
#include "CubicExpoCalibration.hpp"
#include "PolyExpoCalibration.hpp"

ConfigParser* ConfigParser::instance = nullptr;

ConfigParser* ConfigParser::Get(){
	if( instance == nullptr ){
		instance = new ConfigParser();
	}
	return instance;
}

ConfigParser::ConfigParser(){
	this->XMLName = nullptr;
}

void ConfigParser::SetConfigFile(std::string* filename){
	this->XMLName = filename;
}

void ConfigParser::Parse(){
	if( this->XMLName == nullptr ){
		throw std::runtime_error("ConfigParser::Parse() SetConfigFile() has not been called");
	}

	pugi::xml_parse_result result = this->XMLDoc.load_file(this->XMLName->c_str());

	if( !result ){
		std::stringstream ss;
		ss << "ConfigParser::Parse() : Unable to open xml file named \""
		   << *(this->XMLName) 
		   << "\". pugixml reports : "
		   << result.description();
		throw std::runtime_error(ss.str());
	}
	this->Configuration = this->XMLDoc.child("Configuration");
	if( !this->Configuration ){
		std::stringstream ss;
		ss << "ConfigParser::Parse() : config file named \""
		   << *(this->XMLName) 
		   << "\" is malformed and there is no Configuration node.";
		throw std::runtime_error(ss.str());
	}

	spdlog::get("genscan")->info("Parsing Description tag");
	ParseDescription();
	spdlog::get("genscan")->info("Parsing Author tag");
	ParseAuthor();
	spdlog::get("genscan")->info("Parsing Global tag");
	ParseGlobal();
	spdlog::get("genscan")->info("Parsing DetectorDriver tag");
	ParseDetectorDriver();
	spdlog::get("genscan")->info("Parsing Map tag");
	ParseMap();
}
		
void ConfigParser::ParseDescription(){
	this->Description = this->Configuration.child("Description");
	if( this->Description ){
		this->DescriptionText = new std::string(this->Description.text().get());
	}
}

void ConfigParser::ParseAuthor(){
	this->Author = this->Configuration.child("Author");
	if( this->Author ){
		if( this->Author.child("Name") )
			AuthorNameText = new std::string(this->Author.child("Name").text().get());
		if( this->Author.child("Email") )
			AuthorEmailText = new std::string(this->Author.child("Email").text().get());
		if( this->Author.child("Date") )
			AuthorDateText = new std::string(this->Author.child("Date").text().get());
	}
}

void ConfigParser::ParseGlobal(){
	this->Global = this->Configuration.child("Global");
	if( this->Global ){
		GlobalEventWidthInS = this->Global.attribute("EventWidth").as_double(-1.0);
		if( GlobalEventWidthInS < 0.0 ){
			std::stringstream ss;
			ss << "ConfigParser::ParseGlobal() : config file named \""
		   	   << *(this->XMLName) 
		   	   << "\" is malformed and Global node is missing EventWidth attribute. expect positive number";
			throw std::runtime_error(ss.str());
		}

		std::string GlobalEventWidthUnit = this->Global.attribute("EventWidthUnit").as_string("x");
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
				ss << "ConfigParser::ParseGlobal() : config file named \""
		   		   << *(this->XMLName) 
		   		   << "\" is malformed and Global node is missing EventWidthUnit attribute. (s,ns,us,ms) are valid";
				throw std::runtime_error(ss.str());
		}
	}else{
		std::stringstream ss;
		ss << "ConfigParser::ParseGlobal() : config file named \""
		   << *(this->XMLName) 
		   << "\" is malformed and Global node is missing.";
		throw std::runtime_error(ss.str());
	}
}

void ConfigParser::ParseDetectorDriver(){
	this->DetectorDriver = this->Configuration.child("DetectorDriver");
	if( this->DetectorDriver ){
		//loop through list of analyzers and processors
		pugi::xml_node processor = this->DetectorDriver.child("Processor");
		pugi::xml_node analyzer = this->DetectorDriver.child("Analyzer");
		if( (!processor) and (!analyzer) ){
			std::stringstream ss;
			ss << "ConfigParser::ParseDetectorDriver() : config file named \""
			   << *(this->XMLName) 
			   << "\" is malformed. No Processors or Analyzers listed.";
			throw std::runtime_error(ss.str());
		}

		for(; processor; processor = processor.next_sibling("Processor")){
			std::string name = processor.attribute("name").as_string("");
			if( name.compare("") == 0 ){
				std::stringstream ss;
				ss << "ConfigParser::ParseDetectorDriver() : config file named \""
				   << *(this->XMLName) 
				   << "\" is malformed and one of the Processor tags in missing the \"name\" attribute.";
				throw std::runtime_error(ss.str());
			}else{
				this->ProcessorNames[name] = processor;
			}
		}
		for(; analyzer; analyzer = analyzer.next_sibling("Analyzer")){
			std::string name = analyzer.attribute("name").as_string("");
			if( name.compare("") == 0 ){
				std::stringstream ss;
				ss << "ConfigParser::ParseDetectorDriver() : config file named \""
				   << *(this->XMLName) 
				   << "\" is malformed and one of the Analyzer tags in missing the \"name\" attribute.";
				throw std::runtime_error(ss.str());
			}else{
				this->AnalyzerNames[name] = analyzer;
			}
		}
	}else{
		//just make raw coincidences but warn/critical????
		std::stringstream ss;
		ss << "ConfigParser::ParseDetectorDriver() : config file named \""
		   << *(this->XMLName) 
		   << "\" is malformed because DetectorDriver tag is missing.";
		throw std::runtime_error(ss.str());
	}
}

std::map<std::string,pugi::xml_node>& ConfigParser::GetProcessors(){
	return ProcessorNames;
}

std::map<std::string,pugi::xml_node>& ConfigParser::GetAnalyzers(){
	return AnalyzerNames;
}

void ConfigParser::ParseMap(){
	this->Map = this->Configuration.child("Map");
	if( this->Map ){
		//create the channel map object that will be used for lookups. Make it global instanced
		auto cmap = ChannelMap::Get();
		//lambda function to parse out string text into individual entries
		auto parse_cal_string = [](std::string calstring,std::vector<double>& vals){
			double curr;
			std::stringstream ss;
			ss << calstring;
			do{
				ss >> curr;
				vals.push_back(curr);
			}while(ss);
			vals.pop_back();
		};

		pugi::xml_node board = this->Map.child("Module");
		if( !board ){
			std::stringstream ss;
			ss << "ConfigParser::ParseMap() : config file named \""
			   << *(this->XMLName) 
			   << "\" is most likely malformed, because there are no Module tags";
			throw std::runtime_error(ss.str());
		}
		for(; board; board = board.next_sibling("Module") ){
			unsigned long long bid = board.attribute("number").as_ullong(std::numeric_limits<unsigned long long>::max());
			if( bid == std::numeric_limits<unsigned long long>::max() ){
				std::stringstream ss;
				ss << "ConfigParser::ParseMap() : config file named \""
				   << *(this->XMLName) 
				   << "\" is most likely malformed, because one of the Module tags is missing the \"number\" attribute.";
				throw std::runtime_error(ss.str());
			}
			std::string firmware = board.attribute("Firmware").as_string("");
			if( firmware.compare("") == 0 ){
				std::stringstream ss;
				ss << "ConfigParser::ParseMap() : config file named \""
				   << *(this->XMLName) 
				   << "\" is most likely malformed, because module with number \""
				   << bid << "\" is missing firmware attribute.";
				throw std::runtime_error(ss.str());
			}
			int frequency = board.attribute("Frequency").as_int(-1);
			if( frequency < 0  ){
				std::stringstream ss;
				ss << "ConfigParser::ParseMap() : config file named \""
				   << *(this->XMLName) 
				   << "\" is most likely malformed, because module with number \""
				   << bid << "\" is either missing Frequency attribute or is negative";
				throw std::runtime_error(ss.str());
			}
			int TraceDelay = board.attribute("TraceDelay").as_int(-1);
			if( TraceDelay < 0 ){
				std::stringstream ss;
				ss << "ConfigParser::ParseMap() : config file named \""
				   << *(this->XMLName) 
				   << "\" is most likely malformed, because module with number \""
				   << bid << "\" is either missing TraceDelay attribute or is negative";
				throw std::runtime_error(ss.str());
			}
			if( !cmap->AddBoardInfo(bid,firmware,frequency,TraceDelay) ){
				std::stringstream ss;
				ss << "ConfigParser::ParseMap() : config file named \""
				   << *(this->XMLName) 
				   << "\" is most malformed, because module with number \""
				   << bid << "\" is included multiple times";
				throw std::runtime_error(ss.str());
			}

			pugi::xml_node channel = board.child("Channel");
			if( !channel ){
				std::stringstream ss;
				ss << "ConfigParser::ParseMap() : config file named \""
				   << *(this->XMLName) 
				   << "\" is most likely malformed, because There are no Channel tags in Board : "
				   << bid;
				throw std::runtime_error(ss.str());
			}
			for(; channel; channel = channel.next_sibling("Channel") ){
				unsigned long long cid = channel.attribute("number").as_ullong(std::numeric_limits<unsigned long long>::max());
				if( cid == std::numeric_limits<unsigned long long>::max() ){
					std::stringstream ss;
					ss << "ConfigParser::ParseMap() : config file named \""
					   << *(this->XMLName) 
					   << "\" is most likely malformed, because one of the Channel tags in Board : "
					   << bid << " is missing the \"number\" attribute.";
					throw std::runtime_error(ss.str());
				}else{
					std::string type = channel.attribute("type").as_string("");
					if( type.compare("") == 0 ){
						std::stringstream ss;
						ss << "ConfigParser::ParseMap() : config file named \""
						   << *(this->XMLName) 
						   << "\" is most likely malformed, because no type is listed for channel with number=\""
						   << cid << "\" in module with number=\""
						   << bid << "\"";
						throw std::runtime_error(ss.str());
					}
					std::string subtype = channel.attribute("subtype").as_string("");
					if( subtype.compare("") == 0 ){
						std::stringstream ss;
						ss << "ConfigParser::ParseMap() : config file named \""
						   << *(this->XMLName) 
						   << "\" is most likely malformed, because no subtype is listed for channel with number=\""
						   << cid << "\" in module with number=\""
						   << bid << "\"";
						throw std::runtime_error(ss.str());
					}
					std::string group = channel.attribute("group").as_string("");
					if( group.compare("") == 0 ){
						std::stringstream ss;
						ss << "ConfigParser::ParseMap() : config file named \""
						   << *(this->XMLName) 
						   << "\" is most likely malformed, because no group is listed for channel with number=\""
						   << cid << "\" in module with number=\""
						   << bid << "\"";
						throw std::runtime_error(ss.str());
					}
					std::vector<std::string> taglist = {};
					std::string tags = channel.attribute("tags").as_string("");
					//regex to parse out tags
					std::regex word_regex("(\\w+)");
					auto words_begin =std::sregex_iterator(tags.begin(),tags.end(), word_regex);
					auto words_end = std::sregex_iterator();
					for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
						std::smatch match = *i;
						taglist.push_back(match.str());
					}
					if( !cmap->AddChannel(bid,cid,type,subtype,group,taglist) ){
						std::stringstream ss;
						ss << "ConfigParser::ParseMap() : config file named \""
						   << *(this->XMLName) 
						   << "\" is most likely malformed, because duplicate channels with number=\""
						   << cid << "\" are detected in module with number=\""
						   << bid << "\"";
						throw std::runtime_error(ss.str());
					}
				
					pugi::xml_node calibration = channel.child("Calibration");
					if( !calibration ){
						std::stringstream ss;
						ss << "ConfigParser::ParseMap() : config file named \""
						   << *(this->XMLName) 
						   << "\" is malformed. Because no Calibration tag exists for channel with number=\""
						   << cid << "\" in module with number=\""
						   << bid << "\"";
						throw std::runtime_error(ss.str());
					}else{
						std::string cal_type = calibration.attribute("model").as_string("");
						if( cal_type.compare("") == 0 ){
							std::stringstream ss;
							ss << "ConfigParser::ParseMap() : config file named \""
							   << *(this->XMLName) 
							   << "\" is malformed. Because Calibration tag for channel with number=\""
							   << cid << "\" in module with number=\""
							   << bid << "\" is missing the \"model\" attribute";
							throw std::runtime_error(ss.str());
						}
						
						std::vector<double> params;
						std::string calstring = calibration.text().get();
						parse_cal_string(calstring,params);
						if( params.size() == 0 ){
							std::stringstream ss;
							ss << "ConfigParser::ParseMap() : config file named \""
							   << *(this->XMLName) 
							   << "\" is malformed. Because Calibration tag for channel with number=\""
							   << cid << "\" in module with number=\""
							   << bid << "\" is missing the \"text\" containing the calibration parameters";
							throw std::runtime_error(ss.str());
						}

						if (cal_type.compare("linear") == 0 ){
							if( params.size() != 2 ){
								std::stringstream ss;
								ss << "ConfigParser::ParseMap() : config file named \""
								   << *(this->XMLName) 
								   << "\" is malformed. Because Calibration tag for channel with number=\""
								   << cid << "\" in module with number=\""
								   << bid << "\" is model=\"linear\" which expects 2 parameters but has "
								   << params.size() << " listed.";
								throw std::runtime_error(ss.str());
							}
							cmap->GetSingleChannel(bid,cid)->calibrator = new LinearCalibration(params);
						}else if (cal_type.compare("quadratic") == 0 ){
							if( params.size() != 3 ){
								std::stringstream ss;
								ss << "ConfigParser::ParseMap() : config file named \""
								   << *(this->XMLName) 
								   << "\" is malformed. Because Calibration tag for channel with number=\""
								   << cid << "\" in module with number=\""
								   << bid << "\" is model=\"quadratic\" which expects 3 parameters but has "
								   << params.size() << " listed.";
								throw std::runtime_error(ss.str());
							}
							cmap->GetSingleChannel(bid,cid)->calibrator = new QuadraticCalibration(params);
						}else if (cal_type.compare("cubic") == 0 ){
							if( params.size() != 4 ){
								std::stringstream ss;
								ss << "ConfigParser::ParseMap() : config file named \""
								   << *(this->XMLName) 
								   << "\" is malformed. Because Calibration tag for channel with number=\""
								   << cid << "\" in module with number=\""
								   << bid << "\" is model=\"cubic\" which expects 4 parameters but has "
								   << params.size() << " listed.";
								throw std::runtime_error(ss.str());
							}
							cmap->GetSingleChannel(bid,cid)->calibrator = new CubicCalibration(params);
						}else if (cal_type.compare("poly") == 0 ){
							if( params.size() <= 4 ){
								std::stringstream ss;
								ss << "ConfigParser::ParseMap() : config file named \""
								   << *(this->XMLName) 
								   << "\" is malformed. Because Calibration tag for channel with number=\""
								   << cid << "\" in module with number=\""
								   << bid << "\" is model=\"poly\" which expects more than 4 parameters but has "
								   << params.size() << " listed.";
								throw std::runtime_error(ss.str());
							}
							cmap->GetSingleChannel(bid,cid)->calibrator = new PolyCalibration(params);
						}else if (cal_type.compare("linear_expo") == 0 ){
							if( params.size() != 2 ){
								std::stringstream ss;
								ss << "ConfigParser::ParseMap() : config file named \""
								   << *(this->XMLName) 
								   << "\" is malformed. Because Calibration tag for channel with number=\""
								   << cid << "\" in module with number=\""
								   << bid << "\" is model=\"linear_expo\" which expects 2 parameters but has "
								   << params.size() << " listed.";
								throw std::runtime_error(ss.str());
							}
							cmap->GetSingleChannel(bid,cid)->calibrator = new LinearExpoCalibration(params);
						}else if (cal_type.compare("quadratic_expo") == 0 ){
							if( params.size() != 3 ){
								std::stringstream ss;
								ss << "ConfigParser::ParseMap() : config file named \""
								   << *(this->XMLName) 
								   << "\" is malformed. Because Calibration tag for channel with number=\""
								   << cid << "\" in module with number=\""
								   << bid << "\" is model=\"quadratic_expo\" which expects 3 parameters but has "
								   << params.size() << " listed.";
								throw std::runtime_error(ss.str());
							}
							cmap->GetSingleChannel(bid,cid)->calibrator = new QuadraticExpoCalibration(params);
						}else if (cal_type.compare("cubic_expo") == 0 ){
							if( params.size() != 4 ){
								std::stringstream ss;
								ss << "ConfigParser::ParseMap() : config file named \""
								   << *(this->XMLName) 
								   << "\" is malformed. Because Calibration tag for channel with number=\""
								   << cid << "\" in module with number=\""
								   << bid << "\" is model=\"cubic_expo\" which expects 4 parameters but has "
								   << params.size() << " listed.";
								throw std::runtime_error(ss.str());
							}
							cmap->GetSingleChannel(bid,cid)->calibrator = new CubicExpoCalibration(params);
						}else if (cal_type.compare("poly_expo") == 0 ){
							if( params.size() <= 4 ){
								std::stringstream ss;
								ss << "ConfigParser::ParseMap() : config file named \""
								   << *(this->XMLName) 
								   << "\" is malformed. Because Calibration tag for channel with number=\""
								   << cid << "\" in module with number=\""
								   << bid << "\" is model=\"poly_expo\" which expects more than 4 parameters but has "
								   << params.size() << " listed.";
								throw std::runtime_error(ss.str());
							}
							cmap->GetSingleChannel(bid,cid)->calibrator = new PolyExpoCalibration(params);
						}else{
							std::stringstream ss;
							ss << "ConfigParser::ParseMap() : config file named \""
							   << *(this->XMLName) 
							   << "\" is malformed. Because Calibration tag for channel with number=\""
							   << cid << "\" in module with number=\""
							   << bid << "\" has calibration type of model=\""
							   << cal_type << "\"";
							throw std::runtime_error(ss.str());
						}
					}
				}
			}
		}
		spdlog::get("genscan")->info("Generating Map Lookup Tables");
		auto max_flat = cmap->GenerateLookupTables();
		spdlog::get("genscan")->info("Found the following Modules [module_id Firmware Frequency TraceDelay]");
		spdlog::get("genscan")->info("Found the following Channels [module_id channel_id type subtype group tags..]");
		for( auto& b : cmap->GetBoards() ){
			spdlog::get("genscan")->info("Found Board with Info [{}]",*b);
			for( auto& c : b->GetChannels() ){
				if( c != nullptr ){
					spdlog::get("genscan")->info("Found Channel with Info : [{}]",*c);
				}
			}
		}
		spdlog::get("genscan")->info("Flattened channel map resulted in {} channels",max_flat);
	}else{
		//throw error
		std::stringstream ss;
		ss << "ConfigParser::ParseMap() : config file named \""
		   << *(this->XMLName) 
		   << "\" is malformed. Map node is missing.";
		throw std::runtime_error(ss.str());
	}
}
