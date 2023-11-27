#ifndef __ANALYZER_HPP__
#define __ANALYZER_HPP__

//will be the generic processor class that all other processors will derive from
//this will act on every piece of data no matter what, will output to special root tree and make generic
//histograms inside root

#include "HistogramManager.hpp"
#include <string>
#include <vector>
#include <memory>
#include <chrono>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <pugixml.hpp>
#include <pugiconfig.hpp>

#include <yaml-cpp/yaml.h>

#include <json/json.h>

class Analyzer{
	public:
		Analyzer(const std::string&,const std::string&);
		[[maybe_unused]] virtual bool PreProcess();
		[[maybe_unused]] virtual bool Process();
		[[maybe_unused]] virtual bool PostProcess();
		[[maybe_unused]] virtual void EndProcess();
		virtual ~Analyzer();

		[[noreturn]] virtual void Init([[maybe_unused]] const pugi::xml_node&);
		[[noreturn]] virtual void Init([[maybe_unused]] const YAML::Node&);
		[[noreturn]] virtual void Init([[maybe_unused]] const Json::Value&);

		[[noreturn]] virtual void DeclarePlots([[maybe_unused]] PLOTS::PlotRegistry*) const;
	protected:
		enum STEP{
			PREPROCESS,
			PROCESS,
			POSTPROCESS,
			UNKNOWN
		};

		STEP currstep;

		std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
		std::chrono::time_point<std::chrono::high_resolution_clock> stop_time;
		double preprocesstime;
		double processtime;
		double postprocesstime;

		unsigned long long preprocesscalls;
		unsigned long long processcalls;
		unsigned long long postprocesscalls;


		std::string AnalyzerName;
		std::string LogName;
		
		std::shared_ptr<spdlog::logger> console;
};

#endif
