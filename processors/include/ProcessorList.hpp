#ifndef __PROCESSOR_LIST_HPP__
#define __PROCESSOR_LIST_HPP__

#include <vector>
#include <map>
#include <string>
#include <memory>

#include "ConfigParser.hpp"
#include "HistogramManager.hpp"
#include "XMLConfigParser.hpp"
#include "YAMLConfigParser.hpp"
#include "JSONConfigParser.hpp"

#include "Processor.hpp"
#include "Analyzer.hpp"

class ProcessorList{
	public:
		ProcessorList(const std::string&);

		void InitializeProcessors(XMLConfigParser*);
		void InitializeAnalyzers(XMLConfigParser*);
		
		void InitializeProcessors(YAMLConfigParser*);
		void InitializeAnalyzers(YAMLConfigParser*);
		
		void InitializeProcessors(JSONConfigParser*);
		void InitializeAnalyzers(JSONConfigParser*);

		void PreAnalyze();
		void PreProcess();
		
		void Analyze();
		void Process();

		void PostAnalyze();
		void PostProcess();

		void DeclarePlots(PLOTS::PlotRegistry*) const;

		~ProcessorList();
	private:
		std::string LogName;
		std::vector<std::shared_ptr<Processor>> processors;
		std::vector<std::shared_ptr<Analyzer>> analyzers;
};

#endif
