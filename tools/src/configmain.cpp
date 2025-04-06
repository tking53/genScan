#include <iostream>
#include <vector>
#include <sstream>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <pugixml.hpp>
#include <pugiconfig.hpp>

#include <boost/program_options.hpp>
#include <boost/regex.hpp>

//to fix, it broke when doing something like [12,14], should just split on , then split on - and remove regex
void ParseNumbers(const std::string& currstr,std::set<int>& currset){
	std::istringstream ss(currstr);
	std::string val;
	if( currstr.find(',') != std::string::npos ){
		std::vector<std::string> refinelist;
		while( std::getline(ss,val,',') ){
			refinelist.push_back(val);
		}
		for( const auto& rstr : refinelist ){
			if( rstr.find('-') != std::string::npos ){
				std::istringstream rr(rstr);
				std::vector<int> range;
				while( std::getline(rr,val,'-') ){
					range.push_back(std::stoi(val));
				}
				range.push_back(1);
				for( auto ii = range[0]; ii <= range[1]; ii += range[2] ){
					currset.insert(ii);
				}
			}else{
				currset.insert(std::stoi(rstr));
			}
		}
	}else if( currstr.find('-') != std::string::npos ){
		std::vector<int> range;
		while( std::getline(ss,val,'-') ){
			range.push_back(std::stoi(val));
		}
		range.push_back(1);
		for( auto ii = range[0]; ii <= range[1]; ii += range[2] ){
			currset.insert(ii);
		}
	}else{
		currset.insert(std::stoi(currstr));
	}
	//boost::regex re("(\\d{1,2}\\-\\d{1,2}(?:\\-\\d){0,})|(\\d)");

	//std::string::const_iterator start = currstr.begin();
	//std::string::const_iterator end = currstr.end();
	//boost::smatch what;
	//boost::match_flag_type flags = boost::match_default;
	//while(regex_search(start, end, what, re, flags)){
	//	auto p = std::string(what[1].first, what[1].second);
	//	if( p.size() == 0 ){
	//		p = std::string(what[2].first, what[2].second);
	//		currset.insert(std::stoi(p));
	//	}else{
	//		std::istringstream ss(p);
	//		std::vector<int> range;
	//		std::string val;
	//		while( std::getline(ss,val,'-') ){
	//			range.push_back(std::stoi(val));
	//		}
	//		range.push_back(1);
	//		for( auto ii = range[0]; ii <= range[1]; ii += range[2] ){
	//			currset.insert(ii);
	//		}	
	//	}
	//	// update search position:
	//	start = what[0].second;
	//	// update flags
	//	flags |= boost::match_prev_avail;
	//	flags |= boost::match_not_bob;
	//}
}


struct CMap{
	std::set<int> crateset;
	std::set<int> boardset;
	std::set<int> channelset;
	std::string type;
	std::string subtype;
	std::string group;
	std::string tags;

	CMap(const std::vector<std::string>& vals){
		type = vals.at(0);
		subtype = vals.at(1);
		group = vals.at(2);
		tags = vals.at(3);

		ParseNumbers(vals.at(4).substr(1,vals.at(4).size()-2),crateset);
		ParseNumbers(vals.at(5).substr(1,vals.at(5).size()-2),boardset);
		ParseNumbers(vals.at(6).substr(1,vals.at(6).size()-2),channelset);
		spdlog::debug("{}:{}:{}:{}",type,subtype,group,tags);
		for( const auto& c : crateset ){
			spdlog::debug("crate -> {}",c);
		}
		for( const auto& c : boardset ){
			spdlog::debug("board -> {}",c);
		}
		for( const auto& c : channelset ){
			spdlog::debug("channel -> {}",c);
		}
	}

	bool IsValid(int crate,int board, int channel) const{
		return (crateset.find(crate) != crateset.end()) and (boardset.find(board) != boardset.end()) and (channelset.find(channel) != channelset.end());
	}
};

