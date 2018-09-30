/*
 * monitoring_plots.cc
 * Keeps track of monitoring data and updates the plots on the screen
 *
 *  Created on: Sep 27, 2018
 *      Author: Josh Tingey
 *       Email: j.tingey.16@ucl.ac.uk
 */

#include "monitoring_plots.h"

Monitoring_plots::Monitoring_plots() {
	// Create a main frame
	TGMainFrame *mainFrame = new TGMainFrame(gClient->GetRoot(),600,600);

	// Channel Rate Frame /////////////////
	TGHorizontalFrame * channelRateFrame = new TGHorizontalFrame(mainFrame, 600, 300);

	TRootEmbeddedCanvas *eChannelRateCanvas = new TRootEmbeddedCanvas("eChannelRateCanvas", channelRateFrame, 600, 300);
	channelRateFrame->AddFrame(eChannelRateCanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 1));
	fChannelRateCanvas = eChannelRateCanvas->GetCanvas();

	mainFrame->AddFrame(channelRateFrame, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
	///////////////////////////////////////

	// Total Rate Frame /////////////////
	TGHorizontalFrame * totalRateFrame = new TGHorizontalFrame(mainFrame, 600, 300);

	TRootEmbeddedCanvas *eTotalRateCanvas = new TRootEmbeddedCanvas("eTotalRateCanvas", totalRateFrame, 600, 300);
	totalRateFrame->AddFrame(eTotalRateCanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 1));
	fTotalRateCanvas = eTotalRateCanvas->GetCanvas();

	mainFrame->AddFrame(totalRateFrame, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
	///////////////////////////////////////

	/*
	/////////  Saving Setting Frame   /////////
	TGHorizontalFrame * textFrame = new TGHorizontalFrame(mainFrame, 600, 200);

	// Save file name
	TGLabel *cycleLabel = new TGLabel(textFrame, "hello!");
	textFrame->AddFrame(cycleLabel, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 1));

	TGCheckButton *SaveDataButton = new TGCheckButton(textFrame, "Save Data");
	SaveDataButton->SetState(kButtonUp);
	textFrame->AddFrame(SaveDataButton, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 1));

	mainFrame->AddFrame(textFrame, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
	///////////////////////////////////////////
	*/

	mainFrame->SetWindowName("DAQoniteGUI - by Josh Tingey MSci, JoshTingeyDAQDemon.Josh");
	mainFrame->MapSubwindows();
	mainFrame->Resize(mainFrame->GetDefaultSize());
	mainFrame->MapWindow();

	fChannelRateCanvas->cd();
	fChannelRatePlot = new TH2F("ChannelRatePlot", "ChannelRatePlot", 30, -0.5, 29.5, 2, -0.5, 1.5);
	fChannelRatePlot->GetZaxis()->SetRangeUser(0, 12000);
	fChannelRatePlot->GetXaxis()->SetTitle("Channel");
	fChannelRatePlot->GetYaxis()->SetTitle("POM");
	fChannelRatePlot->SetStats(0);
	fChannelRatePlot->Draw();
	fChannelRateCanvas->Update();

	fTotalRateCanvas->cd();
	fTotalRatePlot = new TH1F("TotalRatePlot", "TotalRatePlot", 100, 0, 100);
	fTotalRatePlot->GetXaxis()->SetTitle("cycleCounter");
	fTotalRatePlot->GetYaxis()->SetTitle("Total Hit Rate");
	fTotalRatePlot->SetStats(0);
	fTotalRatePlot->Draw();
	fTotalRateCanvas->Update();

	windowsPackets = 0;
	startPomID = 0;
	startTime_ms = 0;
	cycleCounter = 1;
	fActivePOMs.clear();
	fRateArray.clear();
}

Monitoring_plots::~Monitoring_plots() {
	fActivePOMs.clear();
	fRateArray.clear();

	delete fChannelRatePlot;
	delete fTotalRatePlot;
}

void Monitoring_plots::addHits(unsigned int pomID, unsigned int channel, unsigned int hits) {

	int pomIndex = 0;
	for(pomIndex = 0; pomIndex < (int)fActivePOMs.size(); pomIndex++) {
	    if (fActivePOMs[pomIndex] == pomID) {
	    	fRateArray[pomIndex][channel] += hits;
	    	return;
	    }
	}

	std::cout << "DAQonite - Adding new POM to monitoring, with ID -> " << pomID << std::endl;
	fActivePOMs.push_back(pomID);
	std::vector<unsigned int> channelVec(30);
	fRateArray.push_back(channelVec);
	clearPOMRates(pomIndex);
}

void Monitoring_plots::clearPOMRates(unsigned int pomIndex) {
	for(int channel = 0; channel<30; channel++) {
		fRateArray[pomIndex][channel] = 0;
	}
}

void Monitoring_plots::updatePlots() {
	// First lets update the total hit rate, this is an average hit rate across all channels
	fChannelRateCanvas->cd();
	float totalRate = 0.0;
	for (int i=0; i<(int)fActivePOMs.size(); i++) {
		for (int j=0; j<30; j++) {
			int hits = (int)fRateArray[i][j];
			totalRate += hits;
			fChannelRatePlot->SetBinContent(j+1, i+1, hits/(PLOTRATE/1000));
		}
		clearPOMRates(i);
	}

	fChannelRatePlot->Draw("COLZ");
	fChannelRateCanvas->Update();

	totalRate = totalRate / (PLOTRATE/1000); // We want it in seconds
	fTotalRateCanvas->cd();
	fTotalRatePlot->SetBinContent(cycleCounter, totalRate);
	fTotalRatePlot->Draw();
	fTotalRateCanvas->Update();
}

void Monitoring_plots::addHeader(UInt_t pomID, UInt_t time_ms) {
	if (cycleCounter == 1) {
		startTime_ms = time_ms;
		startPomID = pomID;
		cycleCounter += 1;
	} else if (pomID != startPomID) {
		windowsPackets += 1;
		return;
	} else if ((pomID == startPomID) && ((time_ms - startTime_ms) < PLOTRATE)) {
		windowsPackets += 1;
		return;
	} else {
		startTime_ms = time_ms;
		updatePlots();
		windowsPackets = 0;
		cycleCounter += 1;
	}
}
