#ifndef __TIMING_ANALYZER_HPP__
#define __TIMING_ANALYZER_HPP__

#include "Analyzer.hpp"
#include "TrapezoidFilter.hpp"

class TimingAnalyzer : public Analyzer {
public:
	TimingAnalyzer(const std::string&);
	virtual ~TimingAnalyzer();

	virtual bool PreProcess(EventHistoryManager*, PLOTS::PlotRegistry*, CUTS::CutRegistry*);
	virtual bool Process(EventHistoryManager*, PLOTS::PlotRegistry*, CUTS::CutRegistry*);
	virtual bool PostProcess(EventHistoryManager*, PLOTS::PlotRegistry*, CUTS::CutRegistry*);

	virtual void Init([[maybe_unused]] const pugi::xml_node&);

	virtual void Finalize();

	virtual void DeclarePlots([[maybe_unused]] PLOTS::PlotRegistry*) const;

private:
	virtual boost::regex GenerateRegex(const std::string&, const std::string&, const std::string&);

	void InsertAdditionalTypes(const std::string&);
	
	std::set<boost::regex> KnownSettings;
	std::vector<std::tuple<boost::regex, TrapezoidFilter<float, float>, size_t, size_t>> Settings;
	size_t BadFilter;
};

#endif
