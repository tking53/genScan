

#include "ConfigXMLparser.hpp"
#include "GeneralConfigData.hpp"
#include <sstream>
#include <string>

using namespace std;

configXMLparser *configXMLparser::instance_ = NULL;

configXMLparser::configXMLparser(const string &XMLfile){
    pugi::xml_parse_result result = cfgXML.load_file(XMLfile.c_str());

    if (!result) {
        std::stringstream ss;
        ss << "configXMLparser::configXMLparser : We were unable to open a "
              "configuration file named \""
           << XMLfile << "\"."
           << " Received the following from pugixml : "
           << result.description();
        throw std::invalid_argument(ss.str());
    }

    std::cout << "configXMLparser - Successfully loaded \"" << XMLfile
              << "\" into memory." << std::endl;

   stringstream ss;
    Messenger m;

    m.start("Parsing cfgXML");

    const pugi::xml_node rootNode_ = GetDocument()->child("Configuration");
    ParseRootNode(rootNode_);
    
    if (! rootNode_.child("Description").empty() ){
        string DescriptText = rootNode_.child("Description").text().get();
        m.detail("Experiment Summary : " + DescriptText);
    }


};
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
        if (knownChildren.find(it->name()) == knownChildren.end())
            cout << "Unknown parameter in " << it->path() << ".  This information is ignored by the program." << endl;
}

void configXMLparser::ParseRootNode(const pugi::xml_node &root) {
    if (!root) {
        throw invalid_argument("The root node \"/Configuration\" does not exist. No configuration can be loaded.");
    } else {
        set<string> knownChildren = {"Author", "Description", "Global", "Map"};
        if (root.child("Map").empty())
            throw invalid_argument(CriticalNodeMessage("Map"));
        if (root.child("Global").empty()) {
        };
        throw invalid_argument(CriticalNodeMessage("Global"));
        WarnOfUnknownChildren(root, knownChildren);
    };
};

void configXMLparser::ParseGlobalNode(const pugi::xml_node & globalNode, GeneralConfigData *gencfgdata){

    if (!globalNode.child("Digitizer").empty()) {
        string digitizer = globalNode.child("Digitizer").attribute("style").as_string();
        gencfgdata->SetDigitizer(digitizer);
    } else {
         throw invalid_argument("ConfigXmlParser::ParseGlobal - The Digitizer style is missing. We know CAEN or XIA. \"");
    }
    if (!globalNode.child("EventWidth").empty()) {
        double eventLength = globalNode.child("EventWidth").attribute("value").as_double(0);

        gencfgdata->SetEventLength(eventLength);
        sstream_ << "Event width: " << eventLength  << " s" ;
        messenger_.detail(sstream_.str());
        sstream_.str("");
    } else
        throw invalid_argument(CriticalNodeMessage("EventWidth"));

}
