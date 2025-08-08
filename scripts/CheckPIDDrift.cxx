#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include "TFile.h"
#include "TH1.h"

TH1* FetchHis(const std::string& file,const std::string& his){
	TFile* f = TFile::Open(file.c_str(),"READ");
	TH1* h = dynamic_cast<TH1*>(f->Get(his.c_str()));
	h->SetDirectory(0);
	return h;
}

//PID_7 is what we usually want to deal with
void CheckPIDDrift(const std::string& txtfile,const std::string& hisname){
	//txtfile is a text file containing the files in order they're to be compared
	//hisname is the name of the histogram to fetch and compare between them

	std::vector<std::string> files;
	std::string fname;
	std::ifstream input(txtfile);
	while( input >> fname ){
		files.push_back(fname);
	}
	input.close();

	std::vector<double> means;
	std::vector<double> meanserror;
	std::ofstream diff("PIDDelta.dat");
	for( size_t ii = 1; ii < files.size(); ++ii ){
		auto h1 = FetchHis(files.at(ii-1),hisname);	
		auto h2 = FetchHis(files.at(ii),hisname);

		means.push_back(h1->GetMean());
		meanserror.push_back(h1->GetMeanError());
		
		auto meandiff = h2->GetMean() - h1->GetMean();
		diff << files.at(ii) << " " << files.at(ii-1) << " " << meandiff << std::endl;
	}
	diff.close();
	auto f = FetchHis(files.back(),hisname);
	means.push_back(f->GetMean());
	meanserror.push_back(f->GetMeanError());

	std::ofstream output("PIDDrift.yaml");
	output << "HisName: " << hisname << std::endl;
	output << "Results: " << std::endl;
	for( size_t ii = 0; ii < files.size(); ++ii ){
		output << "  - FileName: " << files.at(ii) << std::endl;
		output << "    Mean: " << means.at(ii) << std::endl;
		output << "    Error: " << meanserror.at(ii) << std::endl;
	}
	output.close();

	std::ofstream dump("PIDDrift.dat")
	for( size_t ii = 0; ii < files.size(); ++ii ){
		dump << files.at(ii) << " " << means.at(ii) << " " << meanserror.at(ii) << std::endl;
	}
	dump.close();

	//PID_7
	//Start from bulkscan_runX.root and go to bulkscan_runY.root
	//Go pairwise and generate the normalized binChi2 
	//from that make the Chi2 distribution and fit with a Gaussian
	//If the centroid isn't around zero then we notify a split
	//Norm Range 150 200
}
