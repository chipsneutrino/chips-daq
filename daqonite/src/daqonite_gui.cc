/*
 * DAQoniteGUI.cc
 *
 *  Created on: Sep 24, 2018
 *      Author: chips
 */

#include "daqonite_gui.h"

DAQoniteGUI::DAQoniteGUI(const TGWindow*p, UInt_t w, UInt_t h) {
	// Create a main frame
	fMain = new TGMainFrame(p,w,h);

	// Create the canvas widget
	fEcanvas = new TRootEmbeddedCanvas("Ecanvas", fMain, 600, 400);
	fMain->AddFrame(fEcanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 1));

	///////// CLB Settings Frame /////////
	TGHorizontalFrame * clbFrame = new TGHorizontalFrame(fMain, 800, 50);

	// Optical Port Set
	fButton_clb_optical = new TGCheckButton(clbFrame, "Mine CLB Opticalite? Port ->");
	fButton_clb_optical->SetState(kButtonDown);
	clbFrame->AddFrame(fButton_clb_optical, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	fPort_clb_optical = new TGNumberEntry(clbFrame, 56015, 9,999, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
			   	   	   	   	   	   	  TGNumberFormat::kNELLimitMinMax, 0, 65535);
	fPort_clb_optical->Connect("ValueSet(Long_t)", "DAQoniteGUI", this, "SetCLBOptPort()");
	(fPort_clb_optical->GetNumberEntry())->Connect("ReturnPressed()", "DAQoniteGUI", this, "SetCLBOptPort()");
	clbFrame->AddFrame(fPort_clb_optical, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

	// Monitoring Port Set
	fButton_clb_monitoring = new TGCheckButton(clbFrame, "Mine CLB Monitorite? Port ->");
	fButton_clb_monitoring->SetState(kButtonDown);
	clbFrame->AddFrame(fButton_clb_monitoring, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	fPort_clb_monitoring = new TGNumberEntry(clbFrame, 56017, 9,999, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
			   	   	   	   	   	   	  TGNumberFormat::kNELLimitMinMax, 0, 65535);
	fPort_clb_monitoring->Connect("ValueSet(Long_t)", "DAQoniteGUI", this, "SetCLBMonPort()");
	(fPort_clb_monitoring->GetNumberEntry())->Connect("ReturnPressed()", "DAQoniteGUI", this, "SetCLBMonPort()");
	clbFrame->AddFrame(fPort_clb_monitoring, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

	fMain->AddFrame(clbFrame, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
	///////////////////////////////////////////

	///////// BBB Settings Frame /////////
	TGHorizontalFrame * bbbFrame = new TGHorizontalFrame(fMain, 800, 50);

	// Optical Port Set
	fButton_bbb_optical = new TGCheckButton(bbbFrame, "Mine BBB Opticalite?");
	fButton_bbb_optical->SetState(kButtonUp);
	bbbFrame->AddFrame(fButton_bbb_optical, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

	// Monitoring Port Set
	fButton_bbb_monitoring = new TGCheckButton(bbbFrame, "Mine BBB Monitorite? Port ->");
	fButton_bbb_monitoring->SetState(kButtonUp);
	bbbFrame->AddFrame(fButton_bbb_monitoring, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	fPort_bbb = new TGNumberEntry(bbbFrame, 6060, 9,999, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
			   	   	   	   	   	   	  TGNumberFormat::kNELLimitMinMax, 0, 65535);
	fPort_bbb->Connect("ValueSet(Long_t)", "DAQoniteGUI", this, "SetBBBPort()");
	(fPort_bbb->GetNumberEntry())->Connect("ReturnPressed()", "DAQoniteGUI", this, "SetBBBPort()");
	bbbFrame->AddFrame(fPort_bbb, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

	fMain->AddFrame(bbbFrame, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
	///////////////////////////////////////////

	/////////  Saving Setting Frame   /////////
	TGHorizontalFrame * saveFrame = new TGHorizontalFrame(fMain, 400, 50);

	// Save Data Button
	fSaveDataButton = new TGCheckButton(saveFrame, "Save Data");
	fSaveDataButton->SetState(kButtonUp);
	saveFrame->AddFrame(fSaveDataButton, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

	// Save file name
	TGLabel *fileLabel = new TGLabel(saveFrame, "Shipping Container:");
	saveFrame->AddFrame(fileLabel, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	fFileNameEntry = new TGTextEntry(saveFrame, "");
	saveFrame->AddFrame(fFileNameEntry, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

	fMain->AddFrame(saveFrame, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
	///////////////////////////////////////////

	/////////     Control Frame      //////////
	TGHorizontalFrame * controlFrame = new TGHorizontalFrame(fMain, 400, 50);

	// Mining Start/Stop Button
	fMiningButton = new TGTextButton(controlFrame, "Start Mining");
	//fMiningButton->Connect("Clicked()","DAQoniteGUI", this, "ToggleMining()");
	controlFrame->AddFrame(fMiningButton, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
	fMining = false;

	// Exit Button
	TGTextButton *exit = new TGTextButton(controlFrame, "&Done for the day", "gApplication->Terminate(0)");
	controlFrame->AddFrame(exit, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

	fMain->AddFrame(controlFrame, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
	///////////////////////////////////////////

	// Set the name of the main frame
	fMain->SetWindowName("DAQoniteGUI - by Josh Tingey MSci, JoshTingeyDAQDemon.Josh");
	fMain->MapSubwindows();
	fMain->Resize(fMain->GetDefaultSize());
	fMain->MapWindow();

	optPort = 56015;
	monPort = 56017;
}

DAQoniteGUI::~DAQoniteGUI() {
	// Cleanup used widgets: frames, buttons, layout hints
	fMain->Cleanup();
	//delete fMain;
}

void DAQoniteGUI::ToggleMining() {

	fMiningButton->SetState(kButtonDown);
	if (!fMining) {
		fMiningButton->SetText("&Stop Mining");
		fMining = true;

		fFileNameEntry->SetState(false);
		fPort_clb_optical->SetState(false);
		fPort_clb_monitoring->SetState(false);
		fPort_bbb->SetState(false);
		fButton_clb_optical->SetState(kButtonDisabled,true);
		fButton_clb_monitoring->SetState(kButtonDisabled,true);
		fButton_bbb_optical->SetState(kButtonDisabled,true);
		fButton_bbb_monitoring->SetState(kButtonDisabled,true);
		fSaveDataButton->SetState(kButtonDisabled,true);

	} else {
		gApplication->Terminate(0);
	}
	fMiningButton->SetState(kButtonUp);
}

void DAQoniteGUI::Setup() {
	// Load a DAQ_handler
	std::cout << "SETUP!!!" << std::endl;
}

void DAQoniteGUI::SetCLBOptPort() {
	optPort = (unsigned int)fPort_clb_optical->GetNumberEntry()->GetIntNumber();
	std::cout << "CLB Optical port set to: " << optPort << std::endl;
}

void DAQoniteGUI::SetCLBMonPort() {
	monPort = (unsigned int)fPort_clb_monitoring->GetNumberEntry()->GetIntNumber();
	std::cout << "CLB Monitoring port set to: " << monPort << std::endl;
}

void DAQoniteGUI::SetBBBPort() {
	bbbPort = (unsigned int)fPort_bbb->GetNumberEntry()->GetIntNumber();
	std::cout << "BBB port set to: " << bbbPort << std::endl;
}
