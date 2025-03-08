#include <string>

#include "TFile.h"
#include "TH1.h"

void DrawFitHist(const std::string& filename,const std::string& hisname){
	TFile* currfile = TFile::Open(filename.c_str(),"READ");
	TH1* currhis = dynamic_cast<TH1*>(currfile->Get(hisname.c_str()));
	currhis->Draw("lf2 hist");
	currhis->GetListOfFunctions()->At(0)->Draw("same");
}
