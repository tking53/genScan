#ifndef __GENERIC_ANALYZER_HPP__
#define __GENERIC_ANALYZER_HPP__

#include "Analyzer.hpp"

class GenericAnalyzer : public Analyzer{
	public:
		GenericAnalyzer();
		bool PreProcess() final;
		bool Process() final;
		bool PostProcess() final;
};

#endif
