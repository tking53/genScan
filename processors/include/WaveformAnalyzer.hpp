#ifndef __WAVEFORM_ANALYZER_HPP__
#define __WAVEFORM_ANALYZER_HPP__

#include "TFitResult.h"
#include "TFitResultPtr.h"

#include "Analyzer.hpp"

#include "PSDCalculator.hpp"
#include "RootFitter.hpp"

class WaveformAnalyzer : public Analyzer {
	public:
		WaveformAnalyzer(const std::string&);
		virtual ~WaveformAnalyzer();

		virtual bool PreProcess(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		virtual bool Process(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		virtual bool PostProcess(EventHistoryManager*,PLOTS::PlotRegistry*,CUTS::CutRegistry*);


		virtual void Init([[maybe_unused]] const pugi::xml_node&);
		virtual void Init([[maybe_unused]] const YAML::Node&);
		virtual void Init([[maybe_unused]] const Json::Value&);

		virtual void Finalize();

		virtual void DeclarePlots([[maybe_unused]] PLOTS::PlotRegistry*) const;

	private:
		bool ValidateSettingsString(const std::string&) const;

		boost::regex GenerateRegex(const std::string&,const std::string&,const std::string&);

		void InsertAdditionalTypes(const std::string&);

		std::set<boost::regex> KnownWaveSettings;
		std::set<boost::regex> KnownTraceSettings;
		std::vector<std::pair<boost::regex,PSDCalculator>> WaveSettings;
		std::vector<std::pair<boost::regex,RootFitter>> TraceFitSettings;

		TFitResultPtr FitResult;
		
		std::chrono::time_point<std::chrono::high_resolution_clock> fit_start_time;
		std::chrono::time_point<std::chrono::high_resolution_clock> fit_stop_time;
		double fittime;

		int MaxSaveFits;
		int currsave;
		int NumTraceFits;
};

#endif
