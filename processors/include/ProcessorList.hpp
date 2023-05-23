#ifndef __PROCESSOR_LIST_HPP__
#define __PROCESSOR_LIST_HPP__

#include <vector>
#include <map>
#include <string>

#include <pugiconfig.hpp>
#include <pugixml.hpp>

class Processor;
class Analyzer;

class ProcessorList{
	public:
		static ProcessorList* Get();
		void InitializeProcessors(std::map<std::string,pugi::xml_node>&);
		void InitializeAnalyzers(std::map<std::string,pugi::xml_node>&);
	private:
		ProcessorList();

		static ProcessorList* instance;
		std::vector<Processor*> processors;
		std::vector<Analyzer*> analyzers;
};

#endif
