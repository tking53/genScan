#ifndef CONFIG_XML_PARSER_HPP
#define CONFIG_XML_PARSER_HPP

#include <iostream>
#include <pugixml.hpp>
#include "ChannelConfiguration.hpp"
#include "Messenger.hpp"
#include "GeneralConfigData.hpp"  

#ifdef USE_SPDLOG
	#include <spdlog/spdlog.h>
	#include <spdlog/cfg/env.h>
	#include <spdlog/fmt/ostr.h>
	#include <spdlog/sinks/basic_file_sink.h>
	#include <spdlog/sinks/stdout_color_sinks.h>
#endif


using namespace std;

class configXMLparser{

	public:

		// return the instance of the parser
		static configXMLparser * get(); 

		// return the instance of the parser
		static configXMLparser * get(const string &file); 

		///@return A constant pointer to the document that was opened up
		const pugi::xml_document *GetDocument() const { return &cfgXML; }

		/// Default destructor that deletes the instance when its called.
		~configXMLparser();

		configXMLparser(const string &XMLfile);

	private:
		// Parse the root node
		void ParseRootNode(const pugi::xml_node &root);

		// Parse the global node
		void ParseGlobalNode(const pugi::xml_node &globalNode, GeneralConfigData *gencfgdata);

		/// Constructs a message string for the throws
		///@param[in] name : The name of the node where we had the error
		///@return The message that we want the throw to contain.
		std::string CriticalNodeMessage(const std::string &name);

		/// Constructs a custom critical error string for the throws
		///@param[in] message : The error we want to throw
		///@return The message that we want the throw to contain.
		std::string CriticalAttributeMessage(const std::string &message);

		/// Warn that we have an unknown parameter in the node.
		///@param [in] node : an iterator pointing to the location of the unknown
		///@param [in] knownChildren: A list of the nodes that are known.
		void WarnOfUnknownChildren(const pugi::xml_node &node, const std::set<std::string> &knownChildren);

		static configXMLparser *instance_;  // instance
		pugi::xml_document cfgXML;
		//An instance of the messenger class so that we can output pretty info
		Messenger messenger_;
		// Stuff used by spdlog
		#ifdef USE_SPDLOG 
			std::shared_ptr<spdlog::logger> LogFile;
		#endif
		// A stringstream that we can use repeatedly without having to redefine.
		std::stringstream sstream_;
};
#endif
