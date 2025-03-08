#ifndef __WAVEFORM_ANALYZER_HPP__
#define __WAVEFORM_ANALYZER_HPP__

#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TF1.h"

#include "Analyzer.hpp"

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
		struct WaveFormParams {
			std::pair<size_t,size_t> PreTriggerBounds;
			std::pair<size_t,size_t> PostTriggerBounds;
			std::vector<size_t> QDCBounds;

			bool HasPSD;
			std::tuple<size_t,size_t,size_t> FixedPSDBounds;
			std::tuple<size_t,size_t,float> FractionalPSDBounds;

			bool CalcDerivative;

			WaveFormParams(){
				PreTriggerBounds = {0,0};
				PostTriggerBounds = {0,0};
				QDCBounds = {};
				HasPSD = false;
				FixedPSDBounds = {0,0,0};
				FractionalPSDBounds = {0,0,2.0};
				CalcDerivative = false;
				//need to implement CFAR for determining triggering trace
				//as well as pierre's trap filter
				//this is in french though so god help me
			}

			~WaveFormParams() = default;

			WaveFormParams(const WaveFormParams&) = default;
			WaveFormParams(WaveFormParams&&) = default;
			WaveFormParams& operator=(const WaveFormParams&) = default;
			WaveFormParams& operator=(WaveFormParams&&) = default;
		};

		struct TraceFitParams{
			std::vector<std::tuple<int,bool,bool,std::string,double,double,double>> ParamInfo;
			std::pair<double,double> FitRange;
			std::string FitFuncName;
			TF1* fitfunc;
			TH1* fithist;
			TraceFitParams(){
			}

			~TraceFitParams() = default;

			TraceFitParams(const TraceFitParams&) = default;
			TraceFitParams(TraceFitParams&&) = default;
			TraceFitParams& operator=(const TraceFitParams&) = default;
			TraceFitParams& operator=(TraceFitParams&&) = default;
		};

		bool ValidateSettingsString(const std::string&) const;

		void ParsePreTrigger(const pugi::xml_node&,WaveFormParams&);
		void ParsePostTrigger(const pugi::xml_node&,WaveFormParams&);
		void ParseQDC(const pugi::xml_node&,WaveFormParams&);
		void ParsePSD(const pugi::xml_node&,WaveFormParams&);

		boost::regex GenerateRegex(const std::string&,const std::string&,const std::string&);

		void InsertAdditionalTypes(const std::string&);

		std::set<boost::regex> KnownWaveSettings;
		std::set<boost::regex> KnownTraceSettings;
		std::vector<std::pair<boost::regex,WaveFormParams>> WaveSettings;
		std::vector<std::pair<boost::regex,TraceFitParams>> TraceFitSettings;

		TFitResultPtr FitResult;
		
		std::chrono::time_point<std::chrono::high_resolution_clock> fit_start_time;
		std::chrono::time_point<std::chrono::high_resolution_clock> fit_stop_time;
		double fittime;

		int MaxSaveFits;
		int currsave;
		int NumTraceFits;
};

#endif
