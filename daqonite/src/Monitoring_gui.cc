/**
 * Monitoring_gui - The ROOT monitoring GUI for DAQonite
 */

#include "Monitoring_gui.h"

#define PLOTLENGTH 100
#define PMTSPERPOM 30
#define HIGHRATE 10000

Monitoring_gui::Monitoring_gui(int updateRate, std::string configFile) : 
							   fUpdateRate(updateRate), fConfigFile(configFile) {

	//////////////////////////////////////
	//			GUI SETUP START			//
	//////////////////////////////////////

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
	fSpecifyButton->Connect("Clicked()","Monitoring_gui",this,"toggleSpecific()");
	fSpecifyButton->SetTextJustify(36);
	fSpecifyButton->SetMargins(0,0,0,0);
	fSpecifyButton->SetWrapLength(-1);
	fFrame3->AddFrame(fSpecifyButton, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fSpecifyButton->MoveResize(410,66,180,22);

	fBackButton = new TGTextButton(fFrame3,"<--- Packets",-1,TGTextButton::GetDefaultGC()(),TGTextButton::GetDefaultFontStruct(),kRaisedFrame);
	fBackButton->Connect("Clicked()","Monitoring_gui",this,"pageBackward()");
	fBackButton->SetTextJustify(36);
	fBackButton->SetMargins(0,0,0,0);
	fBackButton->SetWrapLength(-1);
	fBackButton->Resize(190,45);
	fBackButton->ChangeBackground(ucolor);
	fFrame3->AddFrame(fBackButton, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fBackButton->MoveResize(405,5,190,45);

	fForwardButton = new TGTextButton(fFrame3,"Environment --->",-1,TGTextButton::GetDefaultGC()(),TGTextButton::GetDefaultFontStruct(),kRaisedFrame);
	fForwardButton->Connect("Clicked()","Monitoring_gui",this,"pageForward()");
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

	//////////////////////////////////////
	//			GUI SETUP END			//
	//////////////////////////////////////

	// Set the configuration variables using the Monitoring_config class
	Monitoring_config* fConfig = new Monitoring_config(fConfigFile.c_str());
	fNumCLBs = fConfig->getNumCLBs();
	fCLBeIDs = fConfig->getCLBeIDs();
	fCLBTypes = fConfig->getCLBTypes();
	fTotalNumChannels = fConfig->getTotalNumChannels();
	fActiveChannelsArr = fConfig->getActiveChannels();
	delete fConfig;

	// Important variables
	fPageNum = 0;
	fMode = false;

	// Current run variables
	fRunNumber = -1;
	fRunType = -1;
	fRunTotalPackets = 0;
	fRunTotalDropped = 0;
	fRunFile = "";
	fRunUpdates = 0;

	// Window variables
	fUpdatePackets = 0;

	// Monitoring variables
	fPacketsReceived = 0;
	fNumUpdates = 0;
	fNumRefresh = 0;
	fActiveCLBs = 0;
	fActiveChannels = 0;
	fOddChannels = 0;
	fNonConfigData = false;

	setupArrays();
	setupPlots();
	draw();
}

Monitoring_gui::~Monitoring_gui() {
	fMainFrame->Cleanup();
}

void Monitoring_gui::addOpticalPacket(unsigned int pomID, unsigned int seqNumber) {
	// Search for this pomID in the config CLB IDs
	std::vector<unsigned int>::iterator it = std::find(fCLBeIDs.begin(), fCLBeIDs.end(), pomID);

	// If this is a config CLB and we are in running mode we do packet checking
	if (it != fCLBeIDs.end() && fMode) {
		int clbIndex = std::distance(fCLBeIDs.begin(), it);

		// Check to see if we have missed a packet
		if (fOptical_packets[clbIndex] != 0) {
			int seqDiff = seqNumber - fOptical_seq[clbIndex];

			if (seqDiff == 1) {} 
			else if (seqDiff > 1) { 
				fOptical_dropped[clbIndex] += seqDiff; 
				fRunTotalDropped += seqDiff;
			} else { 
				//std::cout << "DAQonite - Error: Seq: " << seqNumber << "," << fMonitoring_seq[clbIndex] << std::endl;  
			}
		}

		// Set the current sequence number for this CLB
		fOptical_seq[clbIndex] = seqNumber;

		// Increment the optical packet counter
		fOptical_packets[clbIndex] ++;	

		// Increment the total run packet counter
		fRunTotalPackets ++;

		// Increment the packet counter 
		fPacketsReceived ++;

	} else if (it == fCLBeIDs.end()) {
		fNonConfigData = true;
	}
}

void Monitoring_gui::addMonitoringPacket(unsigned int pomID, unsigned int timeStamp_ms, 
										 unsigned int hits[30], float temp, float humidity,
										 unsigned int seqNumber) {
	// Search for this pomID in the config CLB IDs
	std::vector<unsigned int>::iterator it = std::find(fCLBeIDs.begin(), fCLBeIDs.end(), pomID);

	if (it != fCLBeIDs.end()) {
		int clbIndex = std::distance(fCLBeIDs.begin(), it);

		// If we are in running mode we do packet checking
		if (fMode) {
			// Check to see if we have missed a packet
			if (fMonitoring_packets[clbIndex] != 0) {
				int seqDiff = seqNumber - fMonitoring_seq[clbIndex];

				if (seqDiff == 1) {} 
				else if (seqDiff > 1) { 
					fMonitoring_dropped[clbIndex] += seqDiff; 
					fRunTotalDropped += seqDiff;
				} else { 
					//std::cout << "DAQonite - Error: Seq: " << seqNumber << "," << fMonitoring_seq[clbIndex] << std::endl;  
				}
			}

			// Set the current sequence number for this CLB
			fMonitoring_seq[clbIndex] = seqNumber;

			// Increment the monitoring packet counter
			fMonitoring_packets[clbIndex] ++;	

			// Increment the total run packet counter
			fRunTotalPackets ++;		
		}

		// This is a valid CLB. Therefore, add the hits...
		for (int channel = 0; channel < 30; channel ++) fRateArray[clbIndex][channel] += hits[channel];

		// Update the temperature and humidity for this CLB
		fTempArray[clbIndex] = temp;
		fHumidityArray[clbIndex] = humidity;

		// Increment the number of packets received in this update window
		fUpdatePackets++;

		// Increment the packet counter 
		fPacketsReceived ++;

	} else {
		fNonConfigData = true;
	}
}

void Monitoring_gui::setupArrays() {
	fRateArray.clear();

	// Add a clean channel vector into the rate array for each CLB
	for (int clbNum = 0; clbNum<fNumCLBs; clbNum++) {
		std::vector<unsigned int> channelVec(PMTSPERPOM);
		fRateArray.push_back(channelVec);
		clearPomRates(clbNum);

		fTempArray.push_back(0.0);
		fHumidityArray.push_back(0.0);

		// Packet variables
		fOptical_packets.push_back(0);
		fOptical_seq.push_back(0);
		fOptical_dropped.push_back(0);

		fMonitoring_packets.push_back(0);
		fMonitoring_seq.push_back(0);
		fMonitoring_dropped.push_back(0);
	}
}

void Monitoring_gui::setupPlots() {

	// Make the individual total plots
	fTotalRatePlot = makeTotalRatePlot();
	fPacketRatePlot = makePacketRatePlot();
	fRateHeatMapPlot = makeHeatMapPlot();
	fAvgTempPlot = makeTemperaturePlot();
	fAvgHumidityPlot = makeHumidityPlot();
	fTotalPacketPlot = makeRunPacketPlot();
	fTotalDroppedPlot = makeRunDroppedPlot();

	for (int clbNum = 0; clbNum<fNumCLBs; clbNum++) {

		// Make the CLB specific temp and humidity plots
		fCLBTempPlots.push_back(makeTemperaturePlot(fCLBeIDs[clbNum]));
		fCLBHumidityPlots.push_back(makeHumidityPlot(fCLBeIDs[clbNum]));

		// Make the CLB specific packet plots
		fCLBPacketPlots.push_back(makeRunPacketPlot(fCLBeIDs[clbNum]));
		fCLBDroppedPlots.push_back(makeRunDroppedPlot(fCLBeIDs[clbNum]));

		// Make the channel specific rate plots
		for (int channel=0; channel<PMTSPERPOM; channel++) {
			fChannelRatePlots.push_back(makeTotalRatePlot(fCLBeIDs[clbNum], channel));
		}
	}
}

void Monitoring_gui::clearPomRates(unsigned int pomIndex) {
	for(int channel = 0; channel<PMTSPERPOM; channel++) fRateArray[pomIndex][channel] = 0;
}

void Monitoring_gui::updatePlots() {
	fNumUpdates++;

	if (fMode) { fRunUpdates ++; }

	// Do we need to clear the plots that have reached PLOTLENGTH and start again?
	if ((fNumUpdates % PLOTLENGTH) == 0) { refreshPlots(); }

	// Need variables to track things...
	fActiveCLBs 				= 0;
	fActiveChannels				= 0;
	fOddChannels 				= 0;

	int 	totalHits 			= 0;
	bool 	clbHits[fNumCLBs];
	float 	sumTemp 			= 0.0;
	float 	sumHumidity 		= 0.0;

	// We need to loop through the fRateArray and update the plots and then clear it...
	for (int pomIndex = 0; pomIndex < fNumCLBs; pomIndex++) {
		clbHits[pomIndex] = false;

		// Update the CLB specific temp plots
		float clbTemp = fTempArray[pomIndex];
		sumTemp += clbTemp;
		fCLBTempPlots[pomIndex]->SetBinContent(fNumUpdates-(fNumRefresh*PLOTLENGTH), clbTemp);

		// Update the CLB specific humidity plots
		float clbHumidity = fHumidityArray[pomIndex];
		sumHumidity += clbHumidity;
		fCLBHumidityPlots[pomIndex]->SetBinContent(fNumUpdates-(fNumRefresh*PLOTLENGTH), clbHumidity);

		// Update the CLB specific packet and dropped plots
		int clbTotalPackets = fOptical_packets[pomIndex] + fMonitoring_packets[pomIndex];
		fCLBPacketPlots[pomIndex]->SetBinContent(fNumUpdates-(fNumRefresh*PLOTLENGTH), clbTotalPackets);
		int clbDroppedPackets = fOptical_dropped[pomIndex] + fMonitoring_dropped[pomIndex];
		fCLBDroppedPlots[pomIndex]->SetBinContent(fNumUpdates-(fNumRefresh*PLOTLENGTH), clbDroppedPackets);

		for (int channelIndex = 0; channelIndex < PMTSPERPOM; channelIndex++) {
			int hits = (int)fRateArray[pomIndex][channelIndex];

			// Add to the total hits for this window
			totalHits += hits;

			// If there are hits the channel is active
			if ( hits > 0 ) { 
				clbHits[pomIndex] = true;
				fActiveChannels++; 
			}

			// If more than HIGHRATE hits its odd
			if ( (hits/((float)fUpdateRate/1000)) > HIGHRATE) { fOddChannels++; }

			// Set the individual channel rate plot
			int plotVectorIndex = (pomIndex*PMTSPERPOM) + (channelIndex);
			fChannelRatePlots[plotVectorIndex]->SetBinContent(fNumUpdates-(fNumRefresh*PLOTLENGTH), hits/((float)fUpdateRate/1000));

			// Set the heat map bin for this channel
			fRateHeatMapPlot->SetBinContent(channelIndex+1, pomIndex+1, hits/((float)fUpdateRate/1000));
		}
		clearPomRates(pomIndex);
	}

	for (int clb = 0; clb<fNumCLBs; clb++) {
		if (clbHits[clb] == true) { fActiveCLBs++; }
	}

	// Set the total channel rate plot
	fTotalRatePlot->SetBinContent(fNumUpdates-(fNumRefresh*PLOTLENGTH), (float)totalHits / ((float)fUpdateRate/1000));

	// Set the total packet rate plot
	fPacketRatePlot->SetBinContent(fNumUpdates-(fNumRefresh*PLOTLENGTH), fUpdatePackets / ((float)fUpdateRate/1000));

	// Set the temperature and humidity average plots
	fAvgTempPlot->SetBinContent(fNumUpdates-(fNumRefresh*PLOTLENGTH), sumTemp / (float)fActiveCLBs);
	fAvgHumidityPlot->SetBinContent(fNumUpdates-(fNumRefresh*PLOTLENGTH), sumHumidity / (float)fActiveCLBs);

	// Set the packet plots
	fTotalPacketPlot->SetBinContent(fNumUpdates-(fNumRefresh*PLOTLENGTH), fRunTotalPackets);
	fTotalDroppedPlot->SetBinContent(fNumUpdates-(fNumRefresh*PLOTLENGTH), fRunTotalDropped);

	fUpdatePackets = 0;
}

void Monitoring_gui::refreshPlots() {
	std::cout << "\nDAQonite - Refresh plots" << std::endl;
	fNumRefresh++;

	// Clear the temp/humidity clb plots and individual channel hit plots
	for(int clb = 0; clb < fNumCLBs; clb++) {

		delete fCLBTempPlots[clb];
		fCLBTempPlots[clb] = NULL;
		fCLBTempPlots[clb] = makeTemperaturePlot(fCLBeIDs[clb]);

		delete fCLBHumidityPlots[clb];
		fCLBHumidityPlots[clb] = NULL;
		fCLBHumidityPlots[clb] = makeHumidityPlot(fCLBeIDs[clb]);

		delete fCLBPacketPlots[clb];
		fCLBPacketPlots[clb] = NULL;
		fCLBPacketPlots[clb] = makeRunPacketPlot(fCLBeIDs[clb]);

		delete fCLBDroppedPlots[clb];
		fCLBDroppedPlots[clb] = NULL;
		fCLBDroppedPlots[clb] = makeRunDroppedPlot(fCLBeIDs[clb]);

		for (int channel=0; channel<PMTSPERPOM; channel++) {
			// Set the individual channel rate plot
			int plotVectorIndex = (clb*PMTSPERPOM) + (channel);
			delete fChannelRatePlots[plotVectorIndex];
			fChannelRatePlots[plotVectorIndex] = NULL;
			fChannelRatePlots[plotVectorIndex] = makeTotalRatePlot(clb, channel);
		}
	}

	// Clean the individual plots and create new ones 
	delete fTotalRatePlot;
	fTotalRatePlot = NULL;
	fTotalRatePlot = makeTotalRatePlot();

	delete fPacketRatePlot;
	fPacketRatePlot = NULL;
	fPacketRatePlot = makePacketRatePlot();

	delete fAvgTempPlot;
	fAvgTempPlot = NULL;
	fAvgTempPlot = makeTemperaturePlot();

	delete fAvgHumidityPlot;
	fAvgHumidityPlot = NULL;
	fAvgHumidityPlot = makeHumidityPlot();

	delete fTotalPacketPlot;
	fTotalPacketPlot = NULL;
	fTotalPacketPlot = makeRunPacketPlot();

	delete fTotalDroppedPlot;
	fTotalDroppedPlot = NULL;
	fTotalDroppedPlot = makeRunDroppedPlot();
}

void Monitoring_gui::draw() {
	drawPlots();
	drawLabels();
	drawDirectionButtons();
}

void Monitoring_gui::drawPlots() {

	// If we have not received any packets just display the CHIPS logo on all canvases

	if (fPacketsReceived == 0) { // If we have not received any packets just display the CHIPS logo on all canvases

		drawLogo(fCanvas1);
		drawLogo(fCanvas2);
		drawLogo(fCanvas3);

	} else if (!fMode) { // If we are not in running mode

		if (fPageNum == 0) {

			// Canvas 1 (Total rate plot or channel specific rate plot)
			fCanvas1->cd();
			if (fSpecifyButton->IsDown()) {
				fChannelRatePlots[((int)fPomIDEntry->GetNumber()*PMTSPERPOM) + ((int)fChannelEntry->GetNumber())]->Draw();
			} else { fTotalRatePlot->Draw(); }
			fCanvas1->Update();

			// Canvas 2 (CHIPS Logo, will be RMS plot(s))
			drawLogo(fCanvas2);

			// Canvas 3 (Rate heat map)
			fCanvas3->cd();
			fRateHeatMapPlot->Draw("COLZ");
			fCanvas3->Update();

		} else if (fPageNum == 1) {

			// Canvas 1 (Average temperature plot or CLB specific temperature plot)
			fCanvas1->cd();
			if (fSpecifyButton->IsDown()) { fCLBTempPlots[(int)fPomIDEntry->GetNumber()]->Draw(); }
			else { fAvgTempPlot->Draw(); }
			fCanvas1->Update();

			// Canvas 2 (Average humidity plot or CLB specific humidity plot)
			fCanvas2->cd();
			if (fSpecifyButton->IsDown()) { fCLBHumidityPlots[(int)fPomIDEntry->GetNumber()]->Draw(); }
			else { fAvgHumidityPlot->Draw(); }
			fCanvas2->Update();

			// Canvas 3 (CHIPS logo)
			drawLogo(fCanvas3);

		} else if (fPageNum == 2) {

			// Canvas 1 (CHIPS logo)
			drawLogo(fCanvas1);

			// Canvas 2 (CHIPS logo)
			drawLogo(fCanvas2);

			// Canvas 3 (Total packet rate plot)
			fCanvas3->cd();
			fPacketRatePlot->Draw();
			fCanvas3->Update();
			
		} else { std::cout << "DAQonite - Error: Wrong GUI page number!" << std::endl; }

	} else { // If we are in running mode

		if (fPageNum == 0) {

			// Canvas 1 (Total rate plot or channel specific rate plot)
			fCanvas1->cd();
			if (fSpecifyButton->IsDown()) {
				fChannelRatePlots[((int)fPomIDEntry->GetNumber()*PMTSPERPOM) + ((int)fChannelEntry->GetNumber())]->Draw();
			} else { fTotalRatePlot->Draw(); }
			fCanvas1->Update();

			// Canvas 2 (CHIPS Logo, will be RMS plot(s))
			drawLogo(fCanvas2);

			// Canvas 3 (Rate heat map)
			fCanvas3->cd();
			fRateHeatMapPlot->Draw("COLZ");
			fCanvas3->Update();

		} else if (fPageNum == 1) {

			// Canvas 1 (Average temperature plot or CLB specific temperature plot)
			fCanvas1->cd();
			if (fSpecifyButton->IsDown()) { fCLBTempPlots[(int)fPomIDEntry->GetNumber()]->Draw(); }
			else { fAvgTempPlot->Draw(); }
			fCanvas1->Update();

			// Canvas 2 (Average humidity plot or CLB specific humidity plot)
			fCanvas2->cd();
			if (fSpecifyButton->IsDown()) { fCLBHumidityPlots[(int)fPomIDEntry->GetNumber()]->Draw(); }
			else { fAvgHumidityPlot->Draw(); }
			fCanvas2->Update();

			// Canvas 3 (CHIPS logo)
			drawLogo(fCanvas3);

		} else if (fPageNum == 2) {

			// Canvas 1 (CHIPS logo)
			fCanvas1->cd();
			if (fSpecifyButton->IsDown()) { fCLBPacketPlots[(int)fPomIDEntry->GetNumber()]->Draw(); }
			else { fTotalPacketPlot->Draw(); }
			fCanvas1->Update();

			// Canvas 2 (CHIPS logo)
			fCanvas2->cd();
			if (fSpecifyButton->IsDown()) { fCLBDroppedPlots[(int)fPomIDEntry->GetNumber()]->Draw(); }
			else { fTotalDroppedPlot->Draw(); }
			fCanvas2->Update();

			// Canvas 3 (Total packet rate plot)
			fCanvas3->cd();
			fPacketRatePlot->Draw();
			fCanvas3->Update();
			
		} else { std::cout << "DAQonite - Error: Wrong GUI page number!" << std::endl; }
	}
}

void Monitoring_gui::drawLabels() {

	// Run Status Label
	TString statusLabelText = "Status: ";
	if (fMode) { 
		statusLabelText += "Mining";
		fRunStatusLabel->SetBackgroundColor(TColor::Number2Pixel(8));
	} else {  
		statusLabelText += "Monitoring From The Lunch Room";
		fRunStatusLabel->SetBackgroundColor(TColor::Number2Pixel(46));
	}
	fRunStatusLabel->SetText(statusLabelText);

	// Run Type Label
	TString typeLabelText = "Ore: "; 
	typeLabelText += fRunType;
	fRunTypeLabel->SetText(typeLabelText);
	if (fMode) { fRunTypeLabel->SetBackgroundColor(TColor::Number2Pixel(8)); }
	else { fRunTypeLabel->SetBackgroundColor(TColor::Number2Pixel(46)); }

	// Run Num Label
	TString numLabelText = "Dig: "; 
	numLabelText += fRunNumber;
	fRunNumLabel->SetText(numLabelText);	
	if (fMode) { fRunNumLabel->SetBackgroundColor(TColor::Number2Pixel(8)); }
	else { fRunNumLabel->SetBackgroundColor(TColor::Number2Pixel(46)); }

	// Run Time Label
	TString timeLabelText = "Time [s]: "; 
	if (fMode) { 
		fRunTimeLabel->SetBackgroundColor(TColor::Number2Pixel(8)); 
		timeLabelText += (float)(fRunUpdates*fUpdateRate)/1000; 
	}
	else { 
		fRunTimeLabel->SetBackgroundColor(TColor::Number2Pixel(46)); 
		timeLabelText += "0"; 
	}
	fRunTimeLabel->SetText(timeLabelText);

	// Run Packets Label
	TString packetsLabelText = "Loads: ";
	packetsLabelText += fRunTotalPackets; 
	fRunPacketsLabel->SetText(packetsLabelText);
	if (fMode) { fRunPacketsLabel->SetBackgroundColor(TColor::Number2Pixel(8)); }
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

	// Draw the correct direction button
	drawDirectionButtons();

	// Fact Label 1
	TString fact1Labeltext;
	if (fPageNum == 0) {
		fact1Labeltext = "Active CLBs: ";
		fact1Labeltext += fActiveCLBs;
		fact1Labeltext += "/";
		fact1Labeltext += fNumCLBs;

		if (fActiveCLBs == fNumCLBs) { fFactLabel1->SetBackgroundColor(TColor::Number2Pixel(8)); }
		else { fFactLabel1->SetBackgroundColor(TColor::Number2Pixel(46)); }

	} else {
		fact1Labeltext = "Not yet implemented";
		fFactLabel1->SetBackgroundColor(TColor::Number2Pixel(38)); 
	}
	fFactLabel1->SetText(fact1Labeltext);

	// Fact Label 2
	TString fact2Labeltext;
	if (fPageNum == 0) {
		fact2Labeltext = "Active Channels: ";
		fact2Labeltext += fActiveChannels;
		fact2Labeltext += "/";
		fact2Labeltext += fTotalNumChannels;

		if (fActiveChannels == fTotalNumChannels) { fFactLabel2->SetBackgroundColor(TColor::Number2Pixel(8)); }
		else { fFactLabel2->SetBackgroundColor(TColor::Number2Pixel(46)); }

	} else {
		fact2Labeltext = "Not yet implemented";
		fFactLabel2->SetBackgroundColor(TColor::Number2Pixel(38)); 
	}
	fFactLabel2->SetText(fact2Labeltext);

	// Fact Label 3
	TString fact3Labeltext;
	if (fPageNum == 0) {
		fact3Labeltext = "Odd Channels: ";
		fact3Labeltext += fOddChannels;
		fact3Labeltext += "/";
		fact3Labeltext += fTotalNumChannels;

		if (fOddChannels == 0) { fFactLabel3->SetBackgroundColor(TColor::Number2Pixel(8)); }
		else { fFactLabel3->SetBackgroundColor(TColor::Number2Pixel(46)); }		

	} else {
		fact3Labeltext = "Not yet implemented";
		fFactLabel3->SetBackgroundColor(TColor::Number2Pixel(38)); 
	}
	fFactLabel3->SetText(fact3Labeltext);

	// Fact Label 4
	TString fact4Labeltext;
	if (fPageNum == 0) {

		if (!fNonConfigData) {
			fact4Labeltext = "No Non Config Data";
			fFactLabel4->SetBackgroundColor(TColor::Number2Pixel(8)); 			
		} else {
			fact4Labeltext = "Non config data present";
			fFactLabel4->SetBackgroundColor(TColor::Number2Pixel(46)); 			
		}

	} else {
		fact4Labeltext = "Not yet implemented";
		fFactLabel4->SetBackgroundColor(TColor::Number2Pixel(38)); 
	}
	fFactLabel4->SetText(fact4Labeltext);
}

void Monitoring_gui::drawDirectionButtons() {

	if (fPageNum == 0) { 
		fBackButton->SetText("<--- Packets");
		fForwardButton->SetText("Environment --->");
	} else if (fPageNum == 1) {
		fBackButton->SetText("<--- Hit Rates");
		fForwardButton->SetText("Packets --->");		
	} else if (fPageNum == 2) {
		fBackButton->SetText("<--- Environment");
		fForwardButton->SetText("Hit Rates --->");		
	} else { std::cout << "DAQonite - Error: Wrong GUI page number!" << std::endl; }
}

void Monitoring_gui::drawLogo(TCanvas* canvas) {
	TImage *logo = TImage::Open("../data/logo.png");
	logo->SetConstRatio(kFALSE);
	canvas->cd(); logo->Draw(); canvas->Update();
}

TH1F* Monitoring_gui::makeTotalRatePlot(unsigned int pomIndex, unsigned int channel) {
	TString plotName = "TotalRatePlot_";
	plotName += pomIndex;
	plotName += "_";
	plotName += channel;

	TH1F* totalRatePlot = new TH1F(plotName, plotName, PLOTLENGTH, 0, PLOTLENGTH);
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

TH1F* Monitoring_gui::makeTotalRatePlot() {
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

TH1F* Monitoring_gui::makePacketRatePlot() {
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

TH2F* Monitoring_gui::makeHeatMapPlot() {
	TH2F* rateHeatMapPlot = new TH2F("RateHeatMapPlot", "RateHeatMapPlot", PMTSPERPOM, -0.5, PMTSPERPOM - 0.5, 
									 fNumCLBs, -0.5, fNumCLBs - 0.5);
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

TH1F* Monitoring_gui::makeTemperaturePlot(unsigned int pomIndex) {
	TString plotName = "temperaturePlot_";
	plotName += pomIndex;

	TH1F* temperaturePlot = new TH1F(plotName, plotName, PLOTLENGTH, 0, PLOTLENGTH);
	temperaturePlot->GetXaxis()->SetTitle("cycleCounter");
	temperaturePlot->GetYaxis()->SetTitle("Temperature [deg Celsius]");
	temperaturePlot->GetYaxis()->CenterTitle();
	temperaturePlot->GetYaxis()->SetTitleSize(0.14);
	temperaturePlot->GetYaxis()->SetTitleOffset(0.3);
	temperaturePlot->GetYaxis()->SetLabelSize(0.08);
	temperaturePlot->SetFillColor(9);
	temperaturePlot->SetLineColor(kBlack);
	temperaturePlot->SetLineWidth(2);
	temperaturePlot->SetStats(0);
	return temperaturePlot;
}

TH1F* Monitoring_gui::makeTemperaturePlot() {
	TH1F* temperaturePlot = new TH1F("temperaturePlot", "temperaturePlot", PLOTLENGTH, 0, PLOTLENGTH);
	temperaturePlot->GetXaxis()->SetTitle("cycleCounter");
	temperaturePlot->GetYaxis()->SetTitle("Temperature [deg Celsius]");
	temperaturePlot->GetYaxis()->CenterTitle();
	temperaturePlot->GetYaxis()->SetTitleSize(0.14);
	temperaturePlot->GetYaxis()->SetTitleOffset(0.3);
	temperaturePlot->GetYaxis()->SetLabelSize(0.08);
	temperaturePlot->SetFillColor(9);
	temperaturePlot->SetLineColor(kBlack);
	temperaturePlot->SetLineWidth(2);
	temperaturePlot->SetStats(0);
	return temperaturePlot;
}

TH1F* Monitoring_gui::makeHumidityPlot(unsigned int pomIndex) {
	TString plotName = "humidityPlot_";
	plotName += pomIndex;

	TH1F* humidityPlot = new TH1F(plotName, plotName, PLOTLENGTH, 0, PLOTLENGTH);
	humidityPlot->GetXaxis()->SetTitle("cycleCounter");
	humidityPlot->GetYaxis()->SetTitle("Humidity [RH]");
	humidityPlot->GetYaxis()->CenterTitle();
	humidityPlot->GetYaxis()->SetTitleSize(0.14);
	humidityPlot->GetYaxis()->SetTitleOffset(0.3);
	humidityPlot->GetYaxis()->SetLabelSize(0.08);
	humidityPlot->SetFillColor(9);
	humidityPlot->SetLineColor(kBlack);
	humidityPlot->SetLineWidth(2);
	humidityPlot->SetStats(0);
	return humidityPlot;
}

TH1F* Monitoring_gui::makeHumidityPlot() {
	TH1F* humidityPlot = new TH1F("humidityPlot", "humidityPlot", PLOTLENGTH, 0, PLOTLENGTH);
	humidityPlot->GetXaxis()->SetTitle("cycleCounter");
	humidityPlot->GetYaxis()->SetTitle("Humidity [RH]");
	humidityPlot->GetYaxis()->CenterTitle();
	humidityPlot->GetYaxis()->SetTitleSize(0.14);
	humidityPlot->GetYaxis()->SetTitleOffset(0.3);
	humidityPlot->GetYaxis()->SetLabelSize(0.08);
	humidityPlot->SetFillColor(9);
	humidityPlot->SetLineColor(kBlack);
	humidityPlot->SetLineWidth(2);
	humidityPlot->SetStats(0);
	return humidityPlot;
}

TH1F* Monitoring_gui::makeRunPacketPlot(unsigned int pomIndex) {
	TString plotName = "runPacketPlot_";
	plotName += pomIndex;

	TH1F* runPacketPlot = new TH1F(plotName, plotName, PLOTLENGTH, 0, PLOTLENGTH);
	runPacketPlot->GetXaxis()->SetTitle("cycleCounter");
	runPacketPlot->GetYaxis()->SetTitle("Optical Packets");
	runPacketPlot->GetYaxis()->CenterTitle();
	runPacketPlot->GetYaxis()->SetTitleSize(0.14);
	runPacketPlot->GetYaxis()->SetTitleOffset(0.3);
	runPacketPlot->GetYaxis()->SetLabelSize(0.08);
	runPacketPlot->SetFillColor(9);
	runPacketPlot->SetLineColor(kBlack);
	runPacketPlot->SetLineWidth(2);
	runPacketPlot->SetStats(0);
	return runPacketPlot;
}

TH1F* Monitoring_gui::makeRunPacketPlot() {
	TH1F* runPacketPlot = new TH1F("runPacketPlot", "runPacketPlot", PLOTLENGTH, 0, PLOTLENGTH);
	runPacketPlot->GetXaxis()->SetTitle("cycleCounter");
	runPacketPlot->GetYaxis()->SetTitle("Packets Received");
	runPacketPlot->GetYaxis()->CenterTitle();
	runPacketPlot->GetYaxis()->SetTitleSize(0.14);
	runPacketPlot->GetYaxis()->SetTitleOffset(0.3);
	runPacketPlot->GetYaxis()->SetLabelSize(0.08);
	runPacketPlot->SetFillColor(9);
	runPacketPlot->SetLineColor(kBlack);
	runPacketPlot->SetLineWidth(2);
	runPacketPlot->SetStats(0);
	return runPacketPlot;
}

TH1F* Monitoring_gui::makeRunDroppedPlot(unsigned int pomIndex) {
	TString plotName = "runDroppedPlot_";
	plotName += pomIndex;

	TH1F* runDroppedPlot = new TH1F(plotName, plotName, PLOTLENGTH, 0, PLOTLENGTH);
	runDroppedPlot->GetXaxis()->SetTitle("cycleCounter");
	runDroppedPlot->GetYaxis()->SetTitle("Dropped Packets");
	runDroppedPlot->GetYaxis()->CenterTitle();
	runDroppedPlot->GetYaxis()->SetTitleSize(0.14);
	runDroppedPlot->GetYaxis()->SetTitleOffset(0.3);
	runDroppedPlot->GetYaxis()->SetLabelSize(0.08);
	runDroppedPlot->SetFillColor(9);
	runDroppedPlot->SetLineColor(kBlack);
	runDroppedPlot->SetLineWidth(2);
	runDroppedPlot->SetStats(0);
	return runDroppedPlot;
}

TH1F* Monitoring_gui::makeRunDroppedPlot() {
	TH1F* runDroppedPlot = new TH1F("runDroppedPlot", "runDroppedPlot", PLOTLENGTH, 0, PLOTLENGTH);
	runDroppedPlot->GetXaxis()->SetTitle("cycleCounter");
	runDroppedPlot->GetYaxis()->SetTitle("Dropped Packets");
	runDroppedPlot->GetYaxis()->CenterTitle();
	runDroppedPlot->GetYaxis()->SetTitleSize(0.14);
	runDroppedPlot->GetYaxis()->SetTitleOffset(0.3);
	runDroppedPlot->GetYaxis()->SetLabelSize(0.08);
	runDroppedPlot->SetFillColor(9);
	runDroppedPlot->SetLineColor(kBlack);
	runDroppedPlot->SetLineWidth(2);
	runDroppedPlot->SetStats(0);
	return runDroppedPlot;
}

void Monitoring_gui::toggleSpecific() {
	std::cout << "\nDAQonite - Toggle specific" << std::endl;

	draw();
}

void Monitoring_gui::pageBackward() {
	if (fPageNum == 0) { fPageNum = 2; }
	else if (fPageNum == 1) { fPageNum = 0; }
	else if (fPageNum == 2) { fPageNum = 1; }
	else { std::cout << "DAQonite - Error: Wrong GUI page number!" << std::endl; }

	// Only draw if we are not receiving packets. Need to setup a mutex do deal with this correctly
	if (fPacketsReceived == 0) { draw(); }
}

void Monitoring_gui::pageForward() {
	if (fPageNum == 0) { fPageNum = 1; }
	else if (fPageNum == 1) { fPageNum = 2; }
	else if (fPageNum == 2) { fPageNum = 0; }
	else { std::cout << "DAQonite - Error: Wrong GUI page number!" << std::endl; }

	// Only draw if we are not receiving packets. Need to setup a mutex do deal with this correctly
	if (fPacketsReceived == 0) { draw(); }
}

void Monitoring_gui::startRun(unsigned int type, unsigned int run, TString fileName) {
	fMode = true;

	fRunNumber = run;
	fRunType = type;
	fRunTotalPackets = 0;
	fRunTotalDropped = 0;
	fRunFile = fileName;	
	fRunUpdates = 0;

	// Only draw if we are not receiving packets. Need to setup a mutex do deal with this correctly
	if (fPacketsReceived == 0) { draw(); }
}

void Monitoring_gui::stopRun() {
	fMode = false;

	// Reset the run variables
	fRunNumber = -1;
	fRunType = -1;
	fRunTotalPackets = 0;
	fRunTotalDropped = 0;
	fRunFile = "";
	fRunUpdates = 0;

	// Reset the packet variables
	for (int clbNum = 0; clbNum<fNumCLBs; clbNum++) {
		fOptical_packets[clbNum] = 0;
		fOptical_seq[clbNum] = 0;
		fOptical_dropped[clbNum] = 0;
		fMonitoring_packets[clbNum] = 0;
		fMonitoring_seq[clbNum] = 0;
		fMonitoring_dropped[clbNum] = 0;
	}

	// Only draw if we are not receiving packets. Need to setup a mutex do deal with this correctly
	if (fPacketsReceived == 0) { draw(); }
}

void Monitoring_gui::update() {
	updatePlots();
	draw();
}