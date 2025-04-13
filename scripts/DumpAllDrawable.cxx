#include <string>
#include <regex>

#include "TROOT.h"
#include "TFile.h"
#include "TString.h"
#include "TKey.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"

void DumpTH1(TFile*& f,const std::string& actualname,const std::string& drawoptions,double xlow,double xhigh){
	auto currobj = dynamic_cast<TH1*>(f->Get(actualname.c_str()));
	currobj->GetXaxis()->SetRangeUser(xlow,xhigh);
	currobj->Draw(drawoptions.c_str());
}

void DumpTH2(TFile*& f,const std::string& actualname,const std::string& drawoptions,double xlow,double xhigh,double ylow,double yhigh){
	auto currobj = dynamic_cast<TH2*>(f->Get(actualname.c_str()));
	currobj->GetXaxis()->SetRangeUser(xlow,xhigh);
	currobj->GetYaxis()->SetRangeUser(ylow,yhigh);
	currobj->Draw(drawoptions.c_str());
}

void DumpAllDrawable(const std::string& filename="GenPeakFitterResults.root",const std::string& restr = ".*",const std::string& drawoptions="",double xlow = 1.0,double xhigh = 4000.0,double ylow = 1.0, double yhigh = 4000.0,const std::string& drawablename="TH1"){
	TFile* f = new TFile(filename.c_str(),"OPEN");
	TIter next(f->GetListOfKeys());
	TKey *key;
	std::regex re(restr);

	bool AllSame = false;
	TString dopt(drawoptions.c_str());
	dopt.ToLower();
	TCanvas* currcanvas;
	std::string canvasname;
	if( dopt.Contains("same") ){
		AllSame = true;
		canvasname = "Scratch";
		currcanvas = new TCanvas(canvasname.c_str(),canvasname.c_str(),600,600);
	}
	while ((key = (TKey*)next())) {
		TClass *clsPtr = gROOT->GetClass(key->GetClassName());
		auto actualname = std::string(key->GetName());
		if( std::regex_match(actualname,re) ){
			TString name = key->GetClassName();
			if( name.Contains(drawablename.c_str()) ){
				if( not AllSame ){
					canvasname = actualname+"_Scratch";
					currcanvas = new TCanvas(canvasname.c_str(),canvasname.c_str(),600,600);
				}
				if( name.Contains("TH1") ){
					DumpTH1(f,actualname,drawoptions,xlow,xhigh);
					currcanvas->SetLogy(1);
				}
				if( name.Contains("TH2") ){
					DumpTH2(f,actualname,drawoptions,xlow,xhigh,ylow,yhigh);
				}
			}
		}
	}
}
