/*
 * daqonite_gui.cc
 *
 *  Created on: Sep 24, 2018
 *      Author: chips
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
	gClient->GetColorByName("#999999",ucolor);

	// horizontal frame
	TGHorizontalFrame *fFrame1 = new TGHorizontalFrame(fMainFrame,500,150,kHorizontalFrame | kRaisedFrame,ucolor);
	fFrame1->SetName("fFrame1");
	fFrame1->SetLayoutBroken(kTRUE);

	// embedded canvas
	TRootEmbeddedCanvas *fECanvas1 = new TRootEmbeddedCanvas(0,fFrame1,480,130,kSunkenFrame);
	fECanvas1->SetName("fECanvas1");
	Int_t wfECanvas1 = fECanvas1->GetCanvasWindowId();
	TCanvas *c123 = new TCanvas("c123", 10, 10, wfECanvas1);
	fECanvas1->AdoptCanvas(c123);
	fFrame1->AddFrame(fECanvas1, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fECanvas1->MoveResize(10,10,480,130);

	fMainFrame->AddFrame(fFrame1, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fFrame1->MoveResize(10,10,500,150);


	// horizontal frame
	TGHorizontalFrame *fFrame2 = new TGHorizontalFrame(fMainFrame,500,150,kHorizontalFrame | kRaisedFrame,ucolor);
	fFrame2->SetName("fFrame2");
	fFrame2->SetLayoutBroken(kTRUE);

	// embedded canvas
	TRootEmbeddedCanvas *fECanvas2 = new TRootEmbeddedCanvas(0,fFrame2,480,130,kSunkenFrame);
	fECanvas2->SetName("fECanvas2");
	Int_t wfECanvas2 = fECanvas2->GetCanvasWindowId();
	TCanvas *c124 = new TCanvas("c124", 10, 10, wfECanvas2);
	fECanvas2->AdoptCanvas(c124);
	fFrame2->AddFrame(fECanvas2, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fECanvas2->MoveResize(10,10,480,130);

	fMainFrame->AddFrame(fFrame2, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fFrame2->MoveResize(10,170,500,150);


	// horizontal frame
	TGHorizontalFrame *fFrame3 = new TGHorizontalFrame(fMainFrame,500,50,kHorizontalFrame,ucolor);
	fFrame3->SetName("fFrame3");
	fFrame3->SetLayoutBroken(kTRUE);
	fPomIDEntry = new TGNumberEntry(fFrame3, (Double_t) 0,2,-1,(TGNumberFormat::EStyle) 5,
									TGNumberFormat::kNEAPositive, TGNumberFormat::kNELLimitMinMax, 0, 29);
	fPomIDEntry->SetName("fPomIDEntry");
	fFrame3->AddFrame(fPomIDEntry, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fPomIDEntry->MoveResize(299,15,60,25);
	fChannelEntry = new TGNumberEntry(fFrame3, (Double_t) 0,2,-1,(TGNumberFormat::EStyle) 5,
									  TGNumberFormat::kNEAPositive, TGNumberFormat::kNELLimitMinMax, 0, 29);
	fChannelEntry->SetName("fChannelEntry");
	fFrame3->AddFrame(fChannelEntry, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fChannelEntry->MoveResize(404,15,60,25);
	fSpecifyButton = new TGCheckButton(fFrame3,"Specify Channel [PomID and Channel]");
	fSpecifyButton->Connect("Clicked()","DAQoniteGUI",this,"toggleSpecific()");
	fSpecifyButton->SetTextJustify(36);
	fSpecifyButton->SetMargins(0,0,0,0);
	fSpecifyButton->SetWrapLength(-1);
	fFrame3->AddFrame(fSpecifyButton, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fSpecifyButton->MoveResize(20,15,250,19);

	fMainFrame->AddFrame(fFrame3, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fFrame3->MoveResize(10,330,500,50);


	// horizontal frame
	TGHorizontalFrame *fFrame4 = new TGHorizontalFrame(fMainFrame,500,100,kHorizontalFrame | kRaisedFrame,ucolor);
	fFrame4->SetName("fFrame4");
	fFrame4->SetLayoutBroken(kTRUE);

	gClient->GetColorByName("#59d454",ucolor);
	fLabel1 = new TGLabel(fFrame4,"---",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kRaisedFrame,ucolor);
	fLabel1->SetTextJustify(36);
	fLabel1->SetMargins(0,0,0,0);
	fLabel1->SetWrapLength(-1);
	fFrame4->AddFrame(fLabel1, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fLabel1->MoveResize(10,10,150,35);

	fLabel4 = new TGLabel(fFrame4,"---",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kRaisedFrame,ucolor);
	fLabel4->SetTextJustify(36);
	fLabel4->SetMargins(0,0,0,0);
	fLabel4->SetWrapLength(-1);
	fFrame4->AddFrame(fLabel4, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fLabel4->MoveResize(10,55,150,35);

	fLabel2 = new TGLabel(fFrame4,"---",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kRaisedFrame,ucolor);
	fLabel2->SetTextJustify(36);
	fLabel2->SetMargins(0,0,0,0);
	fLabel2->SetWrapLength(-1);
	fFrame4->AddFrame(fLabel2, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fLabel2->MoveResize(175,10,150,35);

	fLabel5 = new TGLabel(fFrame4,"---",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kRaisedFrame,ucolor);
	fLabel5->SetTextJustify(36);
	fLabel5->SetMargins(0,0,0,0);
	fLabel5->SetWrapLength(-1);
	fFrame4->AddFrame(fLabel5, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fLabel5->MoveResize(175,55,150,35);

	fLabel3 = new TGLabel(fFrame4,"---",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kRaisedFrame,ucolor);
	fLabel3->SetTextJustify(36);
	fLabel3->SetMargins(0,0,0,0);
	fLabel3->SetWrapLength(-1);
	fFrame4->AddFrame(fLabel3, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fLabel3->MoveResize(340,10,150,35);

	fLabel6 = new TGLabel(fFrame4,"---",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kRaisedFrame,ucolor);
	fLabel6->SetTextJustify(36);
	fLabel6->SetMargins(0,0,0,0);
	fLabel6->SetWrapLength(-1);
	fFrame4->AddFrame(fLabel6, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fLabel6->MoveResize(340,55,150,35);

	fMainFrame->AddFrame(fFrame4, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fFrame4->MoveResize(10,390,500,100);

	gClient->GetColorByName("#999999",ucolor);

	// horizontal frame
	TGHorizontalFrame *fFrame5 = new TGHorizontalFrame(fMainFrame,500,200,kHorizontalFrame,ucolor);
	fFrame5->SetName("fFrame5");
	fFrame5->SetLayoutBroken(kTRUE);

	// embedded canvas
	TRootEmbeddedCanvas *fECanvas3 = new TRootEmbeddedCanvas(0,fFrame5,480,180,kSunkenFrame);
	fECanvas3->SetName("fECanvas3");
	Int_t wfECanvas3 = fECanvas3->GetCanvasWindowId();
	TCanvas *c125 = new TCanvas("c125", 10, 10, wfECanvas3);
	fECanvas3->AdoptCanvas(c125);
	fFrame5->AddFrame(fECanvas3, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fECanvas3->MoveResize(10,10,480,180);

	fMainFrame->AddFrame(fFrame5, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fFrame5->MoveResize(10,500,500,200);

	fMainFrame->SetMWMHints(kMWMDecorAll,
						kMWMFuncAll,
						kMWMInputModeless);

	fMainFrame->MapSubwindows();
	fMainFrame->Resize(fMainFrame->GetDefaultSize());
	fMainFrame->MapWindow();
	fMainFrame->Resize(523,708);

	// Set up the other variables...
	fCanvas1 = fECanvas1->GetCanvas();
	fCanvas2 = fECanvas2->GetCanvas();
	fCanvas3 = fECanvas3->GetCanvas();

	fCanvas1->cd();
	fTotalRatePlot = makeTotalRatePlot();
	fTotalRatePlot->Draw();
	fCanvas1->Update();

	fCanvas2->cd();
	fPacketRatePlot = makePacketRatePlot();
	fPacketRatePlot->Draw();
	fCanvas2->Update();

	fCanvas3->cd();
	fRateHeatMapPlot = makeHeatMapPlot();
	fRateHeatMapPlot->Draw();
	fCanvas3->Update();

	fPacketsReceived = 0;
	fNumUpdates = 0;
	fNumRefresh = 0;
	fModifyPlots = false;
	fWindowPackets = 0;
	fStartPomID = 0;
	fStartTime_ms = 0;

	fActivePOMs.clear();
	fRateArray.clear();

	fRunNumber = 1;
	fStartTime = 0;
	fRunType = 1;
	fActiveChannels = 0;
	fOddChannels = 0;
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
		updateLabels();
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

void DAQoniteGUI::updateLabels() {
	// Run Labels
	TString label1 = "Run Number: "; label1 += fRunNumber; fLabel1->SetText(label1);
	TString label2 = "Run Type: "; label2 += fRunType; fLabel2->SetText(label2);
	TString label3 = "Run Time [s]: "; label3 += (float)(fStartTime_ms - fStartTime)/1000; fLabel3->SetText(label3);

	// Active Channels Label
	TString label4 = "Active Channels: "; label4 += fActiveChannels; fLabel4->SetText(label4);
	if ((fActiveChannels % PMTSPERPOM) != 0) {
		fLabel4->SetBackgroundColor(TColor::Number2Pixel(kRed));
	} else {
		fLabel4->SetBackgroundColor(TColor::Number2Pixel(8));
	}

	// Off Channels label
	TString label5 = "Odd Channels: "; label5 += fOddChannels; fLabel5->SetText(label5);
	if (((float)fOddChannels / (float)fActiveChannels) > 0.2) {
		fLabel5->SetBackgroundColor(TColor::Number2Pixel(kRed));
	} else if (((float)fOddChannels / (float)fActiveChannels) > 0.1) {
		fLabel5->SetBackgroundColor(TColor::Number2Pixel(kOrange));
	} else {
		fLabel5->SetBackgroundColor(TColor::Number2Pixel(8));
	}

	TString label6 = "Packets: "; label6 += fPacketsReceived; fLabel6->SetText(label6);
	//if (((fPacketsReceived/fNumUpdates)/) != 0) { fLabel6->SetBackgroundColor(TColor::Number2Pixel(kRed)); }
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
	std::cout << "DAQonite - Show Specific Channel Toggle" << std::endl;
	if (fNumUpdates >= 1) { drawPlots(); }
}