#ifndef __PROCESSOR_LIST_HPP__
#define __PROCESSOR_LIST_HPP__

#include <random>
#include <vector>
#include <string>
#include <memory>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>


#include "ChannelMap.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "PhysicsData.hpp"
#include "RootFileManager.hpp"
#include "XMLConfigParser.hpp"
#include "YAMLConfigParser.hpp"
#include "JSONConfigParser.hpp"

#include "Processor.hpp"
#include "Analyzer.hpp"
#include "boost/container/deque.hpp"

class ProcessorList{
	public:
		ProcessorList(const std::string&);

		void InitializeProcessors(XMLConfigParser*);
		void InitializeAnalyzers(XMLConfigParser*);
		
		void InitializeProcessors(YAMLConfigParser*);
		void InitializeAnalyzers(YAMLConfigParser*);
		
		void InitializeProcessors(JSONConfigParser*);
		void InitializeAnalyzers(JSONConfigParser*);

		void PreAnalyze(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		void PreProcess(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		
		void Analyze(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		void Process(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*);

		void PostAnalyze(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		void PostProcess(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*);

		void RegisterOutputTrees(RootFileManager*);
		void DeclarePlots(PLOTS::PlotRegistry*) const;

		void ThreshAndCal(boost::container::deque<PhysicsData>&,ChannelMap*);
		void ProcessRaw(boost::container::deque<PhysicsData>&,PLOTS::PlotRegistry*);

		void Finalize();

		void CleanupTrees();

		~ProcessorList();

		const std::vector<std::shared_ptr<Processor>>& GetProcessors() const;
		const std::vector<std::shared_ptr<Analyzer>>& GetAnalyzers() const;
	private:
		void CreateProc(const std::string&);
		void CreateAnal(const std::string&);

		std::string LogName;
		std::shared_ptr<spdlog::logger> console;
		std::vector<std::shared_ptr<Processor>> known_processors;
		std::vector<std::shared_ptr<Analyzer>> known_analyzers;

		std::mt19937_64 randGen;
		std::uniform_real_distribution<double> randNum;
		double FirstTimeStamp;
		unsigned long long EventStamp;
};

#endif
