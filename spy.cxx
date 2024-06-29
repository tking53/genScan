#include "TGButton.h"
#include "TRootEmbeddedCanvas.h"
#include "TGLayout.h"
#include "TGComboBox.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TSocket.h"
#include "TMessage.h"
#include "RQ_OBJECT.h"
#include <iostream>

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
		TGButton            *fPlotSelected1D;
		TGButton            *fPlotSelected2D;
		TGComboBox          *fCBox1D;
		TGComboBox          *fCBox2D;
		TSocket             *fSock;
		TH1                 *fHist;

	public:
		Spy();
		~Spy();

		void Connect();
		void DoButton();

		void PrintSelected1D();
		void PrintSelected2D();
		void PrintLists();
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
		
	fUpdateLists = new TGTextButton(fHorz2, "UpdateLists");
	fUpdateLists->Connect("Clicked()", "Spy", this, "PrintLists()");
	fHorz2->AddFrame(fUpdateLists, fLbut);
		
	fCBox1D = new TGComboBox(fHorz2,-1,kHorizontalFrame | kSunkenFrame | kOwnBackground);
	fCBox1D->SetName("fCBox1D");
	fCBox1D->AddEntry("Entry 1 ",0);
	fCBox1D->AddEntry("Entry 2 ",1);
	fCBox1D->AddEntry("Entry 3 ",2);
	fCBox1D->AddEntry("Entry 4 ",3);
	fCBox1D->AddEntry("Entry 5 ",4);
	fCBox1D->AddEntry("Entry 6 ",5);
	fCBox1D->AddEntry("Entry 7 ",6);
	fCBox1D->Resize(102,21);
	fCBox1D->Select(-1);
	fHorz2->AddFrame(fCBox1D,fLbut);

	fPlotSelected1D = new TGTextButton(fHorz2, "Plot1D");
	fPlotSelected1D->Connect("Clicked()", "Spy", this, "PrintSelected1D()");
	fHorz2->AddFrame(fPlotSelected1D, fLbut);
		
	fCBox2D = new TGComboBox(fHorz2,-1,kHorizontalFrame | kSunkenFrame | kOwnBackground);
	fCBox2D->SetName("fCBox2D");
	fCBox2D->AddEntry("Entry 1 ",0);
	fCBox2D->AddEntry("Entry 2 ",1);
	fCBox2D->AddEntry("Entry 3 ",2);
	fCBox2D->AddEntry("Entry 4 ",3);
	fCBox2D->AddEntry("Entry 5 ",4);
	fCBox2D->AddEntry("Entry 6 ",5);
	fCBox2D->AddEntry("Entry 7 ",6);
	fCBox2D->Resize(102,21);
	fCBox2D->Select(-1);
	fHorz2->AddFrame(fCBox2D,fLbut);

	fPlotSelected2D = new TGTextButton(fHorz2, "Plot2D");
	fPlotSelected2D->Connect("Clicked()", "Spy", this, "PrintSelected2D()");
	fHorz2->AddFrame(fPlotSelected2D, fLbut);


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

void Spy::PrintSelected1D(){
	auto id = fCBox1D->GetSelected();
	std::cout << id << std::endl;
}

void Spy::PrintSelected2D(){
	auto id = fCBox2D->GetSelected();
	std::cout << id << std::endl;
}

void Spy::PrintLists(){
	fCBox1D->RemoveAll();
	fCBox2D->RemoveAll();

	fCBox1D->AddEntry("new 1",0);
	fCBox2D->AddEntry("new 1",0);
}

void spy()
{
	new Spy;
}
