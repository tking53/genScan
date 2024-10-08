#include "TGButton.h"
#include "TRootEmbeddedCanvas.h"
#include "TGLayout.h"
#include "TGTextEntry.h"
#include "TGTextEntry.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TSocket.h"
#include "TMessage.h"
#include "RQ_OBJECT.h"
#include <TGClient.h>
#include <TGTextBuffer.h>
#include <set>

class Spy {

	RQ_OBJECT("Spy")

	private:
		TGMainFrame         *fMain;
		TRootEmbeddedCanvas *fCanvas;
		TGHorizontalFrame   *fHorz;
		TGHorizontalFrame   *fHorz2;
		TGHorizontalFrame   *fHorz3;
		TGLayoutHints       *fLbut;
		TGLayoutHints       *fLhorz;
		TGLayoutHints       *fLcan;
		TGButton            *fHpx;
		TGButton            *fHpxpy;
		TGButton            *fHprof;
		TGButton            *fConnect;
		TGButton            *fQuit;
		TGButton            *fUpdateLists;
		TGButton            *fPlotSelected;
		TGTextEntry         *fSelectHistogram;
		TGTextEntry         *fSelectOptions;
		TSocket             *fSock;
		TH1                 *fHist;
		std::set<std::string>    fTemp;
		std::vector<TH1*> fTempHis;

	public:
		Spy();
		~Spy();

		void Connect();
		void DoButton();

		void PlotSelected();
};

void Spy::DoButton()
{
	// Ask for histogram...

	if (!fSock->IsValid())
		return;

	TGButton *btn = (TGButton *) gTQSender;
	switch (btn->WidgetId()) {
		case 1:
			fSock->Send("Raw");
			break;
		case 2:
			fSock->Send("Scalar");
			break;
		case 3:
			fSock->Send("Cal");
			break;
	}
	TMessage *mess;
	if (fSock->Recv(mess) <= 0) {
		Error("Spy::DoButton", "error receiving message");
		return;
	}

	if (fHist) delete fHist;
	if (mess->GetClass()->InheritsFrom(TH1::Class())) {
		fHist = (TH1*) mess->ReadObject(mess->GetClass());
		if (mess->GetClass()->InheritsFrom(TH2::Class()))
			fHist->Draw("colz");
		else
			fHist->Draw();
		fCanvas->GetCanvas()->Modified();
		fCanvas->GetCanvas()->Update();
	}

	delete mess;
}

void Spy::Connect()
{
	// Connect to SpyServ
	fSock = new TSocket("localhost", 9090);
	fConnect->SetState(kButtonDisabled);
	fHpx->SetState(kButtonUp);
	fHpxpy->SetState(kButtonUp);
	fHprof->SetState(kButtonUp);
}

