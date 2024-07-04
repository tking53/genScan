#ifndef __WAVEFORM_ANALYZER_HPP__
#define __WAVEFORM_ANALYZER_HPP__

#include "Analyzer.hpp"

class WaveformAnalyzer : public Analyzer {
	public:
		WaveformAnalyzer(const std::string&);
		virtual ~WaveformAnalyzer() = default;

		virtual bool PreProcess(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		virtual bool Process(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*);
		virtual bool PostProcess(EventSummary&,PLOTS::PlotRegistry*,CUTS::CutRegistry*);


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

			WaveFormParams(){
				PreTriggerBounds = {0,0};
				PostTriggerBounds = {0,0};
				QDCBounds = {};
				HasPSD = false;
				FixedPSDBounds = {0,0,0};
				FractionalPSDBounds = {0,0,2.0};
			}

			~WaveFormParams() = default;

			WaveFormParams(const WaveFormParams&) = default;
			WaveFormParams(WaveFormParams&&) = default;
			WaveFormParams& operator=(const WaveFormParams&) = default;
			WaveFormParams& operator=(WaveFormParams&&) = default;
		};

		bool ValidateSettingsString(const std::string&) const;

		void ParsePreTrigger(const pugi::xml_node&,WaveFormParams&);
		void ParsePostTrigger(const pugi::xml_node&,WaveFormParams&);
		void ParseQDC(const pugi::xml_node&,WaveFormParams&);
		void ParsePSD(const pugi::xml_node&,WaveFormParams&);

		boost::regex GenerateRegex(const std::string&,const std::string&,const std::string&);

		void InsertAdditionalTypes(const std::string&);

		std::set<boost::regex> KnownWaveSettings;
		std::vector<std::pair<boost::regex,WaveFormParams>> WaveSettings;

};

#endif
