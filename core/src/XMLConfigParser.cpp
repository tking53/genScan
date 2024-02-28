#include <stdexcept>
#include <sstream>
#include <limits>
#include <regex>

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <string>

#include "XMLConfigParser.hpp"
#include "ChannelMap.hpp"
#include "StringManipFunctions.hpp"

XMLConfigParser::XMLConfigParser(const std::string& log) : ConfigParser(log){
}

void XMLConfigParser::Parse(ChannelMap* cmap){
	if( this->ConfigName == nullptr ){
		throw std::runtime_error("XMLConfigParser::Parse() SetConfigFile() has not been called");
	}

	pugi::xml_parse_result result = this->XMLDoc.load_file(this->ConfigName->c_str());

	if( !result ){
		std::stringstream ss;
		ss << "XMLConfigParser::Parse() : Unable to open xml file named \""
		   << *(this->ConfigName) 
		   << "\". pugixml reports : "
		   << result.description();
		throw std::runtime_error(ss.str());
	}
	this->Configuration = this->XMLDoc.child("Configuration");
	if( !this->Configuration ){
		std::stringstream ss;
		ss << "XMLConfigParser::Parse() : config file named \""
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
	spdlog::get(this->LogName)->info("Parsing Cuts tag");
	ParseCuts();
	spdlog::get(this->LogName)->info("Parsing Map tag");
	ParseMap(cmap);
}
		
void XMLConfigParser::ParseDescription(){
	this->Description = this->Configuration.child("Description");
	if( this->Description ){
		this->DescriptionText.reset(new std::string(this->Description.text().get()));
	}
}

void XMLConfigParser::ParseCuts(){
	this->Cuts = this->Configuration.child("Cuts");
	if( this->Cuts ){
		pugi::xml_node cut = this->Cuts.child("Cut");
		for(; cut; cut = cut.next_sibling("Cut")){
			std::string id = cut.attribute("name").as_string("");
			std::string file = cut.attribute("filename").as_string("");
			this->CutFiles.push_back(std::make_pair(id,file));
		}
	}
}

void XMLConfigParser::ParseAuthor(){
	this->Author = this->Configuration.child("Author");
	if( this->Author ){
		if( this->Author.child("Name") )
			this->AuthorNameText.reset(new std::string(this->Author.child("Name").text().get()));
		if( this->Author.child("Email") )
			this->AuthorEmailText.reset(new std::string(this->Author.child("Email").text().get()));
		if( this->Author.child("Date") )
			this->AuthorDateText.reset(new std::string(this->Author.child("Date").text().get()));
	}
}

void XMLConfigParser::ParseGlobal(){
	this->Global = this->Configuration.child("Global");
	if( this->Global ){
		GlobalEventWidthInS = this->Global.attribute("EventWidth").as_double(-1.0);
		if( GlobalEventWidthInS < 0.0 ){
			std::stringstream ss;
			ss << "XMLConfigParser::ParseGlobal() : config file named \""
		   	   << *(this->ConfigName) 
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
				ss << "XMLConfigParser::ParseGlobal() : config file named \""
		   		   << *(this->ConfigName) 
		   		   << "\" is malformed and Global node is missing EventWidthUnit attribute. (s,ns,us,ms) are valid";
				throw std::runtime_error(ss.str());
		}

		this->CoincidenceType.reset(new std::string(this->Global.attribute("CorrelationType").as_string("rolling-window")));
	}else{
		std::stringstream ss;
		ss << "XMLConfigParser::ParseGlobal() : config file named \""
		   << *(this->ConfigName) 
		   << "\" is malformed and Global node is missing.";
		throw std::runtime_error(ss.str());
	}
}

void XMLConfigParser::ParseDetectorDriver(){
	this->DetectorDriver = this->Configuration.child("DetectorDriver");
	if( this->DetectorDriver ){
		//loop through list of analyzers and processors
		pugi::xml_node processor = this->DetectorDriver.child("Processor");
		pugi::xml_node analyzer = this->DetectorDriver.child("Analyzer");
		if( (!processor) and (!analyzer) ){
			std::stringstream ss;
			ss << "XMLConfigParser::ParseDetectorDriver() : config file named \""
			   << *(this->ConfigName) 
			   << "\" is malformed. No Processors or Analyzers listed.";
			throw std::runtime_error(ss.str());
		}

		for(; processor; processor = processor.next_sibling("Processor")){
			std::string name = processor.attribute("name").as_string("");
			if( name.compare("") == 0 ){
				std::stringstream ss;
				ss << "XMLConfigParser::ParseDetectorDriver() : config file named \""
				   << *(this->ConfigName) 
				   << "\" is malformed and one of the Processor tags in missing the \"name\" attribute.";
				throw std::runtime_error(ss.str());
			}else{
				this->ProcessorNames[name] = processor;
				AddProcessorName(name);
			}
		}
		for(; analyzer; analyzer = analyzer.next_sibling("Analyzer")){
			std::string name = analyzer.attribute("name").as_string("");
			if( name.compare("") == 0 ){
				std::stringstream ss;
				ss << "XMLConfigParser::ParseDetectorDriver() : config file named \""
				   << *(this->ConfigName) 
				   << "\" is malformed and one of the Analyzer tags in missing the \"name\" attribute.";
				throw std::runtime_error(ss.str());
			}else{
				this->AnalyzerNames[name] = analyzer;
				AddAnalyzerName(name);
			}
		}
	}else{
		//just make raw coincidences but warn/critical????
		std::stringstream ss;
		ss << "XMLConfigParser::ParseDetectorDriver() : config file named \""
		   << *(this->ConfigName) 
		   << "\" is malformed because DetectorDriver tag is missing.";
		throw std::runtime_error(ss.str());
	}
}

pugi::xml_node XMLConfigParser::GetProcessorXMLInfo(const std::string& name) const{
	return ProcessorNames.at(name);
}

pugi::xml_node XMLConfigParser::GetAnalyzerXMLInfo(const std::string& name) const{
	return AnalyzerNames.at(name);
}

void XMLConfigParser::ParseMap(ChannelMap* cmap){
	this->Map = this->Configuration.child("Map");
	if( this->Map ){
		pugi::xml_node crate = this->Map.child("Crate");
		if( !crate ){
			std::stringstream ss;
			ss << "XMLConfigParser::ParseMap() : config file named \""
			   << *(this->ConfigName) 
			   << "\" is most likely malformed, because there are no Crate tags";
			throw std::runtime_error(ss.str());

		}
		for(; crate; crate = crate.next_sibling("Crate") ){
			int crid = crate.attribute("number").as_ullong(std::numeric_limits<int>::max());
			auto crid_result = this->KnownCrates.insert_unique(crid);
			if( not crid_result.second ){
				throw std::runtime_error("Duplicate crate number found for Crate : "+std::to_string(crid));
			}
			if( crid == std::numeric_limits<int>::max() ){
				std::stringstream ss;
				ss << "XMLConfigParser::ParseMap() : config file named \""
					<< *(this->ConfigName) 
					<< "\" is most likely malformed, because one of the Crate tags is missing the \"number\" attribute.";
				throw std::runtime_error(ss.str());
			}

			pugi::xml_node board = crate.child("Module");
			if( !board ){
				std::stringstream ss;
				ss << "XMLConfigParser::ParseMap() : config file named \""
					<< *(this->ConfigName) 
					<< "\" is most likely malformed, because there are no Module tags";
				throw std::runtime_error(ss.str());
			}
			for(; board; board = board.next_sibling("Module") ){
				int bid = board.attribute("number").as_ullong(std::numeric_limits<int>::max());
				auto board_result = this->KnownBoardsInCrate.insert_unique(bid);
				if( not board_result.second ){
					throw std::runtime_error("Duplicate board number : "+std::to_string(bid)+" in Crate : "+std::to_string(crid));
				}
				if( bid == std::numeric_limits<int>::max() ){
					std::stringstream ss;
					ss << "XMLConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because one of the Module tags is missing the \"number\" attribute in crate with number \""
						<< crid << "\"";
					throw std::runtime_error(ss.str());
				}
				std::string revision = board.attribute("Revision").as_string("");
				if( revision.compare("") == 0 ){
					std::stringstream ss;
					ss << "XMLConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because module with number \""
						<< bid << "\" in crate with number \""
						<< crid << "\" is missing Revision attribute.";
					throw std::runtime_error(ss.str());
				}
				std::string firmware = board.attribute("Firmware").as_string("");
				if( firmware.compare("") == 0 ){
					std::stringstream ss;
					ss << "XMLConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because module with number \""
						<< bid << "\" in crate with number \""
						<< crid << "\" is missing Firmware attribute.";
					throw std::runtime_error(ss.str());
				}
				int frequency = board.attribute("Frequency").as_int(-1);
				if( frequency < 0  ){
					std::stringstream ss;
					ss << "XMLConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because module with number \""
						<< bid << "\" is either missing Frequency attribute or is negative";
					throw std::runtime_error(ss.str());
				}
				int TraceDelay = board.attribute("TraceDelay").as_int(-1);
				if( TraceDelay < 0 ){
					std::stringstream ss;
					ss << "XMLConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because module with number \""
						<< bid << "\" is either missing TraceDelay attribute or is negative";
					throw std::runtime_error(ss.str());
				}
				auto duplicate = cmap->SetBoardInfo(crid,bid,revision[0],firmware,frequency,TraceDelay);
				if( duplicate ){
					std::stringstream ss;
					ss << "XMLConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because module with number \""
						<< bid << "\" is duplicated within the same crate with number \""
						<< crid << "\"";
					throw std::runtime_error(ss.str());

				}

				pugi::xml_node channel = board.child("Channel");
				if( !channel ){
					std::stringstream ss;
					ss << "XMLConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because There are no Channel tags in Board : "
						<< bid;
					throw std::runtime_error(ss.str());
				}
				for(; channel; channel = channel.next_sibling("Channel") ){
					int cid = channel.attribute("number").as_ullong(std::numeric_limits<int>::max());
					auto channel_result = this->KnownChannelsInBoard.insert_unique(cid);
					if( not channel_result.second ){
						throw std::runtime_error("Duplicate channel : "+std::to_string(cid)+" in Board : "+std::to_string(bid)+" in Crate : "+std::to_string(crid));
					}
					if( cid == std::numeric_limits<int>::max() ){
						std::stringstream ss;
						ss << "XMLConfigParser::ParseMap() : config file named \""
							<< *(this->ConfigName) 
							<< "\" is most likely malformed, because one of the Channel tags in Board : "
							<< bid << " is missing the \"number\" attribute.";
						throw std::runtime_error(ss.str());
					}else{
						std::string type = channel.attribute("type").as_string("");
						if( type.compare("") == 0 ){
							std::stringstream ss;
							ss << "XMLConfigParser::ParseMap() : config file named \""
								<< *(this->ConfigName) 
								<< "\" is most likely malformed, because no type is listed for channel with number=\""
								<< cid << "\" in module with number=\""
								<< bid << "\"";
							throw std::runtime_error(ss.str());
						}
						std::string subtype = channel.attribute("subtype").as_string("");
						if( subtype.compare("") == 0 ){
							std::stringstream ss;
							ss << "XMLConfigParser::ParseMap() : config file named \""
								<< *(this->ConfigName) 
								<< "\" is most likely malformed, because no subtype is listed for channel with number=\""
								<< cid << "\" in module with number=\""
								<< bid << "\"";
							throw std::runtime_error(ss.str());
						}
						std::string group = channel.attribute("group").as_string("");
						std::set<std::string> taglist = {};
						std::string tags = channel.attribute("tags").as_string("");
						//regex to parse out tags
						std::regex word_regex("(\\w+)");
						auto words_begin =std::sregex_iterator(tags.begin(),tags.end(), word_regex);
						auto words_end = std::sregex_iterator();
						for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
							std::smatch match = *i;
							taglist.insert(match.str());
						}

						int LocalTraceDelay = channel.attribute("TraceDelay").as_int(TraceDelay);
						if( LocalTraceDelay < 0 ){
							std::stringstream ss;
							ss << "XMLConfigParser::ParseMap() : config file named \""
								<< *(this->ConfigName) 
								<< "\" is malformed. Because no TraceDelay for channel with number=\""
								<< cid << "\" in module with number=\""
								<< bid << "\" is negative";
							throw std::runtime_error(ss.str());
						}


						pugi::xml_node calibration = channel.child("Calibration");
						if( !calibration ){
							std::stringstream ss;
							ss << "XMLConfigParser::ParseMap() : config file named \""
								<< *(this->ConfigName) 
								<< "\" is malformed. Because no Calibration tag exists for channel with number=\""
								<< cid << "\" in module with number=\""
								<< bid << "\"";
							throw std::runtime_error(ss.str());
						}else{
							std::string cal_type = calibration.attribute("model").as_string("");
							if( cal_type.compare("") == 0 ){
								std::stringstream ss;
								ss << "XMLConfigParser::ParseMap() : config file named \""
									<< *(this->ConfigName) 
									<< "\" is malformed. Because Calibration tag for channel with number=\""
									<< cid << "\" in module with number=\""
									<< bid << "\" is missing the \"model\" attribute";
								throw std::runtime_error(ss.str());
							}

							double threshmin = calibration.attribute("thresh_min").as_double(0);
							double threshmax = calibration.attribute("thresh_max").as_double(65536);
							std::pair<double,double> ThreshVals = {threshmin,threshmax};

							std::vector<double> params;
							std::string calstring = calibration.text().get();
							ChannelMap::CalType ct = ChannelMap::CalType::Unknown;
							StringManip::ParseCalString(calstring,params);
							if( params.size() == 0 ){
								std::stringstream ss;
								ss << "XMLConfigParser::ParseMap() : config file named \""
									<< *(this->ConfigName) 
									<< "\" is malformed. Because Calibration tag for channel with number=\""
									<< cid << "\" in module with number=\""
									<< bid << "\" is missing the \"text\" containing the calibration parameters";
								throw std::runtime_error(ss.str());
							}

							if (cal_type.compare("linear") == 0 ){
								if( params.size() != 2 ){
									std::stringstream ss;
									ss << "XMLConfigParser::ParseMap() : config file named \""
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
									ss << "XMLConfigParser::ParseMap() : config file named \""
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
									ss << "XMLConfigParser::ParseMap() : config file named \""
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
									ss << "XMLConfigParser::ParseMap() : config file named \""
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
									ss << "XMLConfigParser::ParseMap() : config file named \""
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
									ss << "XMLConfigParser::ParseMap() : config file named \""
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
								ss << "XMLConfigParser::ParseMap() : config file named \""
									<< *(this->ConfigName) 
									<< "\" is malformed. Because Calibration tag for channel with number=\""
									<< cid << "\" in module with number=\""
									<< bid << "\" has calibration type of model=\""
									<< cal_type << "\"";
								throw std::runtime_error(ss.str());
							}
							auto duplicate = cmap->SetParams(crid,bid,cid,type,subtype,group,tags,taglist,ct,params,LocalTraceDelay,ThreshVals); 
							if( duplicate ){
								std::stringstream ss;
								ss << "XMLConfigParser::ParseMap() : config file named \""
									<< *(this->ConfigName) 
									<< "\" is malformed. Because Parameters for channel with number=\""
									<< cid << "\" in module with number=\""
									<< bid << "\" in crate with number=\""
									<< crid << "\" is duplicated";
								throw std::runtime_error(ss.str());

							}
						}
					}
				}
				this->KnownChannelsInBoard.clear();
			}
			this->KnownBoardsInCrate.clear();
		}
		this->KnownCrates.clear();
		spdlog::get(this->LogName)->debug("Here is the ChannelMap Info we were able to parse");
		auto boardconfig = cmap->GetBoardConfig();
		for( const auto& currboard : boardconfig ){
			spdlog::get(this->LogName)->debug("Found Board with this info : [{}]",currboard.second);
		}
		auto chanconfig = cmap->GetChannelConfig();
		for( const auto& currchan : chanconfig ){
			spdlog::get(this->LogName)->debug("Found Channel with this info : [{}]",currchan.second);
		}
	}else{
		//throw error
		std::stringstream ss;
		ss << "XMLConfigParser::ParseMap() : config file named \""
		   << *(this->ConfigName) 
		   << "\" is malformed. Map node is missing.";
		throw std::runtime_error(ss.str());
	}
}
