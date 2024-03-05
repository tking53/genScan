#ifndef __MTAS_STRUCT_HPP__
#define __MTAS_STRUCT_HPP__

#include <vector>

#include <TString.h>

namespace ProcessorStruct{
	struct MtasSegment{
		double energy = -999;
		double timestamp = -999;
		TString Ring = "";
		TString Segment = "";
		TString FrontBack = "";
	};
	static const MtasSegment DEFAULT_MTAS_SEGMENT_STRUCT;

	struct MtasTotal{
		std::vector<double> CenterEnergy = std::vector<double>(6,-999);
		std::vector<double> CenterTS = std::vector<double>(6,-999);
		double CenterSum = -999;
		
		std::vector<double> InnerEnergy = std::vector<double>(6,-999);
		std::vector<double> InnerTS = std::vector<double>(6,-999);
		double InnerSum = -999;

		std::vector<double> MiddleEnergy = std::vector<double>(6,-999);
		std::vector<double> MiddleTS = std::vector<double>(6,-999);
		double MiddleSum = -999;

		std::vector<double> OuterEnergy = std::vector<double>(6,-999);
		std::vector<double> OuterTS = std::vector<double>(6,-999);
		double OuterSum = -999;

		double Total = -999;
	};
	static const MtasTotal DEFAULT_MTAS_TOTAL_STRUCT; 
}

#endif