Spy::Spy()
{
	// Create a main frame
	fMain = new TGMainFrame(0, 100, 100);
	fMain->SetCleanup(kDeepCleanup);

	// Create an embedded canvas and add to the main frame, centered in x and y
	// and with 30 pixel margins all around
	fCanvas = new TRootEmbeddedCanvas("Canvas", fMain, 600, 400);
	fLcan = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY,30,30,30,30);
	fMain->AddFrame(fCanvas, fLcan);

	// Create a horizontal frame containing three text buttons
	fLhorz = new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 30);
	fHorz = new TGHorizontalFrame(fMain, 100, 100);
	fMain->AddFrame(fHorz, fLhorz);

	// Create three text buttons to get objects from server
	// Add to horizontal frame
	fLbut = new TGLayoutHints(kLHintsCenterX, 10, 10, 0, 0);

	fHpx = new TGTextButton(fHorz, "Raw", 1);
	fHpx->SetState(kButtonDisabled);
	fHpx->Connect("Clicked()", "Spy", this, "DoButton()");
	fHorz->AddFrame(fHpx, fLbut);

	fHpxpy = new TGTextButton(fHorz, "Scalar", 2);
	fHpxpy->SetState(kButtonDisabled);
	fHpxpy->Connect("Clicked()", "Spy", this, "DoButton()");
	fHorz->AddFrame(fHpxpy, fLbut);

	fHprof = new TGTextButton(fHorz, "Cal", 3);
	fHprof->SetState(kButtonDisabled);
	fHprof->Connect("Clicked()", "Spy", this, "DoButton()");
	fHorz->AddFrame(fHprof, fLbut);

	// Create a horizontal frame containing two text buttons
	fHorz2 = new TGHorizontalFrame(fMain, 100, 100);
	fMain->AddFrame(fHorz2, fLhorz);
		
	TGFont *ufont;         // will reflect user font changes
	ufont = gClient->GetFont("-*-helvetica-medium-r-*-*-12-*-*-*-*-*-iso8859-1");

	TGGC   *uGC;           // will reflect user GC changes
			       //
	GCValues_t valEntryBox1D;
	gClient->GetColorByName("#000000",valEntryBox1D.fForeground);
	gClient->GetColorByName("#e7e7e7",valEntryBox1D.fBackground);
	valEntryBox1D.fFillStyle = kFillSolid;
	valEntryBox1D.fFont = ufont->GetFontHandle();
	valEntryBox1D.fGraphicsExposures = kFALSE;
	uGC = gClient->GetGC(&valEntryBox1D,kTRUE);
		
	fSelectHistogram = new TGTextEntry(fHorz2, new TGTextBuffer(64),-1,uGC->GetGC(),ufont->GetFontStruct(),kHorizontalFrame | kSunkenFrame | kOwnBackground);
	fSelectHistogram->SetName("fSelectHistogram");
	fSelectHistogram->Resize(102,21);
	fHorz2->AddFrame(fSelectHistogram,fLbut);
		
	fSelectOptions = new TGTextEntry(fHorz2, new TGTextBuffer(64),-1,uGC->GetGC(),ufont->GetFontStruct(),kHorizontalFrame | kSunkenFrame | kOwnBackground);
	fSelectOptions->SetName("fSelectOptions");
	fSelectOptions->Resize(102,21);
	fHorz2->AddFrame(fSelectOptions,fLbut);

	fPlotSelected = new TGTextButton(fHorz2, "Plot", 4);
	fPlotSelected->Connect("Clicked()", "Spy", this, "PlotSelected()");
	fHorz2->AddFrame(fPlotSelected, fLbut);

	// Create a horizontal frame containing two text buttons
	fHorz3 = new TGHorizontalFrame(fMain, 100, 100);
	fMain->AddFrame(fHorz3, fLhorz);

	// Create "Connect" and "Quit" buttons
	// Add to horizontal frame
	fConnect = new TGTextButton(fHorz3, "Connect");
	fConnect->Connect("Clicked()", "Spy", this, "Connect()");
	fHorz3->AddFrame(fConnect, fLbut);

	fQuit = new TGTextButton(fHorz3, "Quit");
	fQuit->SetCommand("gApplication->Terminate()");
	fHorz3->AddFrame(fQuit, fLbut);

	// Set main frame name, map sub windows (buttons), initialize layout
	// algorithm via Resize() and map main frame
	fMain->SetWindowName("Spy on SpyServ");
	fMain->MapSubwindows();
	fMain->Resize(fMain->GetDefaultSize());
	fMain->MapWindow();

	fHist = 0;
}

Spy::~Spy()
{
	// Clean up

	delete fHist;
	delete fSock;
	delete fLbut;
	delete fLhorz;
	delete fLcan;
	delete fHpx;
	delete fHpxpy;
	delete fHprof;
	delete fConnect;
	delete fQuit;
	delete fHorz;
	delete fHorz2;
	delete fCanvas;
	delete fMain;
}

void Spy::PlotSelected(){
	auto id = std::string(fSelectHistogram->GetText());
	auto opts = std::string(fSelectOptions->GetText());
	for (auto& x : opts) {
		x = tolower(x);
	}

	bool hassame = (opts.find("same") != std::string::npos);
	if( not hassame ){
		fTemp.clear();
		fTemp.insert(id);
	}else{
		fTemp.insert(id);
	}

	if( hassame ){
		if( not fTempHis.empty() ){
			for( auto& h : fTempHis ){
				delete h;
			}
			fTempHis.clear();
		}
		for( const auto& cid : fTemp ){
			fSock->Send(cid.c_str());

			TMessage *mess;
			if (fSock->Recv(mess) <= 0) {
				Error("Spy::DoButton", "error receiving message");
				return;
			}

			if (mess->GetClass()->InheritsFrom(TH1::Class())) {
				fTempHis.push_back((TH1*) mess->ReadObject(mess->GetClass()));
			}
		}
		if( not fTempHis.empty() ){
			fTempHis.at(0)->Draw();
			for( size_t ii = 1; ii < fTempHis.size(); ++ii ){
				fTempHis.at(ii)->Draw("same");
			}

			fCanvas->GetCanvas()->Modified();
			fCanvas->GetCanvas()->Update();
		}
	}else{
		fSock->Send(id.c_str());

		TMessage *mess;
		if (fSock->Recv(mess) <= 0) {
			Error("Spy::DoButton", "error receiving message");
			return;
		}

		if (fHist){
			delete fHist;
		}
		if (mess->GetClass()->InheritsFrom(TH1::Class())) {
			fHist = (TH1*) mess->ReadObject(mess->GetClass());
			if (mess->GetClass()->InheritsFrom(TH2::Class())){
				fHist->Draw("colz");
			}else{
				fHist->Draw();
			}
		}
		fCanvas->GetCanvas()->Modified();
		fCanvas->GetCanvas()->Update();
	}

}

void spy()
{
	new Spy;
}
