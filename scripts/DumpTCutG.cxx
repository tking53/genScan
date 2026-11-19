#include <TCutG.h>
#include <iostream>
#include <string>
#include <regex>
#include <filesystem>

#include "TROOT.h"
#include "TFile.h"
#include "TString.h"
#include "TKey.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"

void DumpTCutG(const std::string& filename = "GenPeakFitterResults.root", const std::string& hisname = "EXP_11012", int idx = 1) {
	TFile* f = new TFile(filename.c_str(), "OPEN");
	TIter next(f->GetListOfKeys());
	TKey* key;

	std::filesystem::path iname(filename);
	std::string name = iname.stem().string();
	iname.replace_extension("Cut.cxx");

	std::regex re(hisname);

	while ((key = (TKey*)next())) {
		TClass* clsPtr = gROOT->GetClass(key->GetClassName());
		auto actualname = std::string(key->GetName());
		if (std::regex_match(actualname, re)) {
			TCutG* cut = dynamic_cast<TCutG*>(dynamic_cast<TH1*>(f->Get(actualname.c_str()))->GetListOfFunctions()->At(idx));
			cut->SetName(name.c_str());
			cut->SaveAs(iname.c_str());
		}
	}
}
