/*
 * daqonite_gui.cc
 *
 *  Created on: Sep 24, 2018
 *      Author: chips
 *
 */

#include "daqonite_gui.h"

#define PLOTLENGTH 100
#define UPDATERATE 2000
#define PMTSPERPOM 30
#define HIGHRATE 10000

DAQoniteGUI::DAQoniteGUI(const TGWindow*p, UInt_t w, UInt_t h) {

	// main frame
	fMainFrame = new TGMainFrame(gClient->GetRoot(),10,10,kMainFrame | kVerticalFrame);
	fMainFrame->SetName("fMainFrame");
	fMainFrame->SetLayoutBroken(kTRUE);

	ULong_t ucolor;        // will reflect user color changes
	gClient->GetColorByName("#cccccc",ucolor);

	// Frame 1 //
	TGHorizontalFrame *fFrame1 = new TGHorizontalFrame(fMainFrame,800,200,kHorizontalFrame | kRaisedFrame,ucolor);
	fFrame1->SetName("fFrame1");
	fFrame1->SetLayoutBroken(kTRUE);

	// embedded canvas
	TRootEmbeddedCanvas *fECanvas1 = new TRootEmbeddedCanvas(0,fFrame1,790,190,kRaisedFrame);
	fECanvas1->SetName("fECanvas1");
	Int_t wfECanvas1 = fECanvas1->GetCanvasWindowId();
	TCanvas *c123 = new TCanvas("c123", 10, 10, wfECanvas1);
	fECanvas1->AdoptCanvas(c123);
	fFrame1->AddFrame(fECanvas1, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fECanvas1->MoveResize(5,5,790,190);

	fMainFrame->AddFrame(fFrame1, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fFrame1->MoveResize(5,5,800,200);
	/////////////

	// Frame 2 //
	TGHorizontalFrame *fFrame2 = new TGHorizontalFrame(fMainFrame,800,200,kHorizontalFrame | kRaisedFrame,ucolor);
	fFrame2->SetName("fFrame2");
	fFrame2->SetLayoutBroken(kTRUE);

	// embedded canvas
	TRootEmbeddedCanvas *fECanvas2 = new TRootEmbeddedCanvas(0,fFrame2,790,190,kRaisedFrame);
	fECanvas2->SetName("fECanvas2");
	Int_t wfECanvas2 = fECanvas2->GetCanvasWindowId();
	TCanvas *c124 = new TCanvas("c124", 10, 10, wfECanvas2);
	fECanvas2->AdoptCanvas(c124);
	fFrame2->AddFrame(fECanvas2, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fECanvas2->MoveResize(5,5,790,190);

	fMainFrame->AddFrame(fFrame2, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fFrame2->MoveResize(5,215,800,200);
	/////////////

	// Frame 4 //
	TGHorizontalFrame *fFrame4 = new TGHorizontalFrame(fMainFrame,800,200,kHorizontalFrame | kRaisedFrame,ucolor);
	fFrame4->SetName("fFrame4");
	fFrame4->SetLayoutBroken(kTRUE);

	// embedded canvas
	TRootEmbeddedCanvas *fECanvas3 = new TRootEmbeddedCanvas(0,fFrame4,790,190,kRaisedFrame);
	fECanvas3->SetName("fECanvas3");
	Int_t wfECanvas3 = fECanvas3->GetCanvasWindowId();
	TCanvas *c125 = new TCanvas("c125", 10, 10, wfECanvas3);
	fECanvas3->AdoptCanvas(c125);
	fFrame4->AddFrame(fECanvas3, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fECanvas3->MoveResize(5,5,790,190);

	fMainFrame->AddFrame(fFrame4, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fFrame4->MoveResize(5,635,800,200);
	/////////////

	// Frame 3 //
	TGHorizontalFrame *fFrame3 = new TGHorizontalFrame(fMainFrame,800,200,kHorizontalFrame | kRaisedFrame,ucolor);
	fFrame3->SetName("fFrame3");
	fFrame3->SetLayoutBroken(kTRUE);
	TGVertical3DLine *fVerticalLine = new TGVertical3DLine(fFrame3,1,190);
	fVerticalLine->SetName("fVerticalLine");
	fFrame3->AddFrame(fVerticalLine, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fVerticalLine->MoveResize(400,5,1,190);

	gClient->GetColorByName("#7d99d1",ucolor);
	fRunStatusLabel = new TGLabel(fFrame3,"In The Lunch Room",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fRunStatusLabel->SetTextJustify(36);
	fRunStatusLabel->SetMargins(0,0,0,0);
	fRunStatusLabel->SetWrapLength(-1);
	fFrame3->AddFrame(fRunStatusLabel, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fRunStatusLabel->MoveResize(5,5,390,45);

	fRunTypeLabel = new TGLabel(fFrame3,"Ore:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fRunTypeLabel->SetTextJustify(36);
	fRunTypeLabel->SetMargins(0,0,0,0);
	fRunTypeLabel->SetWrapLength(-1);
	fFrame3->AddFrame(fRunTypeLabel, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fRunTypeLabel->MoveResize(5,58,190,40);

	fRunNumLabel = new TGLabel(fFrame3,"Dig:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fRunNumLabel->SetTextJustify(36);
	fRunNumLabel->SetMargins(0,0,0,0);
	fRunNumLabel->SetWrapLength(-1);
	fFrame3->AddFrame(fRunNumLabel, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fRunNumLabel->MoveResize(205,58,190,40);

	fRunTimeLabel = new TGLabel(fFrame3,"Time:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fRunTimeLabel->SetTextJustify(36);
	fRunTimeLabel->SetMargins(0,0,0,0);
	fRunTimeLabel->SetWrapLength(-1);
	fFrame3->AddFrame(fRunTimeLabel, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fRunTimeLabel->MoveResize(5,106,190,40);

	fRunPacketsLabel = new TGLabel(fFrame3,"Loads:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fRunPacketsLabel->SetTextJustify(36);
	fRunPacketsLabel->SetMargins(0,0,0,0);
	fRunPacketsLabel->SetWrapLength(-1);
	fFrame3->AddFrame(fRunPacketsLabel, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fRunPacketsLabel->MoveResize(205,106,190,40);

	fRunFileLabel = new TGLabel(fFrame3,"Container:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fRunFileLabel->SetTextJustify(36);
	fRunFileLabel->SetMargins(0,0,0,0);
	fRunFileLabel->SetWrapLength(-1);
	fFrame3->AddFrame(fRunFileLabel, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fRunFileLabel->MoveResize(5,154,390,40);

	fPomIDEntry = new TGNumberEntry(fFrame3, (Double_t) 0,6,-1,(TGNumberFormat::EStyle) 5);
	fPomIDEntry->SetName("fPomIDEntry");
	fFrame3->AddFrame(fPomIDEntry, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fPomIDEntry->MoveResize(630,66,60,22);

	fChannelEntry = new TGNumberEntry(fFrame3, (Double_t) 0,6,-1,(TGNumberFormat::EStyle) 5);
	fChannelEntry->SetName("fChannelEntry");
	fFrame3->AddFrame(fChannelEntry, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fChannelEntry->MoveResize(710,66,60,22);

	fSpecifyButton = new TGCheckButton(fFrame3,"Specify [CLB] and [Channel]:",-1,TGCheckButton::GetDefaultGC()(),TGCheckButton::GetDefaultFontStruct(),kRaisedFrame);
	fSpecifyButton->Connect("Clicked()","DAQoniteGUI",this,"toggleSpecific()");
	fSpecifyButton->SetTextJustify(36);
	fSpecifyButton->SetMargins(0,0,0,0);
	fSpecifyButton->SetWrapLength(-1);
	fFrame3->AddFrame(fSpecifyButton, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fSpecifyButton->MoveResize(410,66,180,22);

	fBackButton = new TGTextButton(fFrame3,"<--- BACK",-1,TGTextButton::GetDefaultGC()(),TGTextButton::GetDefaultFontStruct(),kRaisedFrame);
	fBackButton->Connect("Clicked()","DAQoniteGUI",this,"pageBackward()");
	fBackButton->SetTextJustify(36);
	fBackButton->SetMargins(0,0,0,0);
	fBackButton->SetWrapLength(-1);
	fBackButton->Resize(190,45);
	fBackButton->ChangeBackground(ucolor);
	fFrame3->AddFrame(fBackButton, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fBackButton->MoveResize(405,5,190,45);

	fForwardButton = new TGTextButton(fFrame3,"FORWARD --->",-1,TGTextButton::GetDefaultGC()(),TGTextButton::GetDefaultFontStruct(),kRaisedFrame);
	fForwardButton->Connect("Clicked()","DAQoniteGUI",this,"pageForward()");
	fForwardButton->SetTextJustify(36);
	fForwardButton->SetMargins(0,0,0,0);
	fForwardButton->SetWrapLength(-1);
	fForwardButton->Resize(190,45);
	fForwardButton->ChangeBackground(ucolor);
	fFrame3->AddFrame(fForwardButton, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fForwardButton->MoveResize(605,5,190,45);

	fFactLabel1 = new TGLabel(fFrame3,"Fact:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fFactLabel1->SetTextJustify(36);
	fFactLabel1->SetMargins(0,0,0,0);
	fFactLabel1->SetWrapLength(-1);
	fFrame3->AddFrame(fFactLabel1, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fFactLabel1->MoveResize(405,106,190,40);

	fFactLabel2 = new TGLabel(fFrame3,"Fact:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fFactLabel2->SetTextJustify(36);
	fFactLabel2->SetMargins(0,0,0,0);
	fFactLabel2->SetWrapLength(-1);
	fFrame3->AddFrame(fFactLabel2, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fFactLabel2->MoveResize(605,106,190,40);

	fFactLabel3 = new TGLabel(fFrame3,"Fact:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fFactLabel3->SetTextJustify(36);
	fFactLabel3->SetMargins(0,0,0,0);
	fFactLabel3->SetWrapLength(-1);
	fFrame3->AddFrame(fFactLabel3, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fFactLabel3->MoveResize(405,154,190,40);

	fFactLabel4 = new TGLabel(fFrame3,"Fact:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fFactLabel4->SetTextJustify(36);
	fFactLabel4->SetMargins(0,0,0,0);
	fFactLabel4->SetWrapLength(-1);
	fFrame3->AddFrame(fFactLabel4, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fFactLabel4->MoveResize(605,154,190,40);

	fMainFrame->AddFrame(fFrame3, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fFrame3->MoveResize(5,425,800,200);
	/////////////

	fMainFrame->SetMWMHints(kMWMDecorAll,
							kMWMFuncAll,
							kMWMInputModeless);
	fMainFrame->MapSubwindows();

	fMainFrame->Resize(fMainFrame->GetDefaultSize());
	fMainFrame->MapWindow();
	fMainFrame->Resize(812,841);

	// Set up the other variables...
	fCanvas1 = fECanvas1->GetCanvas();
	fCanvas2 = fECanvas2->GetCanvas();
	fCanvas3 = fECanvas3->GetCanvas();

	TImage *logo = TImage::Open("../data/logo.png");
	logo->SetConstRatio(kFALSE);

	fCanvas1->cd();
	fTotalRatePlot = makeTotalRatePlot();
	logo->Draw();
	fCanvas1->Update();

	fCanvas2->cd();
	fPacketRatePlot = makePacketRatePlot();
	logo->Draw();
	fCanvas2->Update();

	fCanvas3->cd();
	fRateHeatMapPlot = makeHeatMapPlot();
	logo->Draw();
	fCanvas3->Update();

	fPageNum = 0;

	fPacketsReceived = 0;
	fNumUpdates = 0;
	fNumRefresh = 0;
	fRunning = false;
	fModifyPlots = false;
	fWindowPackets = 0;
	fStartPomID = 0;
	fStartTime_ms = 0;

	fActivePOMs.clear();
	fRateArray.clear();

	fRunNumber = 0;
	fStartTime = 0;
	fRunType = 0;
	fActiveChannels = 0;
	fOddChannels = 0;

	drawLabels();
}

DAQoniteGUI::~DAQoniteGUI() {
	fMainFrame->Cleanup();
}

void DAQoniteGUI::addHits(unsigned int pomID, unsigned int channel, unsigned int hits) {
	int pomIndex = 0;
	for(pomIndex = 0; pomIndex < (int)fActivePOMs.size(); pomIndex++) {
	    if (fActivePOMs[pomIndex] == pomID) {
	    	fRateArray[pomIndex][channel] += hits;
	    	return;
	    }
	}
	// If we get here we have not yet recorded hits for this POM
	// Therefore, add the POM to the monitoring
	addPom(pomID, pomIndex);
}

void DAQoniteGUI::addHeader(UInt_t pomID, UInt_t time_ms) {
	if (fPacketsReceived == 0) { // If first header set as timer POM and set start time
		fPacketsReceived++;
		fStartPomID = pomID;
		fStartTime_ms = time_ms;
		fStartTime = time_ms;
	} else if (pomID != fStartPomID) { // If not timer POM skip
		fPacketsReceived++;
		fWindowPackets++;
		return;
	} else if ((time_ms - fStartTime_ms) < UPDATERATE) { // If not greater than UPDATERATE skip
		fPacketsReceived++;
		fWindowPackets++;
		return;
	} else { // Else it's time to update the plots
		fStartTime_ms = time_ms;
		updatePlots();
		drawPlots();
		drawLabels();
		fPacketsReceived++;
		fWindowPackets = 0;
	}
}

void DAQoniteGUI::addPom(unsigned int pomID, unsigned int pomIndex) {
	std::cout << "DAQonite - Adding new POM to monitoring, with ID -> " << pomID << std::endl;
	fModifyPlots = true;
	fActivePOMs.push_back(pomID);

	// Add a clean channel vector into the rate array
	std::vector<unsigned int> channelVec(PMTSPERPOM);
	fRateArray.push_back(channelVec);
	clearPomRates(pomIndex);

	// Add the individual channel plots
	for (int i=0; i<PMTSPERPOM; i++) {
		fChannelRatePlots.push_back(makeTotalRatePlot(pomID, i));
	}
}

void DAQoniteGUI::clearPomRates(unsigned int pomIndex) {
	for(int channel = 0; channel<PMTSPERPOM; channel++) {
		fRateArray[pomIndex][channel] = 0;
	}
}

void DAQoniteGUI::updatePlots() {
	fNumUpdates++;
	if (fModifyPlots) { modifyPlots(); }
	if ((fNumUpdates % PLOTLENGTH) == 0) { refreshPlots(); }

	// We need to loop through the fRateArray and update the plots and then clear it...
	int totalHits = 0;
	int activeChannels = 0;
	int oddChannels = 0;
	for (int pomIndex = 0; pomIndex < (int)fActivePOMs.size(); pomIndex++) {
		for (int channelIndex = 0; channelIndex < PMTSPERPOM; channelIndex++) {
			int hits = (int)fRateArray[pomIndex][channelIndex];

			// add to the total hits for this window
			totalHits += hits;

			// if there are hits the channel is active
			if ( hits > 0 ) { activeChannels++; }

			// If more than HIGHRATE hits its odd
			if ( hits > HIGHRATE) { oddChannels++; }

			// Set the individual channel rate plot
			int plotVectorIndex = (pomIndex*PMTSPERPOM) + (channelIndex);
			fChannelRatePlots[plotVectorIndex]->SetBinContent(fNumUpdates-(fNumRefresh*PLOTLENGTH), hits/((float)UPDATERATE/1000));

			// Set the heat map bin for this channel
			fRateHeatMapPlot->SetBinContent(channelIndex+1, pomIndex+1, hits/((float)UPDATERATE/1000));
		}
		clearPomRates(pomIndex);
	}

	// Set the channel monitoring variables from our scan
	fActiveChannels = activeChannels;
	fOddChannels = oddChannels;

	// Set the total channel rate plot
	float hitRate = (float)totalHits / ((float)UPDATERATE/1000);
	fTotalRatePlot->SetBinContent(fNumUpdates-(fNumRefresh*PLOTLENGTH), hitRate);

	// Set the total packet rate plot
	float packetRate = fWindowPackets / ((float)UPDATERATE/1000);
	fPacketRatePlot->SetBinContent(fNumUpdates-(fNumRefresh*PLOTLENGTH), packetRate);
}

void DAQoniteGUI::modifyPlots() {
	std::cout << "DAQonite - Modifying Plots" << std::endl;
	delete fRateHeatMapPlot;
	fRateHeatMapPlot = NULL;
	fRateHeatMapPlot = new TH2F("RateHeatMapPlot", "RateHeatMapPlot", PMTSPERPOM, -0.5, PMTSPERPOM - 0.5, (int)fActivePOMs.size(),
								 -0.5, (int)fActivePOMs.size() - 0.5);
	fRateHeatMapPlot->GetZaxis()->SetRangeUser(0, 12000);
	fRateHeatMapPlot->GetXaxis()->SetTitle("Channel");
	fRateHeatMapPlot->GetYaxis()->SetTitle("POM");
	fRateHeatMapPlot->GetYaxis()->CenterTitle();
	fRateHeatMapPlot->GetYaxis()->SetTitleSize(0.14);
	fRateHeatMapPlot->GetYaxis()->SetTitleOffset(0.3);
	fRateHeatMapPlot->GetYaxis()->SetLabelSize(0.08);
	fRateHeatMapPlot->SetStats(0);
	fModifyPlots = false;
}

void DAQoniteGUI::refreshPlots() {
	std::cout << "DAQonite - Refresh Plots" << std::endl;
	// Clear the individual channel plots and make new clean ones
	for(int pom = 0; pom < (int)fActivePOMs.size(); pom++) {
		for (int channel=0; channel<PMTSPERPOM; channel++) {
			// Set the individual channel rate plot
			int plotVectorIndex = (pom*PMTSPERPOM) + (channel);
			delete fChannelRatePlots[plotVectorIndex];
			fChannelRatePlots[plotVectorIndex] = NULL;
			fChannelRatePlots[plotVectorIndex] = makeTotalRatePlot(pom, channel);
		}
	}

	// Clean the individual plots and create new ones
	delete fTotalRatePlot;
	fTotalRatePlot = NULL;
	fTotalRatePlot = makeTotalRatePlot();

	delete fPacketRatePlot;
	fPacketRatePlot = NULL;
	fPacketRatePlot = makePacketRatePlot();

	fNumRefresh++;
}

void DAQoniteGUI::drawPlots() {
	// Canvas 1
	fCanvas1->cd();
	if (fSpecifyButton->IsDown()) {
		int plotVectorIndex = ((int)fPomIDEntry->GetNumber()*PMTSPERPOM) + ((int)fChannelEntry->GetNumber());
		fChannelRatePlots[plotVectorIndex]->Draw();
	} else {
		fTotalRatePlot->Draw();
	}
	fCanvas1->Update();

	// Canvas 2
	fCanvas2->cd();
	fPacketRatePlot->Draw();
	fCanvas2->Update();

	// Canvas 3
	fCanvas3->cd();
	fRateHeatMapPlot->Draw("COLZ");
	fCanvas3->Update();
}

void DAQoniteGUI::drawLabels() {
	// Run Status Label
	TString statusLabelText = "Status: ";
	if (fRunning) { 
		statusLabelText += "Mining";
		fRunStatusLabel->SetBackgroundColor(TColor::Number2Pixel(8));
	} else { 
		statusLabelText += "In The Lunch Room";
		fRunStatusLabel->SetBackgroundColor(TColor::Number2Pixel(46));
	}
	fRunStatusLabel->SetText(statusLabelText);

	// Run Type Label
	TString typeLabelText = "Ore: "; 
	typeLabelText += fRunType;
	fRunTypeLabel->SetText(typeLabelText);
	if (fRunning) { fRunTypeLabel->SetBackgroundColor(TColor::Number2Pixel(8)); }
	else { fRunTypeLabel->SetBackgroundColor(TColor::Number2Pixel(46)); }

	// Run Num Label
	TString numLabelText = "Dig: "; 
	numLabelText += fRunNumber;
	fRunNumLabel->SetText(numLabelText);	
	if (fRunning) { fRunNumLabel->SetBackgroundColor(TColor::Number2Pixel(8)); }
	else { fRunNumLabel->SetBackgroundColor(TColor::Number2Pixel(46)); }

	// Run Time Label
	TString timeLabelText = "Time [s]: "; 
	timeLabelText += (float)(fStartTime_ms - fStartTime)/1000; 
	fRunTimeLabel->SetText(timeLabelText);
	if (fRunning) { fRunTimeLabel->SetBackgroundColor(TColor::Number2Pixel(8)); }
	else { fRunTimeLabel->SetBackgroundColor(TColor::Number2Pixel(46)); }

	// Run Packets Label
	TString packetsLabelText = "Loads: ";
	packetsLabelText += fPacketsReceived; 
	fRunPacketsLabel->SetText(packetsLabelText);
	if (fRunning) { fRunPacketsLabel->SetBackgroundColor(TColor::Number2Pixel(8)); }
	else { fRunPacketsLabel->SetBackgroundColor(TColor::Number2Pixel(46)); }

	// Run File Label
	TString fileLabelText = "Container: ";
	if (fRunFile != "") {
		fileLabelText += fRunFile; 
		fRunFileLabel->SetBackgroundColor(TColor::Number2Pixel(8));	
	} else { 
		fileLabelText += "Not Filling a Container"; 
		fRunFileLabel->SetBackgroundColor(TColor::Number2Pixel(46));
	}
	fRunFileLabel->SetText(fileLabelText);

	// Fact Label 1
	TString fact1Labeltext;
	if (fPageNum == 0) {
		fact1Labeltext = "Active Channels: ";
		fact1Labeltext += fActiveChannels;
	} else {
		fact1Labeltext = "Not yet implemented";
	}
	fFactLabel1->SetText(fact1Labeltext);

	// Fact Label 2
	TString fact2Labeltext;
	if (fPageNum == 0) {
		fact2Labeltext = "Odd Channels: ";
		fact2Labeltext += fOddChannels;
	} else {
		fact2Labeltext = "Not yet implemented";
	}
	fFactLabel2->SetText(fact2Labeltext);

	// Fact Label 3
	TString fact3Labeltext;
	fact3Labeltext = "Not yet implemented";
	fFactLabel3->SetText(fact3Labeltext);

	// Fact Label 4
	TString fact4Labeltext;
	fact4Labeltext = "Not yet implemented";
	fFactLabel4->SetText(fact4Labeltext);
}

TH1F* DAQoniteGUI::makeTotalRatePlot(unsigned int pomID, unsigned int channel) {
	TString plotName = "TotalRatePlot_";
	plotName += pomID;
	plotName += "_";
	plotName += channel;
	TH1F* totalRatePlot = new TH1F(plotName, plotName, PLOTLENGTH, 0, PLOTLENGTH);
	totalRatePlot->GetXaxis()->SetTitle("cycleCounter");
	totalRatePlot->GetYaxis()->SetTitle("Total Hit Rate");
	totalRatePlot->GetYaxis()->CenterTitle();
	totalRatePlot->GetYaxis()->SetTitleSize(0.14);
	totalRatePlot->GetYaxis()->SetTitleOffset(0.3);
	totalRatePlot->GetYaxis()->SetLabelSize(0.08);
	totalRatePlot->SetFillColor(49);
	totalRatePlot->SetLineColor(kBlack);
	totalRatePlot->SetLineWidth(2);
	totalRatePlot->SetStats(0);
	return totalRatePlot;
}

TH1F* DAQoniteGUI::makeTotalRatePlot() {
	TH1F* totalRatePlot = new TH1F("TotalRatePlot", "TotalRatePlot", PLOTLENGTH, 0, PLOTLENGTH);
	totalRatePlot->GetXaxis()->SetTitle("cycleCounter");
	totalRatePlot->GetYaxis()->SetTitle("Total Hit Rate");
	totalRatePlot->GetYaxis()->CenterTitle();
	totalRatePlot->GetYaxis()->SetTitleSize(0.14);
	totalRatePlot->GetYaxis()->SetTitleOffset(0.3);
	totalRatePlot->GetYaxis()->SetLabelSize(0.08);
	totalRatePlot->SetFillColor(9);
	totalRatePlot->SetLineColor(kBlack);
	totalRatePlot->SetLineWidth(2);
	totalRatePlot->SetStats(0);
	return totalRatePlot;
}

TH1F* DAQoniteGUI::makePacketRatePlot() {
	TH1F* packetRatePlot = new TH1F("PacketRatePlot", "PacketRatePlot", PLOTLENGTH, 0, PLOTLENGTH);
	packetRatePlot->GetXaxis()->SetTitle("cycleCounter");
	packetRatePlot->GetYaxis()->SetTitle("Packet Rate");
	packetRatePlot->GetYaxis()->CenterTitle();
	packetRatePlot->GetYaxis()->SetTitleSize(0.14);
	packetRatePlot->GetYaxis()->SetTitleOffset(0.3);
	packetRatePlot->GetYaxis()->SetLabelSize(0.08);
	packetRatePlot->SetFillColor(9);
	packetRatePlot->SetLineColor(kBlack);
	packetRatePlot->SetLineWidth(2);
	packetRatePlot->SetStats(0);
	return packetRatePlot;
}

TH2F* DAQoniteGUI::makeHeatMapPlot() {
	TH2F* rateHeatMapPlot = new TH2F("RateHeatMapPlot", "RateHeatMapPlot", PMTSPERPOM, -0.5, PMTSPERPOM - 0.5, 2, -0.5, 1.5);
	rateHeatMapPlot->GetZaxis()->SetRangeUser(0, 12000);
	rateHeatMapPlot->GetXaxis()->SetTitle("Channel");
	rateHeatMapPlot->GetYaxis()->SetTitle("POM");
	rateHeatMapPlot->GetYaxis()->CenterTitle();
	rateHeatMapPlot->GetYaxis()->SetTitleSize(0.14);
	rateHeatMapPlot->GetYaxis()->SetTitleOffset(0.3);
	rateHeatMapPlot->GetYaxis()->SetLabelSize(0.08);
	rateHeatMapPlot->SetStats(0);
	return rateHeatMapPlot;
}

void DAQoniteGUI::toggleSpecific() {
	std::cout << "Toggle Specific..." << std::endl;
	if (fNumUpdates >= 1) { drawPlots(); }
}

void DAQoniteGUI::pageBackward() {
	std::cout << "Page Backward..." << std::endl;
}

void DAQoniteGUI::pageForward() {
	std::cout << "Page Forward..." << std::endl;
}

void DAQoniteGUI::startRun(unsigned int type, unsigned int run, TString fileName) {
	fRunning = true;
	fRunType = type;
	fRunNumber = run;
	fRunFile = fileName;
	drawLabels();
}

void DAQoniteGUI::stopRun() {
	fRunning = false;
	fRunType = 0;
	fRunNumber = 0;
	fStartTime_ms = 0;
	fStartTime = 0;
	fPacketsReceived = 0;
	fRunFile = "";
	drawLabels();
}
