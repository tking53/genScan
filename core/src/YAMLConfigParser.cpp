#include <stdexcept>
#include <sstream>
#include <limits>
#include <regex>

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "yaml-cpp/exceptions.h"
#include "yaml-cpp/node/detail/iterator_fwd.h"
#include "yaml-cpp/node/type.h"
#include "yaml-cpp/parser.h"

#include "YAMLConfigParser.hpp"
#include "ChannelMap.hpp"
#include "StringManipFunctions.hpp"

YAMLConfigParser::YAMLConfigParser(const std::string& log) : ConfigParser(log){
}

void YAMLConfigParser::Parse(ChannelMap* cmap){
	if( this->ConfigName == nullptr ){
		throw std::runtime_error("YAMLConfigParser::Parse() SetConfigFile() has not been called");
	}

	try{
		YAMLDoc = YAML::LoadFile(*(this->ConfigName));;
	}catch( YAML::ParserException& e ){
		throw std::runtime_error(e.what());
	}

	this->Configuration = YAMLDoc["Configuration"];
	if( !this->Configuration ){
		std::stringstream ss;
		ss << "YAMLConfigParser::Parse() : config file named \""
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
		
void YAMLConfigParser::ParseDescription(){
	this->Description = this->Configuration["Description"];
	if( this->Description ){
		this->DescriptionText.reset(new std::string(this->Description.as<std::string>()));
	}
}

void YAMLConfigParser::ParseAuthor(){
	this->Author = this->Configuration["Author"];
	if( this->Author ){
		if( this->Author["Name"] )
			this->AuthorNameText.reset(new std::string(this->Author["Name"].as<std::string>()));
		if( this->Author["Email"] )
			this->AuthorEmailText.reset(new std::string(this->Author["Email"].as<std::string>()));
		if( this->Author["Date"] )
			this->AuthorDateText.reset(new std::string(this->Author["Date"].as<std::string>()));
	}
}

void YAMLConfigParser::ParseGlobal(){
	this->Global = this->Configuration["Global"];
	if( this->Global ){
		GlobalEventWidthInS = this->Global["EventWidth"].as<double>(-1.0);
		if( GlobalEventWidthInS < 0.0 ){
			std::stringstream ss;
			ss << "YAMLConfigParser::ParseGlobal() : config file named \""
		   	   << *(this->ConfigName) 
		   	   << "\" is malformed and Global node is missing EventWidth attribute. expect positive number";
			throw std::runtime_error(ss.str());
		}

		std::string GlobalEventWidthUnit = this->Global["EventWidthUnit"].as<std::string>("x");
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
				ss << "YAMLConfigParser::ParseGlobal() : config file named \""
		   		   << *(this->ConfigName) 
		   		   << "\" is malformed and Global node is missing EventWidthUnit attribute. (s,ns,us,ms) are valid";
				throw std::runtime_error(ss.str());
		}
		this->CoincidenceType.reset(new std::string(this->Global["CorrelationType"].as<std::string>("rolling-window")));
	}else{
		std::stringstream ss;
		ss << "YAMLConfigParser::ParseGlobal() : config file named \""
		   << *(this->ConfigName) 
		   << "\" is malformed and Global node is missing.";
		throw std::runtime_error(ss.str());
	}
}

void YAMLConfigParser::ParseDetectorDriver(){
	this->DetectorDriver = this->Configuration["DetectorDriver"];
	if( this->DetectorDriver ){

		YAML::Node processor = this->DetectorDriver["Processor"];
		YAML::Node analyzer = this->DetectorDriver["Analyzer"];
		if( (!processor) and (!analyzer) ){
			std::stringstream ss;
			ss << "YAMLConfigParser::ParseDetectorDriver() : config file named \""
			   << *(this->ConfigName) 
			   << "\" is malformed. No Processors or Analyzers listed.";
			throw std::runtime_error(ss.str());
		}

		for( size_t ii = 0; ii < processor.size(); ++ii ){
			std::string name = processor[ii]["name"].as<std::string>();
			this->ProcessorNames[name] = processor[ii];
			AddProcessorName(name);
		}

		for( size_t ii = 0; ii < analyzer.size(); ++ii ){
			std::string name = analyzer[ii]["name"].as<std::string>();
			this->AnalyzerNames[name] = analyzer[ii];
			AddAnalyzerName(name);
		}


	}else{
		std::stringstream ss;
		ss << "YAMLConfigParser::ParseDetectorDriver() : config file named \""
		   << *(this->ConfigName) 
		   << "\" is malformed because DetectorDriver tag is missing.";
		throw std::runtime_error(ss.str());
	}
}

void YAMLConfigParser::ParseMap(ChannelMap* cmap){
	this->Map = this->Configuration["Map"];
	if( this->Map ){
		YAML::Node crate = this->Map["Crate"];
		if( !crate ){
			std::stringstream ss;
			ss << "YAMLConfigParser::ParseMap() : config file named \""
				<< *(this->ConfigName) 
				<< "\" is most likely malformed, because there are no Crate tags";
			throw std::runtime_error(ss.str());
		}
		for( size_t kk = 0; kk < crate.size(); ++kk ){
			int crid = crate[kk]["number"].as<int>(std::numeric_limits<int>::max());
			if( crid == std::numeric_limits<int>::max() ){
				std::stringstream ss;
				ss << "YAMLConfigParser::ParseMap() : config file named \""
					<< *(this->ConfigName) 
					<< "\" is most likely malformed, because one of the Crate tags is missing the \"number\" attribute.";
				throw std::runtime_error(ss.str());
			}

			YAML::Node board = crate[kk]["Module"];
			if( !board ){
				std::stringstream ss;
				ss << "YAMLConfigParser::ParseMap() : config file named \""
					<< *(this->ConfigName) 
					<< "\" is most likely malformed, because there are no Module tags";
				throw std::runtime_error(ss.str());
			}
			for( size_t ii = 0; ii < board.size(); ++ii ){
				int bid = board[ii]["number"].as<int>(std::numeric_limits<int>::max());
				if( bid == std::numeric_limits<int>::max() ){
					std::stringstream ss;
					ss << "YAMLConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because one of the Module tags is missing the \"number\" attribute.";
					throw std::runtime_error(ss.str());
				}
				std::string revision = board[ii]["Revision"].as<std::string>("");
				if( revision.compare("") == 0 ){
					std::stringstream ss;
					ss << "YAMLConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because module with number \""
						<< bid << "\" in crate with number \""
						<< crid << "\" is missing Revision attribute.";
					throw std::runtime_error(ss.str());
				}

				std::string firmware = board[ii]["Firmware"].as<std::string>("");
				if( firmware.compare("") == 0 ){
					std::stringstream ss;
					ss << "YAMLConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because module with number \""
						<< bid << "\" is missing firmware attribute.";
					throw std::runtime_error(ss.str());
				}
				int frequency = board[ii]["Frequency"].as<int>(-1);
				if( frequency < 0  ){
					std::stringstream ss;
					ss << "YAMLConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because module with number \""
						<< bid << "\" is either missing Frequency attribute or is negative";
					throw std::runtime_error(ss.str());
				}
				int TraceDelay = board[ii]["TraceDelay"].as<int>(-1);
				if( TraceDelay < 0 ){
					std::stringstream ss;
					ss << "YAMLConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because module with number \""
						<< bid << "\" is either missing TraceDelay attribute or is negative";
					throw std::runtime_error(ss.str());
				}
				auto duplicate = cmap->SetBoardInfo(crid,bid,revision[0],firmware,frequency,TraceDelay);
				if( duplicate ){
					std::stringstream ss;
					ss << "YAMLConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because module with number \""
						<< bid << "\" is duplicated within the same crate with number \""
						<< crid << "\"";
					throw std::runtime_error(ss.str());
				}

				YAML::Node channel = board[ii]["Channel"];
				if( !channel ){
					std::stringstream ss;
					ss << "YAMLConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because There are no Channel tags in Board : "
						<< bid;
					throw std::runtime_error(ss.str());
				}
				for(size_t jj = 0; jj < channel.size(); ++jj ){
					int cid = channel[jj]["number"].as<int>(std::numeric_limits<int>::max());
					if( cid == std::numeric_limits<int>::max() ){
						std::stringstream ss;
						ss << "YAMLConfigParser::ParseMap() : config file named \""
							<< *(this->ConfigName) 
							<< "\" is most likely malformed, because one of the Channel tags in Board : "
							<< bid << " is missing the \"number\" attribute.";
						throw std::runtime_error(ss.str());
					}else{
						std::string type = channel[jj]["type"].as<std::string>("");
						if( type.compare("") == 0 ){
							std::stringstream ss;
							ss << "YAMLConfigParser::ParseMap() : config file named \""
								<< *(this->ConfigName) 
								<< "\" is most likely malformed, because no type is listed for channel with number=\""
								<< cid << "\" in module with number=\""
								<< bid << "\"";
							throw std::runtime_error(ss.str());
						}
						std::string supertype = channel[jj]["supertype"].as<std::string>(type);
						std::string subtype = channel[jj]["subtype"].as<std::string>("");
						if( subtype.compare("") == 0 ){
							std::stringstream ss;
							ss << "YAMLConfigParser::ParseMap() : config file named \""
								<< *(this->ConfigName) 
								<< "\" is most likely malformed, because no subtype is listed for channel with number=\""
								<< cid << "\" in module with number=\""
								<< bid << "\"";
							throw std::runtime_error(ss.str());
						}
						std::string group = channel[jj]["group"].as<std::string>("");
						if( group.compare("") == 0 ){
							std::stringstream ss;
							ss << "YAMLConfigParser::ParseMap() : config file named \""
								<< *(this->ConfigName) 
								<< "\" is most likely malformed, because no group is listed for channel with number=\""
								<< cid << "\" in module with number=\""
								<< bid << "\"";
							throw std::runtime_error(ss.str());
						}
						std::set<std::string> taglist = {};
						std::string tags = channel[jj]["tags"].as<std::string>("");
						//regex to parse out tags
						std::regex word_regex("(\\w+)");
						auto words_begin =std::sregex_iterator(tags.begin(),tags.end(), word_regex);
						auto words_end = std::sregex_iterator();
						for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
							std::smatch match = *i;
							taglist.insert(match.str());
						}

						int LocalTraceDelay = channel[jj]["TraceDelay"].as<int>(TraceDelay);
						if( LocalTraceDelay < 0 ){
							std::stringstream ss;
							ss << "YAMLConfigParser::ParseMap() : config file named \""
								<< *(this->ConfigName) 
								<< "\" is malformed. Because no TraceDelay for channel with number=\""
								<< cid << "\" in module with number=\""
								<< bid << "\" is negative";
							throw std::runtime_error(ss.str());
						}

						YAML::Node calibration = channel[jj]["Calibration"];
						if( !calibration ){
							std::stringstream ss;
							ss << "YAMLConfigParser::ParseMap() : config file named \""
								<< *(this->ConfigName) 
								<< "\" is malformed. Because no Calibration tag exists for channel with number=\""
								<< cid << "\" in module with number=\""
								<< bid << "\"";
							throw std::runtime_error(ss.str());
						}else{
							std::string cal_type = calibration["model"].as<std::string>("");
							if( cal_type.compare("") == 0 ){
								std::stringstream ss;
								ss << "YAMLConfigParser::ParseMap() : config file named \""
									<< *(this->ConfigName) 
									<< "\" is malformed. Because Calibration tag for channel with number=\""
									<< cid << "\" in module with number=\""
									<< bid << "\" is missing the \"model\" attribute";
								throw std::runtime_error(ss.str());
							}

							double threshmin = calibration["thresh_min"].as<double>(0);
							double threshmax = calibration["thresh_max"].as<double>(65536);
							std::pair<double,double> ThreshVals = {threshmin,threshmax};

							std::vector<double> params;
							std::string calstring = calibration["value"].as<std::string>("0.0 1.0");
							ChannelMap::CalType ct = ChannelMap::CalType::Unknown;
							StringManip::ParseCalString(calstring,params);
							if( params.size() == 0 ){
								std::stringstream ss;
								ss << "YAMLConfigParser::ParseMap() : config file named \""
									<< *(this->ConfigName) 
									<< "\" is malformed. Because Calibration tag for channel with number=\""
									<< cid << "\" in module with number=\""
									<< bid << "\" is missing the \"text\" containing the calibration parameters";
								throw std::runtime_error(ss.str());
							}

							if (cal_type.compare("linear") == 0 ){
								if( params.size() != 2 ){
									std::stringstream ss;
									ss << "YAMLConfigParser::ParseMap() : config file named \""
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
									ss << "YAMLConfigParser::ParseMap() : config file named \""
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
									ss << "YAMLConfigParser::ParseMap() : config file named \""
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
									ss << "YAMLConfigParser::ParseMap() : config file named \""
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
									ss << "YAMLConfigParser::ParseMap() : config file named \""
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
									ss << "YAMLConfigParser::ParseMap() : config file named \""
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
								ss << "YAMLConfigParser::ParseMap() : config file named \""
									<< *(this->ConfigName) 
									<< "\" is malformed. Because Calibration tag for channel with number=\""
									<< cid << "\" in module with number=\""
									<< bid << "\" has calibration type of model=\""
									<< cal_type << "\"";
								throw std::runtime_error(ss.str());
							}
							auto duplicate = cmap->SetParams(crid,bid,cid,supertype,type,subtype,group,tags,taglist,ct,params,LocalTraceDelay,ThreshVals); 
							if( duplicate ){
								std::stringstream ss;
								ss << "YAMLConfigParser::ParseMap() : config file named \""
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
			}
		}
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
		ss << "YAMLConfigParser::ParseMap() : config file named \""
		   << *(this->ConfigName) 
		   << "\" is malformed. Map node is missing.";
		throw std::runtime_error(ss.str());
	}
}

YAML::Node YAMLConfigParser::GetProcessorYAMLInfo(const std::string& name) const{
	return this->ProcessorNames.at(name);
}

YAML::Node YAMLConfigParser::GetAnalyzerYAMLInfo(const std::string& name) const{
	return this->AnalyzerNames.at(name);
}
