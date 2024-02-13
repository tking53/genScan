#ifndef __PROCESSOR_LIST_HPP__
#define __PROCESSOR_LIST_HPP__

#include <random>
#include <vector>
#include <map>
#include <string>
#include <memory>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>


#include "ChannelMap.hpp"
#include "ConfigParser.hpp"
#include "HistogramManager.hpp"
#include "PhysicsData.hpp"
#include "RootFileManager.hpp"
#include "XMLConfigParser.hpp"
#include "YAMLConfigParser.hpp"
#include "JSONConfigParser.hpp"

#include "Processor.hpp"
#include "Analyzer.hpp"
#include "boost/container/devector.hpp"

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

		void RegisterOutputTrees(RootFileManager*);
		void DeclarePlots(PLOTS::PlotRegistry*) const;

		void ThreshAndCal(boost::container::devector<PhysicsData>&,ChannelMap*);
		void ProcessRaw(boost::container::devector<PhysicsData>&,PLOTS::PlotRegistry*);

		~ProcessorList();
	private:
		std::string LogName;
		std::shared_ptr<spdlog::logger> console;
		std::vector<std::shared_ptr<Processor>> processors;
		std::vector<std::shared_ptr<Analyzer>> analyzers;

		std::mt19937_64 randGen;
		std::uniform_real_distribution<double> randNum;
		double FirstTimeStamp;
};

#endif
