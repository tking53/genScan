#include <string>

#include "TROOT.h"
#include "TFile.h"
#include "TString.h"
#include "TKey.h"
#include "TCanvas.h"
#include "TH1.h"

void DumpAllDrawable(const std::string& filename="GenPeakFitterResults.root",const std::string& drawablename="TH1",const std::string& drawoptions=""){
	TFile* f = new TFile(filename.c_str(),"OPEN");
	TIter next(f->GetListOfKeys());
	TKey *key;

	while ((key = (TKey*)next())) {
		TClass *clsPtr = gROOT->GetClass(key->GetClassName());
		TString name = key->GetClassName();
		if( name.Contains(drawablename.c_str()) ){
			auto canvasname = std::string(key->GetName())+"_Scratch";
			auto currcanvas = new TCanvas(canvasname.c_str(),canvasname.c_str(),600,600);
			auto currobj = dynamic_cast<TH1*>(f->Get(key->GetName()));
			currobj->GetXaxis()->SetRangeUser(1.0,4000.0);
			currobj->Draw(drawoptions.c_str());
			currcanvas->SetLogy(1);
		}
	}
}
