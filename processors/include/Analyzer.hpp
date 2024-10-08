#ifndef __ANALYZER_HPP__
#define __ANALYZER_HPP__

//will be the generic processor class that all other processors will derive from
//this will act on every piece of data no matter what, will output to special root tree and make generic
//histograms inside root

#include <initializer_list>
#include <unordered_map>
#include <string>
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

#include <boost/regex.hpp>

#include "HistogramManager.hpp"
#include "CutManager.hpp"
#include "EventSummary.hpp"

class Analyzer : public std::enable_shared_from_this<Analyzer>{
	public:
		Analyzer(const std::string&,const std::string&,const std::initializer_list<std::string>&);
		virtual ~Analyzer();
		[[maybe_unused]] virtual bool PreProcess() final;
		[[maybe_unused]] virtual bool Process() final;
		[[maybe_unused]] virtual bool PostProcess() final;
		[[noreturn]] virtual bool PreProcess(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		[[noreturn]] virtual bool Process(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		[[noreturn]] virtual bool PostProcess(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		virtual void EndProcess() final;

		std::shared_ptr<Analyzer> GetPtr();
		virtual std::string GetAnalyzerName() const final;

		[[nodiscard]] virtual bool ContainsType(const std::string&) const final;
		[[nodiscard]] virtual bool ContainsAnyType(const std::set<std::string>&) const final;

		[[noreturn]] virtual void Init([[maybe_unused]] const pugi::xml_node&);
		[[noreturn]] virtual void Init([[maybe_unused]] const YAML::Node&);
		[[noreturn]] virtual void Init([[maybe_unused]] const Json::Value&);

		virtual void AssociateType(const std::string&) final;
		[[noreturn]] virtual void Finalize();

		[[noreturn]] virtual void DeclarePlots([[maybe_unused]] PLOTS::PlotRegistry*) const;

	  	[[nodiscard]] virtual const boost::regex& GetDefaultRegex() const final;
		[[nodiscard]] virtual const std::unordered_map<std::string,boost::regex>& GetAllDefaultRegex() const final;
		
		[[nodiscard]] virtual std::set<std::string> GetKnownTypes() const final;

	protected:
		virtual void LoadHistogramSettings(const pugi::xml_node&) final;
		virtual void LoadHistogramSettings(const YAML::Node&) final;
		virtual void LoadHistogramSettings(const Json::Value&) final;

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

		std::set<std::string> Types;
		
		std::vector<PhysicsData*> SummaryData;

		std::string AnalyzerName;
		std::string LogName;

		std::string DefaultRegexString;
		boost::regex DefaultRegex;
		std::unordered_map<std::string,boost::regex> AllDefaultRegex;
		
		std::shared_ptr<spdlog::logger> console;

		std::map<int,PLOTS::HisHelper1D> h1dsettings;
		std::map<int,PLOTS::HisHelper2D> h2dsettings;
};

#endif