struct BMap{
	std::set<int> crateset;
	std::set<int> boardset;
	std::string revision;
	std::string frequency;
	std::string firmware;
	std::string tracedelay;

	BMap(const std::vector<std::string>& vals){
		revision = vals.at(0);
		frequency = vals.at(1);
		firmware = vals.at(2);
		tracedelay = vals.at(3);

		ParseNumbers(vals.at(4).substr(1,vals.at(4).size()-2),crateset);
		ParseNumbers(vals.at(5).substr(1,vals.at(5).size()-2),boardset);
	}

	bool IsValid(int crate,int board) const{
		return (crateset.find(crate) != crateset.end()) and (boardset.find(board) != boardset.end());
	}
};


int main(int argc, char *argv[]) {

	std::string outputfile;
	int numcrate;
	int nummodule;
	int numchannel;
	std::string authorinfo;
	std::string description;
	std::string eventbuild;
	std::vector<std::string> processorlist;
	std::vector<std::string> analyzerlist;
	std::vector<std::string> cmapregexlist;
	std::vector<std::string> moduleregexlist;

	boost::program_options::options_description cmdline_options("Generic Options");
	cmdline_options.add_options()
		("help,h", "produce help message notable regex patterns All Even Numbers : ^\\d*[02468]$ All Odd Numbers : ^\\d*[13579]$")
		("authorinfo,i",boost::program_options::value<std::string>(&authorinfo)->default_value("Author:GenConfig;Date:Today;Email:default AT gmail DOT com"),"Author info to generate, follow format we split on \";\"")
		("description,d",boost::program_options::value<std::string>(&description)->default_value("Automatically Generated xml by GenConfig"),"Description to insert into xml")
		("eventbuild,e",boost::program_options::value<std::string>(&eventbuild)->default_value("EventWidth:500;EventWidthUnit:ns;CorrelationType:rolling-trigger"),"Global info to generate, follow format we split on \";\"")
		("outputfile,o",boost::program_options::value<std::string>(&outputfile)->default_value("config.xml"),"name of config file to dump to")
		("numcrate,x",boost::program_options::value<int>(&numcrate)->default_value(1),"number of crates in default xml")
		("nummodule,y",boost::program_options::value<int>(&nummodule)->default_value(13),"number of modules per crate in default xml")
		("numchannel,z",boost::program_options::value<int>(&numchannel)->default_value(16),"number of channels per module in default xml")
		("processor,p",boost::program_options::value<std::vector<std::string>>(&processorlist),"name of processor to include [Allowed Multiple times]")
		("analyzer,a",boost::program_options::value<std::vector<std::string>>(&analyzerlist),"name of analyzer to include [Allowed Multiple times]")
		("cmapregex,r",boost::program_options::value<std::vector<std::string>>(&cmapregexlist),"how to populate the type:subtype:group:tags for each channel format is \"type:subtype:group:tags:[crate]:[module]:[channels]\" [Allowed Multiple times] example regex -> mtas:center:1:front:[1]:[0-4]:[0,2,4,6,8,10] defines MTAS's center front ring")
		("moduleregex,m",boost::program_options::value<std::vector<std::string>>(&moduleregexlist),"how to populate the type:subtype:group:tags for each channel format is \"Revision:Frequency:Firmware:TraceDelay:[crate]:[module]\" [Allowed Multiple times] example regex -> F:250:R42950:264:[0-12]:[0-2] defines the first three modules in all crates use RevF-250 with a 264ns trace delay")
		;


	boost::program_options::positional_options_description p;

	try{
		boost::program_options::variables_map vm;
		store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
        	notify(vm);
		if( vm.count("help") or argc <= 2 ){
			spdlog::info(cmdline_options);
			exit(EXIT_SUCCESS);
		}

	}catch( std::exception& e){
		spdlog::error(e.what());
		exit(EXIT_FAILURE);
	}    
	
	pugi::xml_document doc;
	pugi::xml_node Configuration = doc.append_child("Configuration");

	auto regexaddnodedata = [](pugi::xml_node& curr,const std::string& searchstring,boost::regex& re,const int& idx,const std::string& nextnodename,const std::string& nextnodedefault,const std::string& errmsg){
		pugi::xml_node next = curr.append_child(nextnodename.c_str());

		boost::smatch match;
		if( boost::regex_search(searchstring,match,re) ){
			next.append_child(pugi::node_pcdata).set_value(std::string(match[idx]).c_str());
		}else{
			spdlog::warn(errmsg);
			next.append_child(pugi::node_pcdata).set_value(nextnodedefault.c_str());
		}
	};
	
	pugi::xml_node Author = Configuration.append_child("Author");

	boost::regex Authorregex("(Author:)(.*?)(;|$)");
	regexaddnodedata(Author,authorinfo,Authorregex,2,"Name","GenConfig","Missing Author information, default to GenConfig");

	boost::regex Dateregex("(Date:)(.*?)(;|$)");
	regexaddnodedata(Author,authorinfo,Dateregex,2,"Date","Today","Missing Date information, default to Today");

	boost::regex Emailregex("(Email:)(.*?)(;|$)");
	regexaddnodedata(Author,authorinfo,Emailregex,2,"Email","default AT gmail DOT com","Missing Email information, default to default AT gmail DOT com");

	pugi::xml_node Description = Configuration.append_child("Description");
	Description.append_child(pugi::node_pcdata).set_value(description.c_str());

	auto regexaddnodeattr = []<class T>(pugi::xml_node& curr,const std::string& searchstring,boost::regex& re,const int& idx,const std::string& attrname,const T& defaultval,const std::string& errmsg){

		boost::smatch match;
		if( boost::regex_search(searchstring,match,re) ){
			curr.append_attribute(attrname.c_str()) = std::string(match[idx]).c_str();
			return true;
		}else{
			spdlog::warn(errmsg);
			curr.append_attribute(attrname.c_str()) = defaultval;
			return false;
		}
	};

	pugi::xml_node Global = Configuration.append_child("Global");

	boost::regex Eventwidthregex("(EventWidth:)(.*?)(;|$)");
	auto foundevtwidth = regexaddnodeattr(Global,eventbuild,Eventwidthregex,2,"EventWidth",500,"Missing EventWidth, using 500 ns");

	boost::regex Eventwidthunitregex("(EventWidthUnit:)(.*?)(;|$)");
	if( not foundevtwidth ){
		Global.append_attribute("EventWidthUnit") = "ns";
	}else{
		auto toss = regexaddnodeattr(Global,eventbuild,Eventwidthunitregex,2,"EventWidthUnit","ns","Missing EventWidthUnit using ns"); 
	}

	boost::regex Correlationtyperegex("(CorrelationType:)(.*?)($|;)");
	{
		auto toss = regexaddnodeattr(Global,eventbuild,Correlationtyperegex,2,"CorrelationType","rolling-trigger","Missing CorrelationType using rolling-trigger");
	}

	pugi::xml_node DetectorDriver = Configuration.append_child("DetectorDriver");
	for( const auto& a : analyzerlist ){
		pugi::xml_node processor = DetectorDriver.append_child("Analyzer");
		processor.append_attribute("name") = a.c_str();
	}
	for( const auto& p : processorlist ){
		pugi::xml_node processor = DetectorDriver.append_child("Processor");
		processor.append_attribute("name") = p.c_str();
	}

	//first one is everyone except last so they need to be trimmed of their ":" at the end
	boost::regex re("(.*?:)|(.+?$)");
	auto gencurrlist = [](const std::string& currstr, const boost::regex& re){
		std::string::const_iterator start = currstr.begin();
		std::string::const_iterator end = currstr.end();
		boost::smatch what;
		boost::match_flag_type flags = boost::match_default;
		std::vector<std::string> vals;
		while(regex_search(start, end, what, re, flags)){
			auto p = std::string(what[1].first, what[1].second);
			if( p.size() == 0 ){
				p = std::string(what[2].first, what[2].second);
			}else{
				p.pop_back();
			}
			vals.push_back(p);
			// update search position:
			start = what[0].second;
			// update flags:
			flags |= boost::match_prev_avail;
			flags |= boost::match_not_bob;
		}
		return vals;
	};
	
	std::vector<BMap*> boardinfo;
	for( const auto& currstr : moduleregexlist ){
		auto currbmap = gencurrlist(currstr,re);
		if( currbmap.size() != 6 ){
			spdlog::error("invalid board map regex, expect revision:frequency:firmware:[cratelist/craterange]:[modulelist/modulerange] for the following regex {}",currstr);
			spdlog::error("got {} tokens instead of 6 listed below",currbmap.size());
			for( const auto& p : currbmap ){
				spdlog::error("{}",p);
			}
			exit(EXIT_FAILURE);
		}
		boardinfo.push_back(new BMap(currbmap));
	}	

	std::vector<CMap*> chaninfo;
	for( const auto& currstr : cmapregexlist ){
		auto currcmap = gencurrlist(currstr,re);
		if( currcmap.size() != 7 ){
			spdlog::error("invalid channel map regex, expect type:subtype:group:tags:[cratelist/craterange]:[modulelist/modulerange]:[channellist/channelrange] for the following regex {}",currstr);
			spdlog::error("got {} tokens instead of 7 listed below",currcmap.size());
			for( const auto& p : currcmap ){
				spdlog::error("{}",p);
			}
			exit(EXIT_FAILURE);
		}
		chaninfo.push_back(new CMap(currcmap));
	}

	pugi::xml_node Map = Configuration.append_child("Map");
	int cnt = 0;
	for( int ii = 0; ii < numcrate; ++ii ){
		pugi::xml_node Crate = Map.append_child("Crate");
		Crate.append_attribute("number") = ii;
		for( int jj = 0; jj < nummodule; ++jj ){
			pugi::xml_node Module = Crate.append_child("Module");
			Module.append_attribute("number") = jj;
			std::string rev = "F";
			std::string freq = "250";
			std::string firm = "R42950";
			std::string delay = "120";
			for( const auto& b : boardinfo ){
				if( b->IsValid(ii,jj) ){
					rev = b->revision;
					freq = b->frequency;
					firm = b->firmware;
					delay = b->tracedelay;
					break;
				}
			}
			Module.append_attribute("Revision") = rev.c_str();
			Module.append_attribute("Frequency") = freq.c_str();
			Module.append_attribute("Firmware") = firm.c_str();
			Module.append_attribute("TraceDelay") = delay.c_str();
			for( int kk = 0; kk < numchannel; ++kk ){
				pugi::xml_node Channel = Module.append_child("Channel");
				Channel.append_attribute("number") = kk;
				std::string type = "generic";
				std::string subtype = "generic";
				std::string group = "generic";
				std::string tags = std::to_string(cnt);
				for( const auto& c : chaninfo ){
					if( c->IsValid(ii,jj,kk) ){
						type = c->type;
						subtype = c->subtype;
						group = c->group;
						tags = c->tags;
						break;
					}
				}
				Channel.append_attribute("type") = type.c_str();
				Channel.append_attribute("subtype") = subtype.c_str();
				Channel.append_attribute("group") = group.c_str();
				Channel.append_attribute("tags") = tags.c_str();
				
				pugi::xml_node Calibration = Channel.append_child("Calibration");
				Calibration.append_attribute("model") = "linear";
				Calibration.append_attribute("min") = 0.0;
				Calibration.append_child(pugi::node_pcdata).set_value("0.0 1.0");

				++cnt;
			}
		}
	}

	std::cout << doc.save_file(outputfile.c_str()) << std::endl;
}
