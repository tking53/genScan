#include <iostream>

#include "genscan.hpp"

#include "CompassROOTUnpacker.hpp"

genscan::genscan(){
}

void genscan::SetCfgFile(std::string* cfg){
	cfgFile = cfg;
}

void genscan::SetInputFile(std::string* infile){
	inputFile = infile;
}

void genscan::SetOutputFile(std::string* outfile){
	outputFile = outfile;
}

void genscan::SetEvtBuild(bool evtbuild){
	evtBuild = evtbuild;
}

void genscan::DoScan(){
	//need to determine which unpacker to use
	//either do this based on file extension of by command line option?
	//default to a specific file extension?
	//probably safest to just read the file extension
	
	unpacker = new CompassROOTUnpacker();
	unpacker->SetCurrFile(inputFile);
	std::cout << "Current File : " << *(unpacker->GetCurrFile()) << std::endl;
}
