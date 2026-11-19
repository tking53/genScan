#include <TTreeReaderValue.h>
#include <iostream>

#include <TFile.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TString.h>

void DumpXML(const std::string& filename) {
	auto file = TFile::Open(filename.c_str(), "READ");
	auto configdata = file->Get<TTree>("configdata");

	TString* xmlfile = nullptr;
	TString* data = nullptr;

	configdata->SetBranchAddress("filename", &xmlfile);
	configdata->SetBranchAddress("data", &data);

	configdata->GetEntry(0);

	std::cout << xmlfile->Data() << std::endl;
	std::cout << data->Data() << std::endl;
}
