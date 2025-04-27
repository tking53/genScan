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

#include "ConfigParser.hpp"
#include "ChannelMap.hpp"
#include "StringManipFunctions.hpp"

ConfigParser::ConfigParser(const std::string& log){
	this->LogName = log;
}

ConfigParser::~ConfigParser(){
}

void ConfigParser::Parse(ChannelMap* cmap){
	if( this->ConfigName == nullptr ){
		throw std::runtime_error("ConfigParser::Parse() SetConfigFile() has not been called");
	}

	pugi::xml_parse_result result = this->XMLDoc.load_file(this->ConfigName->c_str());

	if( !result ){
		std::stringstream ss;
		ss << "ConfigParser::Parse() : Unable to open xml file named \""
		   << *(this->ConfigName) 
		   << "\". pugixml reports : "
		   << result.description();
		throw std::runtime_error(ss.str());
	}
	this->Configuration = this->XMLDoc.child("Configuration");
	if( !this->Configuration ){
		std::stringstream ss;
		ss << "ConfigParser::Parse() : config file named \""
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
		
void ConfigParser::ParseDescription(){
	this->Description = this->Configuration.child("Description");
	if( this->Description ){
		this->DescriptionText.reset(new std::string(this->Description.text().get()));
	}
}

void ConfigParser::ParseCuts(){
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

void ConfigParser::ParseAuthor(){
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

void ConfigParser::ParseGlobal(){
	this->Global = this->Configuration.child("Global");
	if( this->Global ){
		GlobalEventWidthInS = this->Global.attribute("EventWidth").as_double(-1.0);
		if( GlobalEventWidthInS < 0.0 ){
			std::stringstream ss;
			ss << "ConfigParser::ParseGlobal() : config file named \""
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
				ss << "ConfigParser::ParseGlobal() : config file named \""
		   		   << *(this->ConfigName) 
		   		   << "\" is malformed and Global node is missing EventWidthUnit attribute. (s,ns,us,ms) are valid";
				throw std::runtime_error(ss.str());
		}

		this->CoincidenceType.reset(new std::string(this->Global.attribute("CorrelationType").as_string("rolling-window")));
	}else{
		std::stringstream ss;
		ss << "ConfigParser::ParseGlobal() : config file named \""
		   << *(this->ConfigName) 
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
			   << *(this->ConfigName) 
			   << "\" is malformed. No Processors or Analyzers listed.";
			throw std::runtime_error(ss.str());
		}

		std::set<std::string> names;
		while( processor ){
			this->RecursiveNameCheck(processor,names);
			processor = processor.next_sibling("Processor");
		}
		while( analyzer ){
			this->RecursiveNameCheck(analyzer,names);
			analyzer = analyzer.next_sibling("Analyzer");
		}

		for(processor = this->DetectorDriver.child("Processor"); processor; processor = processor.next_sibling("Processor")){
			std::string name = processor.attribute("name").as_string("");
			if( name.compare("") == 0 ){
				std::stringstream ss;
				ss << "ConfigParser::ParseDetectorDriver() : config file named \""
				   << *(this->ConfigName) 
				   << "\" is malformed and one of the Processor tags in missing the \"name\" attribute.";
				throw std::runtime_error(ss.str());
			}else{
				this->ProcessorNodes[name] = processor;
				AddProcessorName(name);
			}
		}
		for(analyzer = this->DetectorDriver.child("Analyzer"); analyzer; analyzer = analyzer.next_sibling("Analyzer")){
			std::string name = analyzer.attribute("name").as_string("");
			if( name.compare("") == 0 ){
				std::stringstream ss;
				ss << "ConfigParser::ParseDetectorDriver() : config file named \""
				   << *(this->ConfigName) 
				   << "\" is malformed and one of the Analyzer tags in missing the \"name\" attribute.";
				throw std::runtime_error(ss.str());
			}else{
				this->AnalyzerNodes[name] = analyzer;
				AddAnalyzerName(name);
			}
		}
	}else{
		//just make raw coincidences but warn/critical????
		std::stringstream ss;
		ss << "ConfigParser::ParseDetectorDriver() : config file named \""
		   << *(this->ConfigName) 
		   << "\" is malformed because DetectorDriver tag is missing.";
		throw std::runtime_error(ss.str());
	}
}

pugi::xml_node ConfigParser::GetProcessorXMLInfo(const std::string& name) const{
	return ProcessorNodes.at(name);
}

pugi::xml_node ConfigParser::GetAnalyzerXMLInfo(const std::string& name) const{
	return AnalyzerNodes.at(name);
}

void ConfigParser::ParseMap(ChannelMap* cmap){
	this->Map = this->Configuration.child("Map");
	if( this->Map ){
		pugi::xml_node crate = this->Map.child("Crate");
		if( !crate ){
			std::stringstream ss;
			ss << "ConfigParser::ParseMap() : config file named \""
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
				ss << "ConfigParser::ParseMap() : config file named \""
					<< *(this->ConfigName) 
					<< "\" is most likely malformed, because one of the Crate tags is missing the \"number\" attribute.";
				throw std::runtime_error(ss.str());
			}

			pugi::xml_node board = crate.child("Module");
			if( !board ){
				std::stringstream ss;
				ss << "ConfigParser::ParseMap() : config file named \""
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
					ss << "ConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because one of the Module tags is missing the \"number\" attribute in crate with number \""
						<< crid << "\"";
					throw std::runtime_error(ss.str());
				}
				std::string revision = board.attribute("Revision").as_string("");
				if( revision.compare("") == 0 ){
					std::stringstream ss;
					ss << "ConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because module with number \""
						<< bid << "\" in crate with number \""
						<< crid << "\" is missing Revision attribute.";
					throw std::runtime_error(ss.str());
				}
				std::string firmware = board.attribute("Firmware").as_string("");
				if( firmware.compare("") == 0 ){
					std::stringstream ss;
					ss << "ConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because module with number \""
						<< bid << "\" in crate with number \""
						<< crid << "\" is missing Firmware attribute.";
					throw std::runtime_error(ss.str());
				}
				int frequency = board.attribute("Frequency").as_int(-1);
				if( frequency < 0  ){
					std::stringstream ss;
					ss << "ConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because module with number \""
						<< bid << "\" is either missing Frequency attribute or is negative";
					throw std::runtime_error(ss.str());
				}
				auto duplicate = cmap->SetBoardInfo(crid,bid,revision[0],firmware,frequency);
				if( duplicate ){
					std::stringstream ss;
					ss << "ConfigParser::ParseMap() : config file named \""
						<< *(this->ConfigName) 
						<< "\" is most likely malformed, because module with number \""
						<< bid << "\" is duplicated within the same crate with number \""
						<< crid << "\"";
					throw std::runtime_error(ss.str());

				}

				pugi::xml_node channel = board.child("Channel");
				if( !channel ){
					std::stringstream ss;
					ss << "ConfigParser::ParseMap() : config file named \""
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
						ss << "ConfigParser::ParseMap() : config file named \""
							<< *(this->ConfigName) 
							<< "\" is most likely malformed, because one of the Channel tags in Board : "
							<< bid << " is missing the \"number\" attribute.";
						throw std::runtime_error(ss.str());
					}else{
						std::string type = channel.attribute("type").as_string("");
						if( type.compare("") == 0 ){
							std::stringstream ss;
							ss << "ConfigParser::ParseMap() : config file named \""
								<< *(this->ConfigName) 
								<< "\" is most likely malformed, because no type is listed for channel with number=\""
								<< cid << "\" in module with number=\""
								<< bid << "\"";
							throw std::runtime_error(ss.str());
						}
						std::string subtype = channel.attribute("subtype").as_string("");
						if( subtype.compare("") == 0 ){
							std::stringstream ss;
							ss << "ConfigParser::ParseMap() : config file named \""
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

						pugi::xml_node calibration = channel.child("Calibration");
						std::vector<double> params;
						if( !calibration ){
							std::stringstream ss;
							ss << "ConfigParser::ParseMap() : config file named \""
								<< *(this->ConfigName) 
								<< "\" is malformed. Because no Calibration tag exists for channel with number=\""
								<< cid << "\" in module with number=\""
								<< bid << "\"";
							throw std::runtime_error(ss.str());
						}else{
							std::string calstring = calibration.text().get();
							StringManip::ParseCalString(calstring,params);
							if( params.size() == 0 ){
								std::stringstream ss;
								ss << "ConfigParser::ParseMap() : config file named \""
									<< *(this->ConfigName) 
									<< "\" is malformed. Because Calibration tag for channel with number=\""
									<< cid << "\" in module with number=\""
									<< bid << "\" is missing the \"text\" containing the calibration parameters";
								throw std::runtime_error(ss.str());
							}
						}

						pugi::xml_node trapfilter = channel.child("TrapFilter");
						if( !trapfilter ){
							auto duplicate = cmap->SetParams(crid,bid,cid,type,subtype,group,tags,taglist,params); 
							if( duplicate ){
								std::stringstream ss;
								ss << "ConfigParser::ParseMap() : config file named \""
									<< *(this->ConfigName) 
									<< "\" is malformed. Because Parameters for channel with number=\""
									<< cid << "\" in module with number=\""
									<< bid << "\" in crate with number=\""
									<< crid << "\" is duplicated";
								throw std::runtime_error(ss.str());
							}
						}else{
							pugi::xml_node trapfiltercalibration = trapfilter.child("Calibration");
							std::vector<double> tfparams;
							if( !trapfiltercalibration ){
								std::stringstream ss;
								ss << "ConfigParser::ParseMap() : config file named \""
									<< *(this->ConfigName) 
									<< "\" is malformed. Because no Calibration tag exists for TrapFilter on channel with number=\""
									<< cid << "\" in module with number=\""
									<< bid << "\"";
								throw std::runtime_error(ss.str());
							}else{
								int len = trapfilter.attribute("len").as_int(1);
								int gap = trapfilter.attribute("gap").as_int(0);
								int bline = trapfilter.attribute("bline").as_int(1);
								float tau = trapfilter.attribute("tau").as_float(1.0);
								std::string calstring = trapfiltercalibration.text().get();
								StringManip::ParseCalString(calstring,tfparams);
								if( params.size() == 0 ){
									std::stringstream ss;
									ss << "ConfigParser::ParseMap() : config file named \""
										<< *(this->ConfigName) 
										<< "\" is malformed. Because TrapFilter Calibration tag for channel with number=\""
										<< cid << "\" in module with number=\""
										<< bid << "\" is missing the \"text\" containing the calibration parameters";
									throw std::runtime_error(ss.str());
								}
								auto duplicate = cmap->SetParams(crid,bid,cid,type,subtype,group,tags,taglist,params,len,gap,bline,tau,tfparams); 
								if( duplicate ){
									std::stringstream ss;
									ss << "ConfigParser::ParseMap() : config file named \""
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
		ss << "ConfigParser::ParseMap() : config file named \""
		   << *(this->ConfigName) 
		   << "\" is malformed. Map node is missing.";
		throw std::runtime_error(ss.str());
	}
}

void ConfigParser::RecursiveNameCheck(pugi::xml_node& currnode,std::set<std::string>& currset) const{
	std::string currnodetype(currnode.name());
	if( currnodetype.compare("Processor") == 0  or currnodetype.compare("Analyzer") == 0 ){
		auto name = currnode.attribute("name").as_string("");
		if( currset.find(name) != currset.end() ){
			std::stringstream ss;
			ss << "ConfigParser::ParseDetectorDriver() : config file named \""
				<< *(this->ConfigName) 
				<< "\" is malformed. "
				<< currnodetype 
				<< " Named:["
				<< name
				<< "] is repeated at some point. Each named object must only exist once within the config file";
			throw std::runtime_error(ss.str());
		}else{
			currset.insert(name);
		}
	}

	for(pugi::xml_node child : currnode.children()) {
		RecursiveNameCheck(child,currset);	
	}
}

void ConfigParser::SetGlobalEventWidthInS(double width){
	GlobalEventWidthInS = width;
}

void ConfigParser::SetConfigFile(std::string& filename){
	this->ConfigName.reset(new std::string(filename));
}

void ConfigParser::SetDescriptionText(std::string* txt){
	this->DescriptionText.reset(txt);
}

void ConfigParser::SetAuthorNameText(std::string* txt){
	this->AuthorNameText.reset(txt);
}

void ConfigParser::SetAuthorDateText(std::string* txt){
	this->AuthorDateText.reset(txt);
}

void ConfigParser::SetAuthorEmailText(std::string* txt){
	this->AuthorEmailText.reset(txt);
}

double ConfigParser::GetGlobalEventWidth() const{
	return this->GlobalEventWidthInS;
}

double ConfigParser::GetGlobalEventWidthInNS() const{
	return this->GlobalEventWidthInS*1.0e9;
}	
std::string* ConfigParser::GetConfigName() const{
	return this->ConfigName.get();
}

std::string* ConfigParser::GetDescriptionText() const{
	return this->DescriptionText.get();
}

std::string* ConfigParser::GetAuthorNameText() const{
	return this->AuthorNameText.get();
}

std::string* ConfigParser::GetAuthorDateText() const{
	return this->AuthorDateText.get();
}

std::string* ConfigParser::GetAuthorEmailText() const{
	return this->AuthorEmailText.get();
}

std::vector<std::string> ConfigParser::GetProcessorNames() const{
	return ProcessorNames;
}

std::vector<std::string> ConfigParser::GetAnalyzerNames() const{
	return AnalyzerNames;
}

void ConfigParser::AddProcessorName(const std::string& name){
	ProcessorNames.push_back(name);
}
void ConfigParser::AddAnalyzerName(const std::string& name){
	AnalyzerNames.push_back(name);
}

std::string* ConfigParser::GetCorrelationType() const{
	return this->CoincidenceType.get();
}

void ConfigParser::SetCorrelationType(std::string* val){
	this->CoincidenceType.reset(val);
}

std::vector<std::pair<std::string,std::string>> ConfigParser::GetCutDetails() const{
	return this->CutFiles;
}

void ConfigParser::GetConfigFileStr(std::stringstream& ss) const{
	this->XMLDoc.save(ss);
}
