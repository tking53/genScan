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
#include "EventHistoryManager.hpp"
#include "EventSummary.hpp"
#include "HistogramManager.hpp"
#include "PhysicsData.hpp"
#include "RootFileManager.hpp"
#include "ConfigParser.hpp"

#include "Processor.hpp"
#include "Analyzer.hpp"
#include "boost/container/devector.hpp"

class ProcessorList{
	public:
		ProcessorList(const std::string&, PLOTS::PlotRegistry*);
		~ProcessorList() = default;

		void InitializeProcessors(ConfigParser*,bool);
		void InitializeAnalyzers(ConfigParser*);
		
		void PreAnalyze(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		void PreProcess(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		
		void Analyze(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		void Process(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*);

		void PostAnalyze(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		void PostProcess(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*);

		void RegisterCuts(CUTS::CutRegistry*);
		void RegisterOutputTrees(RootFileManager*);
		void DeclarePlots(PLOTS::PlotRegistry*) const;

		void ThreshAndCal(boost::container::devector<PhysicsData>&,ChannelMap*);
		void ProcessRaw(EventHistoryManager*,PLOTS::PlotRegistry*);

		void Finalize();

		void CleanupTrees();


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
		std::vector<std::string> QDCHisNames;
		std::vector<short> Hits;
};

#endif
