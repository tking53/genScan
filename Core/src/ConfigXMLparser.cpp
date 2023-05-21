

#include "ConfigXMLparser.hpp"
#include "GeneralConfigData.hpp"
#include <sstream>
#include <string>

using namespace std;

configXMLparser *configXMLparser::instance_ = NULL;

configXMLparser::configXMLparser(const string &XMLfile){
	pugi::xml_parse_result result = cfgXML.load_file(XMLfile.c_str());
	#ifdef USE_SPDLOG
		LogFile = spdlog::basic_logger_mt("configXMLparser","configXMLparser.log",true);
		//LogFile->set_pattern("%v");
	#endif

	if (!result) {
		std::stringstream ss;
		ss << "configXMLparser::configXMLparser : We were unable to open a "
			"configuration file named \""
			<< XMLfile << "\"."
			<< " Received the following from pugixml : "
			<< result.description();
		#ifdef USE_SPDLOG 
			LogFile->error(ss.str());
		#endif 
		throw std::invalid_argument(ss.str());
	}

	#ifdef USE_SPDLOG 
		LogFile->info("configXMLparser - Successfully loaded {} into memory.",XMLfile);
	#else
		std::cout << "configXMLparser - Successfully loaded \"" << XMLfile
			<< "\" into memory." << std::endl;
	#endif 

	stringstream ss;
	Messenger m;

	#ifdef USE_SPDLOG 
		LogFile->info("Parsing cfgXML");
	#else
		m.start("Parsing cfgXML");
	#endif 

	const pugi::xml_node rootNode_ = GetDocument()->child("Configuration");
	WarnOfUnknownChildren(rootNode_, RootKnownChildren);
	ParseRootNode(rootNode_);
}
/** Instance is created upon first call */
configXMLparser *configXMLparser::get() {
	if (!instance_)
		instance_ = new configXMLparser("genscan_config.xml");
	return (instance_);
}

/** Instance is created upon first call */
configXMLparser *configXMLparser::get(const std::string &file) {
	if (!instance_)
		instance_ = new configXMLparser(file);
	return (instance_);
}

configXMLparser::~configXMLparser() {
	delete (instance_);
	instance_ = NULL;
}

///We simply return an error message that will be used to give the user
/// information about what went wrong.
string configXMLparser::CriticalNodeMessage(const std::string &name) {
	return "configXMLparse - We couldn't find the \"" + name + "\" node. This node is critical to operation.";
}

///We simply return an error message that will be used to give the user
/// information about what went wrong.
string configXMLparser::CriticalAttributeMessage(const std::string &message) {
	return "configXMLparse - \"" + message;
}

void configXMLparser::WarnOfUnknownChildren(const pugi::xml_node &node, const set<string> &knownChildren) {
	for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
		if (knownChildren.find(it->name()) == knownChildren.end()){
			#ifdef USE_SPDLOG 
				LogFile->critical("Unknown parameter in {}. This information is ignored by the program.",it->path());
			#else
				cout << "Unknown parameter in " << it->path() << ".  This information is ignored by the program." << endl;
			#endif 
		}
}

void configXMLparser::ParseRootNode(const pugi::xml_node &root) {
	if (!root) {
		throw std::invalid_argument("The root node \"/Configuration\" does not exist. No configuration can be loaded.");
	} else {

		if (!root.child("Description").empty() ){
			string DescriptText = root.child("Description").text().get();
			#ifdef USE_SPDLOG 
				LogFile->critical("Experiment Summary : " + DescriptText);
			#else
				m.detail("Experiment Summary : " + DescriptText);
			#endif 
		}

		if (root.child("Map").empty()){
			#ifdef USE_SPDLOG 
				LogFile->error(CriticalNodeMessage("Map"));
			#endif 
			throw std::invalid_argument(CriticalNodeMessage("Map"));
		}else{
			WarnOfUnknownChildren(root.child("Map"),MapKnownChildren);
		}

		if (root.child("Global").empty()) {
			#ifdef USE_SPDLOG 
				LogFile->error(CriticalNodeMessage("Global"));
			#endif 
			throw std::invalid_argument(CriticalNodeMessage("Global"));
		}else{
			WarnOfUnknownChildren(root.child("Global"),GlobalKnownChildren);
		}
	};
}

void configXMLparser::ParseGlobalNode(const pugi::xml_node & globalNode, GeneralConfigData *gencfgdata){

	if (!globalNode.child("Digitizer").empty()) {
		string digitizer = globalNode.child("Digitizer").attribute("style").as_string();
		gencfgdata->SetDigitizer(digitizer);
	} else {
		#ifdef USE_SPDLOG 
			LogFile->error("ConfigXmlParser::ParseGlobal - The Digitizer style is missing. We know CAEN or XIA. \"");
		#endif 
		throw std::invalid_argument("ConfigXmlParser::ParseGlobal - The Digitizer style is missing. We know CAEN or XIA. \"");
	}
	if (!globalNode.child("EventWidth").empty()) {
		double eventLength = globalNode.child("EventWidth").attribute("value").as_double(0);

		gencfgdata->SetEventLength(eventLength);
		#ifdef USE_SPDLOG
			LogFile->info("EventWidth : {} s",eventLength);
		#else
			sstream_ << "Event width: " << eventLength  << " s" ;
			messenger_.detail(sstream_.str());
			sstream_.str("");
		#endif
	} else {
		#ifdef USE_SPDLOG 
			LogFile->error(CriticalNodeMessage("EventWidth"));
		#endif 
		throw std::invalid_argument(CriticalNodeMessage("EventWidth"));
	}
}
