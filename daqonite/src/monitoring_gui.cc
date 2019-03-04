/**
 * MonitoringGui - The ROOT monitoring GUI for DAQonite
 */

#include "monitoring_gui.h"

#define PLOTLENGTH 100
#define PMTSPERPOM 30
#define HIGHRATE 10000

MonitoringGui::MonitoringGui(int updateRate, std::string configFile) : 
							 fUpdate_rate(updateRate), fConfig_file(configFile) {

	//////////////////////////////////////
	//			GUI SETUP START			//
	//////////////////////////////////////

	// main frame
	fMain_frame = new TGMainFrame(gClient->GetRoot(),10,10,kMainFrame | kVerticalFrame);
	fMain_frame->SetName("fMain_frame");
	fMain_frame->SetLayoutBroken(kTRUE);

	ULong_t ucolor;        // will reflect user color changes
	gClient->GetColorByName("#cccccc",ucolor);

	// Frame 1 //
	TGHorizontalFrame *frame_1 = new TGHorizontalFrame(fMain_frame,800,200,kHorizontalFrame | kRaisedFrame,ucolor);
	frame_1->SetName("frame_1");
	frame_1->SetLayoutBroken(kTRUE);

	// embedded canvas
	TRootEmbeddedCanvas *e_canvas_1 = new TRootEmbeddedCanvas(0,frame_1,790,190,kRaisedFrame);
	e_canvas_1->SetName("e_canvas_1");
	Int_t we_canvas_1 = e_canvas_1->GetCanvasWindowId();
	TCanvas *c123 = new TCanvas("c123", 10, 10, we_canvas_1);
	e_canvas_1->AdoptCanvas(c123);
	frame_1->AddFrame(e_canvas_1, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	e_canvas_1->MoveResize(5,5,790,190);

	fMain_frame->AddFrame(frame_1, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	frame_1->MoveResize(5,5,800,200);
	/////////////

	// Frame 2 //
	TGHorizontalFrame *frame_2 = new TGHorizontalFrame(fMain_frame,800,200,kHorizontalFrame | kRaisedFrame,ucolor);
	frame_2->SetName("frame_2");
	frame_2->SetLayoutBroken(kTRUE);

	// embedded canvas
	TRootEmbeddedCanvas *e_canvas_2 = new TRootEmbeddedCanvas(0,frame_2,790,190,kRaisedFrame);
	e_canvas_2->SetName("e_canvas_2");
	Int_t we_canvas_2 = e_canvas_2->GetCanvasWindowId();
	TCanvas *c124 = new TCanvas("c124", 10, 10, we_canvas_2);
	e_canvas_2->AdoptCanvas(c124);
	frame_2->AddFrame(e_canvas_2, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	e_canvas_2->MoveResize(5,5,790,190);

	fMain_frame->AddFrame(frame_2, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	frame_2->MoveResize(5,215,800,200);
	/////////////

	// Frame 4 //
	TGHorizontalFrame *frame_4 = new TGHorizontalFrame(fMain_frame,800,200,kHorizontalFrame | kRaisedFrame,ucolor);
	frame_4->SetName("frame_4");
	frame_4->SetLayoutBroken(kTRUE);

	// embedded canvas
	TRootEmbeddedCanvas *e_canvas_3 = new TRootEmbeddedCanvas(0,frame_4,790,190,kRaisedFrame);
	e_canvas_3->SetName("e_canvas_3");
	Int_t we_canvas_3 = e_canvas_3->GetCanvasWindowId();
	TCanvas *c125 = new TCanvas("c125", 10, 10, we_canvas_3);
	e_canvas_3->AdoptCanvas(c125);
	frame_4->AddFrame(e_canvas_3, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	e_canvas_3->MoveResize(5,5,790,190);

	fMain_frame->AddFrame(frame_4, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	frame_4->MoveResize(5,635,800,200);
	/////////////

	// Frame 3 //
	TGHorizontalFrame *frame_3 = new TGHorizontalFrame(fMain_frame,800,200,kHorizontalFrame | kRaisedFrame,ucolor);
	frame_3->SetName("frame_3");
	frame_3->SetLayoutBroken(kTRUE);
	TGVertical3DLine *vertical_line = new TGVertical3DLine(frame_3,1,190);
	vertical_line->SetName("vertical_line");
	frame_3->AddFrame(vertical_line, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	vertical_line->MoveResize(400,5,1,190);

	gClient->GetColorByName("#7d99d1",ucolor);
	fRun_status_label = new TGLabel(frame_3,"In The Lunch Room",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fRun_status_label->SetTextJustify(36);
	fRun_status_label->SetMargins(0,0,0,0);
	fRun_status_label->SetWrapLength(-1);
	frame_3->AddFrame(fRun_status_label, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fRun_status_label->MoveResize(5,5,390,45);

	fRun_type_label = new TGLabel(frame_3,"Ore:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fRun_type_label->SetTextJustify(36);
	fRun_type_label->SetMargins(0,0,0,0);
	fRun_type_label->SetWrapLength(-1);
	frame_3->AddFrame(fRun_type_label, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fRun_type_label->MoveResize(5,58,190,40);

	fRun_num_label = new TGLabel(frame_3,"Dig:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fRun_num_label->SetTextJustify(36);
	fRun_num_label->SetMargins(0,0,0,0);
	fRun_num_label->SetWrapLength(-1);
	frame_3->AddFrame(fRun_num_label, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fRun_num_label->MoveResize(205,58,190,40);

	fRun_time_label = new TGLabel(frame_3,"Time:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fRun_time_label->SetTextJustify(36);
	fRun_time_label->SetMargins(0,0,0,0);
	fRun_time_label->SetWrapLength(-1);
	frame_3->AddFrame(fRun_time_label, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fRun_time_label->MoveResize(5,106,190,40);

	fRun_packets_label = new TGLabel(frame_3,"Loads:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fRun_packets_label->SetTextJustify(36);
	fRun_packets_label->SetMargins(0,0,0,0);
	fRun_packets_label->SetWrapLength(-1);
	frame_3->AddFrame(fRun_packets_label, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fRun_packets_label->MoveResize(205,106,190,40);

	fRun_file_label = new TGLabel(frame_3,"Container:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fRun_file_label->SetTextJustify(36);
	fRun_file_label->SetMargins(0,0,0,0);
	fRun_file_label->SetWrapLength(-1);
	frame_3->AddFrame(fRun_file_label, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fRun_file_label->MoveResize(5,154,390,40);

	fPom_id_entry = new TGNumberEntry(frame_3, (Double_t) 0,6,-1,(TGNumberFormat::EStyle) 5);
	fPom_id_entry->SetName("fPom_id_entry");
	frame_3->AddFrame(fPom_id_entry, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fPom_id_entry->MoveResize(630,66,60,22);

	fChannel_entry = new TGNumberEntry(frame_3, (Double_t) 0,6,-1,(TGNumberFormat::EStyle) 5);
	fChannel_entry->SetName("fChannel_entry");
	frame_3->AddFrame(fChannel_entry, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fChannel_entry->MoveResize(710,66,60,22);

	fSpecify_button = new TGCheckButton(frame_3,"Specify [CLB] and [Channel]:",-1,TGCheckButton::GetDefaultGC()(),TGCheckButton::GetDefaultFontStruct(),kRaisedFrame);
	fSpecify_button->Connect("Clicked()","MonitoringGui",this,"toggleSpecific()");
	fSpecify_button->SetTextJustify(36);
	fSpecify_button->SetMargins(0,0,0,0);
	fSpecify_button->SetWrapLength(-1);
	frame_3->AddFrame(fSpecify_button, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fSpecify_button->MoveResize(410,66,180,22);

	fBack_button = new TGTextButton(frame_3,"<--- Packets",-1,TGTextButton::GetDefaultGC()(),TGTextButton::GetDefaultFontStruct(),kRaisedFrame);
	fBack_button->Connect("Clicked()","MonitoringGui",this,"pageBackward()");
	fBack_button->SetTextJustify(36);
	fBack_button->SetMargins(0,0,0,0);
	fBack_button->SetWrapLength(-1);
	fBack_button->Resize(190,45);
	fBack_button->ChangeBackground(ucolor);
	frame_3->AddFrame(fBack_button, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fBack_button->MoveResize(405,5,190,45);

	fForward_button = new TGTextButton(frame_3,"Environment --->",-1,TGTextButton::GetDefaultGC()(),TGTextButton::GetDefaultFontStruct(),kRaisedFrame);
	fForward_button->Connect("Clicked()","MonitoringGui",this,"pageForward()");
	fForward_button->SetTextJustify(36);
	fForward_button->SetMargins(0,0,0,0);
	fForward_button->SetWrapLength(-1);
	fForward_button->Resize(190,45);
	fForward_button->ChangeBackground(ucolor);
	frame_3->AddFrame(fForward_button, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fForward_button->MoveResize(605,5,190,45);

	fFact_label_1 = new TGLabel(frame_3,"Fact:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fFact_label_1->SetTextJustify(36);
	fFact_label_1->SetMargins(0,0,0,0);
	fFact_label_1->SetWrapLength(-1);
	frame_3->AddFrame(fFact_label_1, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fFact_label_1->MoveResize(405,106,190,40);

	fFact_label_2 = new TGLabel(frame_3,"Fact:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fFact_label_2->SetTextJustify(36);
	fFact_label_2->SetMargins(0,0,0,0);
	fFact_label_2->SetWrapLength(-1);
	frame_3->AddFrame(fFact_label_2, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fFact_label_2->MoveResize(605,106,190,40);

	fFact_label_3 = new TGLabel(frame_3,"Fact:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fFact_label_3->SetTextJustify(36);
	fFact_label_3->SetMargins(0,0,0,0);
	fFact_label_3->SetWrapLength(-1);
	frame_3->AddFrame(fFact_label_3, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fFact_label_3->MoveResize(405,154,190,40);

	fFact_label_4 = new TGLabel(frame_3,"Fact:",TGLabel::GetDefaultGC()(),TGLabel::GetDefaultFontStruct(),kSunkenFrame,ucolor);
	fFact_label_4->SetTextJustify(36);
	fFact_label_4->SetMargins(0,0,0,0);
	fFact_label_4->SetWrapLength(-1);
	frame_3->AddFrame(fFact_label_4, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	fFact_label_4->MoveResize(605,154,190,40);

	fMain_frame->AddFrame(frame_3, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
	frame_3->MoveResize(5,425,800,200);
	/////////////

	fMain_frame->SetMWMHints(kMWMDecorAll,
							kMWMFuncAll,
							kMWMInputModeless);
	fMain_frame->MapSubwindows();

	fMain_frame->Resize(fMain_frame->GetDefaultSize());
	fMain_frame->MapWindow();
	fMain_frame->Resize(812,841);

	// Set up the other variables...
	fCanvas_1 = e_canvas_1->GetCanvas();
	fCanvas_2 = e_canvas_2->GetCanvas();
	fCanvas_3 = e_canvas_3->GetCanvas();

	//////////////////////////////////////
	//			GUI SETUP END			//
	//////////////////////////////////////

	// Set the configuration variables using the Monitoring_config class
	MonitoringConfig fConfig(fConfig_file.c_str());
	fNum_clbs = fConfig.getNumCLBs();
	fCLB_eids = fConfig.getCLBeIDs();
	fTotal_num_channels = fConfig.getTotalNumChannels();

	// Important variables
	fPage_num = 0;
	fMode = false;

	// Current run variables
	fRun_num = -1;
	fRun_type = -1;
	fRun_total_packets = 0;
	fRun_total_dropped = 0;
	fRun_file = "";
	fRun_updates = 0;

	// Window variables
	fUpdate_packets = 0;

	// Monitoring variables
	fPackets_received = 0;
	fNum_updates = 0;
	fNum_refresh = 0;
	fActive_clbs = 0;
	fActive_channels = 0;
	fOdd_channels = 0;
	fNon_config_data = false;

	setupArrays();
	setupPlots();
	draw();
}

MonitoringGui::~MonitoringGui() {
	fMain_frame->Cleanup();
}

void MonitoringGui::addOpticalPacket(unsigned int pomID, unsigned int seqNumber) {
	// Search for this pomID in the config CLB IDs
	std::vector<unsigned int>::iterator it = std::find(fCLB_eids.begin(), fCLB_eids.end(), pomID);

	// If this is a config CLB and we are in running mode we do packet checking
	if (it != fCLB_eids.end() && fMode) {
		int clbIndex = std::distance(fCLB_eids.begin(), it);

		// Check to see if we have missed a packet
		if (fOptical_packets[clbIndex] != 0) {
			int seqDiff = seqNumber - fOptical_seq[clbIndex];

			if (seqDiff == 1) {} 
			else if (seqDiff > 1) { 
				fOptical_dropped[clbIndex] += seqDiff; 
				fRun_total_dropped += seqDiff;
			} else { 
				//std::cout << "DAQonite - Error: Seq: " << seqNumber << "," << fMonitoring_seq[clbIndex] << std::endl;  
			}
		}

		// Set the current sequence number for this CLB
		fOptical_seq[clbIndex] = seqNumber;

		// Increment the optical packet counter
		fOptical_packets[clbIndex] ++;	

		// Increment the total run packet counter
		fRun_total_packets ++;

		// Increment the packet counter 
		fPackets_received ++;

	} else if (it == fCLB_eids.end()) {
		fNon_config_data = true;
	}
}

void MonitoringGui::addMonitoringPacket(unsigned int pomID, unsigned int hits[30], 
										 float temp, float humidity, unsigned int seqNumber) {
	// Search for this pomID in the config CLB IDs
	std::vector<unsigned int>::iterator it = std::find(fCLB_eids.begin(), fCLB_eids.end(), pomID);

	if (it != fCLB_eids.end()) {
		int clbIndex = std::distance(fCLB_eids.begin(), it);

		// If we are in running mode we do packet checking
		if (fMode) {
			// Check to see if we have missed a packet
			if (fMonitoring_packets[clbIndex] != 0) {
				int seqDiff = seqNumber - fMonitoring_seq[clbIndex];

				if (seqDiff == 1) {} 
				else if (seqDiff > 1) { 
					fMonitoring_dropped[clbIndex] += seqDiff; 
					fRun_total_dropped += seqDiff;
				} else { 
					//std::cout << "DAQonite - Error: Seq: " << seqNumber << "," << fMonitoring_seq[clbIndex] << std::endl;  
				}
			}

			// Set the current sequence number for this CLB
			fMonitoring_seq[clbIndex] = seqNumber;

			// Increment the monitoring packet counter
			fMonitoring_packets[clbIndex] ++;	

			// Increment the total run packet counter
			fRun_total_packets ++;		
		}

		// This is a valid CLB. Therefore, add the hits...
		for (int channel = 0; channel < 30; channel ++) fRate_array[clbIndex][channel] += hits[channel];

		// Update the temperature and humidity for this CLB
		fTemp_array[clbIndex] = temp;
		fHumidity_array[clbIndex] = humidity;

		// Increment the number of packets received in this update window
		fUpdate_packets++;

		// Increment the packet counter 
		fPackets_received ++;

	} else {
		fNon_config_data = true;
	}
}

void MonitoringGui::setupArrays() {
	fRate_array.clear();

	// Add a clean channel vector into the rate array for each CLB
	for (int clbNum = 0; clbNum<fNum_clbs; clbNum++) {
		std::vector<unsigned int> channelVec(PMTSPERPOM);
		fRate_array.push_back(channelVec);
		clearPomRates(clbNum);

		fTemp_array.push_back(0.0);
		fHumidity_array.push_back(0.0);

		// Packet variables
		fOptical_packets.push_back(0);
		fOptical_seq.push_back(0);
		fOptical_dropped.push_back(0);

		fMonitoring_packets.push_back(0);
		fMonitoring_seq.push_back(0);
		fMonitoring_dropped.push_back(0);
	}
}

void MonitoringGui::setupPlots() {

	// Make the individual total plots
	fTotal_rate_plot = makeTotalRatePlot();
	fPacket_rate_plot = makePacketRatePlot();
	fRate_map_plot = makeHeatMapPlot();
	fAvg_temp_plot = makeTemperaturePlot();
	fAvg_humidity_plot = makeHumidityPlot();
	fTotal_packet_plot = makeRunPacketPlot();
	fTotal_dropped_plot = makeRunDroppedPlot();

	for (int clbNum = 0; clbNum<fNum_clbs; clbNum++) {

		// Make the CLB specific temp and humidity plots
		fCLB_temp_plots.push_back(makeTemperaturePlot(fCLB_eids[clbNum]));
		fCLB_humidity_plots.push_back(makeHumidityPlot(fCLB_eids[clbNum]));

		// Make the CLB specific packet plots
		fCLB_packet_plots.push_back(makeRunPacketPlot(fCLB_eids[clbNum]));
		fCLB_dropped_plots.push_back(makeRunDroppedPlot(fCLB_eids[clbNum]));

		// Make the channel specific rate plots
		for (int channel=0; channel<PMTSPERPOM; channel++) {
			fChannel_rate_plots.push_back(makeTotalRatePlot(fCLB_eids[clbNum], channel));
		}
	}
}

void MonitoringGui::clearPomRates(unsigned int pomIndex) {
	for(int channel = 0; channel<PMTSPERPOM; channel++) fRate_array[pomIndex][channel] = 0;
}

void MonitoringGui::updatePlots() {
	fNum_updates++;

	if (fMode) { fRun_updates ++; }

	// Do we need to clear the plots that have reached PLOTLENGTH and start again?
	if ((fNum_updates % PLOTLENGTH) == 0) { refreshPlots(); }

	// Need variables to track things...
	fActive_clbs 				= 0;
	fActive_channels				= 0;
	fOdd_channels 				= 0;

	int 	totalHits 			= 0;
	bool 	clbHits[fNum_clbs];
	float 	sumTemp 			= 0.0;
	float 	sumHumidity 		= 0.0;

	// We need to loop through the fRate_array and update the plots and then clear it...
	for (int pomIndex = 0; pomIndex < fNum_clbs; pomIndex++) {
		clbHits[pomIndex] = false;

		// Update the CLB specific temp plots
		float clbTemp = fTemp_array[pomIndex];
		sumTemp += clbTemp;
		fCLB_temp_plots[pomIndex]->SetBinContent(fNum_updates-(fNum_refresh*PLOTLENGTH), clbTemp);

		// Update the CLB specific humidity plots
		float clbHumidity = fHumidity_array[pomIndex];
		sumHumidity += clbHumidity;
		fCLB_humidity_plots[pomIndex]->SetBinContent(fNum_updates-(fNum_refresh*PLOTLENGTH), clbHumidity);

		// Update the CLB specific packet and dropped plots
		int clbTotalPackets = fOptical_packets[pomIndex] + fMonitoring_packets[pomIndex];
		fCLB_packet_plots[pomIndex]->SetBinContent(fNum_updates-(fNum_refresh*PLOTLENGTH), clbTotalPackets);
		int clbDroppedPackets = fOptical_dropped[pomIndex] + fMonitoring_dropped[pomIndex];
		fCLB_dropped_plots[pomIndex]->SetBinContent(fNum_updates-(fNum_refresh*PLOTLENGTH), clbDroppedPackets);

		for (int channelIndex = 0; channelIndex < PMTSPERPOM; channelIndex++) {
			int hits = (int)fRate_array[pomIndex][channelIndex];

			// Add to the total hits for this window
			totalHits += hits;

			// If there are hits the channel is active
			if ( hits > 0 ) { 
				clbHits[pomIndex] = true;
				fActive_channels++; 
			}

			// If more than HIGHRATE hits its odd
			if ( (hits/((float)fUpdate_rate/1000)) > HIGHRATE) { fOdd_channels++; }

			// Set the individual channel rate plot
			int plotVectorIndex = (pomIndex*PMTSPERPOM) + (channelIndex);
			fChannel_rate_plots[plotVectorIndex]->SetBinContent(fNum_updates-(fNum_refresh*PLOTLENGTH), hits/((float)fUpdate_rate/1000));

			// Set the heat map bin for this channel
			fRate_map_plot->SetBinContent(channelIndex+1, pomIndex+1, hits/((float)fUpdate_rate/1000));
		}
		clearPomRates(pomIndex);
	}

	for (int clb = 0; clb<fNum_clbs; clb++) {
		if (clbHits[clb] == true) { fActive_clbs++; }
	}

	// Set the total channel rate plot
	fTotal_rate_plot->SetBinContent(fNum_updates-(fNum_refresh*PLOTLENGTH), (float)totalHits / ((float)fUpdate_rate/1000));

	// Set the total packet rate plot
	fPacket_rate_plot->SetBinContent(fNum_updates-(fNum_refresh*PLOTLENGTH), fUpdate_packets / ((float)fUpdate_rate/1000));

	// Set the temperature and humidity average plots
	fAvg_temp_plot->SetBinContent(fNum_updates-(fNum_refresh*PLOTLENGTH), sumTemp / (float)fActive_clbs);
	fAvg_humidity_plot->SetBinContent(fNum_updates-(fNum_refresh*PLOTLENGTH), sumHumidity / (float)fActive_clbs);

	// Set the packet plots
	fTotal_packet_plot->SetBinContent(fNum_updates-(fNum_refresh*PLOTLENGTH), fRun_total_packets);
	fTotal_dropped_plot->SetBinContent(fNum_updates-(fNum_refresh*PLOTLENGTH), fRun_total_dropped);

	fUpdate_packets = 0;
}

void MonitoringGui::refreshPlots() {
	std::cout << "\nDAQonite - Refresh plots" << std::endl;
	fNum_refresh++;

	// Clear the temp/humidity clb plots and individual channel hit plots
	for(int clb = 0; clb < fNum_clbs; clb++) {

		delete fCLB_temp_plots[clb];
		fCLB_temp_plots[clb] = NULL;
		fCLB_temp_plots[clb] = makeTemperaturePlot(fCLB_eids[clb]);

		delete fCLB_humidity_plots[clb];
		fCLB_humidity_plots[clb] = NULL;
		fCLB_humidity_plots[clb] = makeHumidityPlot(fCLB_eids[clb]);

		delete fCLB_packet_plots[clb];
		fCLB_packet_plots[clb] = NULL;
		fCLB_packet_plots[clb] = makeRunPacketPlot(fCLB_eids[clb]);

		delete fCLB_dropped_plots[clb];
		fCLB_dropped_plots[clb] = NULL;
		fCLB_dropped_plots[clb] = makeRunDroppedPlot(fCLB_eids[clb]);

		for (int channel=0; channel<PMTSPERPOM; channel++) {
			// Set the individual channel rate plot
			int plotVectorIndex = (clb*PMTSPERPOM) + (channel);
			delete fChannel_rate_plots[plotVectorIndex];
			fChannel_rate_plots[plotVectorIndex] = NULL;
			fChannel_rate_plots[plotVectorIndex] = makeTotalRatePlot(clb, channel);
		}
	}

	// Clean the individual plots and create new ones 
	delete fTotal_rate_plot;
	fTotal_rate_plot = NULL;
	fTotal_rate_plot = makeTotalRatePlot();

	delete fPacket_rate_plot;
	fPacket_rate_plot = NULL;
	fPacket_rate_plot = makePacketRatePlot();

	delete fAvg_temp_plot;
	fAvg_temp_plot = NULL;
	fAvg_temp_plot = makeTemperaturePlot();

	delete fAvg_humidity_plot;
	fAvg_humidity_plot = NULL;
	fAvg_humidity_plot = makeHumidityPlot();

	delete fTotal_packet_plot;
	fTotal_packet_plot = NULL;
	fTotal_packet_plot = makeRunPacketPlot();

	delete fTotal_dropped_plot;
	fTotal_dropped_plot = NULL;
	fTotal_dropped_plot = makeRunDroppedPlot();
}

void MonitoringGui::draw() {
	drawPlots();
	drawLabels();
	drawDirectionButtons();
}

void MonitoringGui::drawPlots() {

	// If we have not received any packets just display the CHIPS logo on all canvases

	if (fPackets_received == 0) { // If we have not received any packets just display the CHIPS logo on all canvases

		drawLogo(fCanvas_1);
		drawLogo(fCanvas_2);
		drawLogo(fCanvas_3);

	} else if (!fMode) { // If we are not in running mode

		if (fPage_num == 0) {

			// Canvas 1 (Total rate plot or channel specific rate plot)
			fCanvas_1->cd();
			if (fSpecify_button->IsDown()) {
				fChannel_rate_plots[((int)fPom_id_entry->GetNumber()*PMTSPERPOM) + ((int)fChannel_entry->GetNumber())]->Draw();
			} else { fTotal_rate_plot->Draw(); }
			fCanvas_1->Update();

			// Canvas 2 (CHIPS Logo, will be RMS plot(s))
			drawLogo(fCanvas_2);

			// Canvas 3 (Rate heat map)
			fCanvas_3->cd();
			fRate_map_plot->Draw("COLZ");
			fCanvas_3->Update();

		} else if (fPage_num == 1) {

			// Canvas 1 (Average temperature plot or CLB specific temperature plot)
			fCanvas_1->cd();
			if (fSpecify_button->IsDown()) { fCLB_temp_plots[(int)fPom_id_entry->GetNumber()]->Draw(); }
			else { fAvg_temp_plot->Draw(); }
			fCanvas_1->Update();

			// Canvas 2 (Average humidity plot or CLB specific humidity plot)
			fCanvas_2->cd();
			if (fSpecify_button->IsDown()) { fCLB_humidity_plots[(int)fPom_id_entry->GetNumber()]->Draw(); }
			else { fAvg_humidity_plot->Draw(); }
			fCanvas_2->Update();

			// Canvas 3 (CHIPS logo)
			drawLogo(fCanvas_3);

		} else if (fPage_num == 2) {

			// Canvas 1 (CHIPS logo)
			drawLogo(fCanvas_1);

			// Canvas 2 (CHIPS logo)
			drawLogo(fCanvas_2);

			// Canvas 3 (Total packet rate plot)
			fCanvas_3->cd();
			fPacket_rate_plot->Draw();
			fCanvas_3->Update();
			
		} else { std::cout << "DAQonite - Error: Wrong GUI page number!" << std::endl; }

	} else { // If we are in running mode

		if (fPage_num == 0) {

			// Canvas 1 (Total rate plot or channel specific rate plot)
			fCanvas_1->cd();
			if (fSpecify_button->IsDown()) {
				fChannel_rate_plots[((int)fPom_id_entry->GetNumber()*PMTSPERPOM) + ((int)fChannel_entry->GetNumber())]->Draw();
			} else { fTotal_rate_plot->Draw(); }
			fCanvas_1->Update();

			// Canvas 2 (CHIPS Logo, will be RMS plot(s))
			drawLogo(fCanvas_2);

			// Canvas 3 (Rate heat map)
			fCanvas_3->cd();
			fRate_map_plot->Draw("COLZ");
			fCanvas_3->Update();

		} else if (fPage_num == 1) {

			// Canvas 1 (Average temperature plot or CLB specific temperature plot)
			fCanvas_1->cd();
			if (fSpecify_button->IsDown()) { fCLB_temp_plots[(int)fPom_id_entry->GetNumber()]->Draw(); }
			else { fAvg_temp_plot->Draw(); }
			fCanvas_1->Update();

			// Canvas 2 (Average humidity plot or CLB specific humidity plot)
			fCanvas_2->cd();
			if (fSpecify_button->IsDown()) { fCLB_humidity_plots[(int)fPom_id_entry->GetNumber()]->Draw(); }
			else { fAvg_humidity_plot->Draw(); }
			fCanvas_2->Update();

			// Canvas 3 (CHIPS logo)
			drawLogo(fCanvas_3);

		} else if (fPage_num == 2) {

			// Canvas 1 (CHIPS logo)
			fCanvas_1->cd();
			if (fSpecify_button->IsDown()) { fCLB_packet_plots[(int)fPom_id_entry->GetNumber()]->Draw(); }
			else { fTotal_packet_plot->Draw(); }
			fCanvas_1->Update();

			// Canvas 2 (CHIPS logo)
			fCanvas_2->cd();
			if (fSpecify_button->IsDown()) { fCLB_dropped_plots[(int)fPom_id_entry->GetNumber()]->Draw(); }
			else { fTotal_dropped_plot->Draw(); }
			fCanvas_2->Update();

			// Canvas 3 (Total packet rate plot)
			fCanvas_3->cd();
			fPacket_rate_plot->Draw();
			fCanvas_3->Update();
			
		} else { std::cout << "DAQonite - Error: Wrong GUI page number!" << std::endl; }
	}
}

void MonitoringGui::drawLabels() {

	// Run Status Label
	TString statusLabelText = "Status: ";
	if (fMode) { 
		statusLabelText += "Mining";
		fRun_status_label->SetBackgroundColor(TColor::Number2Pixel(8));
	} else {  
		statusLabelText += "Monitoring From The Lunch Room";
		fRun_status_label->SetBackgroundColor(TColor::Number2Pixel(46));
	}
	fRun_status_label->SetText(statusLabelText);

	// Run Type Label
	TString typeLabelText = "Ore: "; 
	typeLabelText += fRun_type;
	fRun_type_label->SetText(typeLabelText);
	if (fMode) { fRun_type_label->SetBackgroundColor(TColor::Number2Pixel(8)); }
	else { fRun_type_label->SetBackgroundColor(TColor::Number2Pixel(46)); }

	// Run Num Label
	TString numLabelText = "Dig: "; 
	numLabelText += fRun_num;
	fRun_num_label->SetText(numLabelText);	
	if (fMode) { fRun_num_label->SetBackgroundColor(TColor::Number2Pixel(8)); }
	else { fRun_num_label->SetBackgroundColor(TColor::Number2Pixel(46)); }

	// Run Time Label
	TString timeLabelText = "Time [s]: "; 
	if (fMode) { 
		fRun_time_label->SetBackgroundColor(TColor::Number2Pixel(8)); 
		timeLabelText += (float)(fRun_updates*fUpdate_rate)/1000; 
	}
	else { 
		fRun_time_label->SetBackgroundColor(TColor::Number2Pixel(46)); 
		timeLabelText += "0"; 
	}
	fRun_time_label->SetText(timeLabelText);

	// Run Packets Label
	TString packetsLabelText = "Loads: ";
	packetsLabelText += fRun_total_packets; 
	fRun_packets_label->SetText(packetsLabelText);
	if (fMode) { fRun_packets_label->SetBackgroundColor(TColor::Number2Pixel(8)); }
	else { fRun_packets_label->SetBackgroundColor(TColor::Number2Pixel(46)); }

	// Run File Label
	TString fileLabelText = "Container: ";
	if (fRun_file != "") {
		fileLabelText += fRun_file; 
		fRun_file_label->SetBackgroundColor(TColor::Number2Pixel(8));	
	} else { 
		fileLabelText += "Not Filling a Container"; 
		fRun_file_label->SetBackgroundColor(TColor::Number2Pixel(46));
	}
	fRun_file_label->SetText(fileLabelText);

	// Draw the correct direction button
	drawDirectionButtons();

	// Fact Label 1
	TString fact1Labeltext;
	if (fPage_num == 0) {
		fact1Labeltext = "Active CLBs: ";
		fact1Labeltext += fActive_clbs;
		fact1Labeltext += "/";
		fact1Labeltext += fNum_clbs;

		if (fActive_clbs == fNum_clbs) { fFact_label_1->SetBackgroundColor(TColor::Number2Pixel(8)); }
		else { fFact_label_1->SetBackgroundColor(TColor::Number2Pixel(46)); }

	} else {
		fact1Labeltext = "Not yet implemented";
		fFact_label_1->SetBackgroundColor(TColor::Number2Pixel(38)); 
	}
	fFact_label_1->SetText(fact1Labeltext);

	// Fact Label 2
	TString fact2Labeltext;
	if (fPage_num == 0) {
		fact2Labeltext = "Active Channels: ";
		fact2Labeltext += fActive_channels;
		fact2Labeltext += "/";
		fact2Labeltext += fTotal_num_channels;

		if (fActive_channels == fTotal_num_channels) { fFact_label_2->SetBackgroundColor(TColor::Number2Pixel(8)); }
		else { fFact_label_2->SetBackgroundColor(TColor::Number2Pixel(46)); }

	} else {
		fact2Labeltext = "Not yet implemented";
		fFact_label_2->SetBackgroundColor(TColor::Number2Pixel(38)); 
	}
	fFact_label_2->SetText(fact2Labeltext);

	// Fact Label 3
	TString fact3Labeltext;
	if (fPage_num == 0) {
		fact3Labeltext = "Odd Channels: ";
		fact3Labeltext += fOdd_channels;
		fact3Labeltext += "/";
		fact3Labeltext += fTotal_num_channels;

		if (fOdd_channels == 0) { fFact_label_3->SetBackgroundColor(TColor::Number2Pixel(8)); }
		else { fFact_label_3->SetBackgroundColor(TColor::Number2Pixel(46)); }		

	} else {
		fact3Labeltext = "Not yet implemented";
		fFact_label_3->SetBackgroundColor(TColor::Number2Pixel(38)); 
	}
	fFact_label_3->SetText(fact3Labeltext);

	// Fact Label 4
	TString fact4Labeltext;
	if (fPage_num == 0) {

		if (!fNon_config_data) {
			fact4Labeltext = "No Non Config Data";
			fFact_label_4->SetBackgroundColor(TColor::Number2Pixel(8)); 			
		} else {
			fact4Labeltext = "Non config data present";
			fFact_label_4->SetBackgroundColor(TColor::Number2Pixel(46)); 			
		}

	} else {
		fact4Labeltext = "Not yet implemented";
		fFact_label_4->SetBackgroundColor(TColor::Number2Pixel(38)); 
	}
	fFact_label_4->SetText(fact4Labeltext);
}

void MonitoringGui::drawDirectionButtons() {

	if (fPage_num == 0) { 
		fBack_button->SetText("<--- Packets");
		fForward_button->SetText("Environment --->");
	} else if (fPage_num == 1) {
		fBack_button->SetText("<--- Hit Rates");
		fForward_button->SetText("Packets --->");		
	} else if (fPage_num == 2) {
		fBack_button->SetText("<--- Environment");
		fForward_button->SetText("Hit Rates --->");		
	} else { std::cout << "DAQonite - Error: Wrong GUI page number!" << std::endl; }
}

void MonitoringGui::drawLogo(TCanvas* canvas) {
	TImage *logo = TImage::Open("../data/logo.png");
	logo->SetConstRatio(kFALSE);
	canvas->cd(); logo->Draw(); canvas->Update();
}

TH1F* MonitoringGui::makeTotalRatePlot(unsigned int pomIndex, unsigned int channel) {
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

TH1F* MonitoringGui::makeTotalRatePlot() {
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

TH1F* MonitoringGui::makePacketRatePlot() {
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

TH2F* MonitoringGui::makeHeatMapPlot() {
	TH2F* rateHeatMapPlot = new TH2F("RateHeatMapPlot", "RateHeatMapPlot", PMTSPERPOM, -0.5, PMTSPERPOM - 0.5, 
									 fNum_clbs, -0.5, fNum_clbs - 0.5);
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

TH1F* MonitoringGui::makeTemperaturePlot(unsigned int pomIndex) {
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

TH1F* MonitoringGui::makeTemperaturePlot() {
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

TH1F* MonitoringGui::makeHumidityPlot(unsigned int pomIndex) {
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

TH1F* MonitoringGui::makeHumidityPlot() {
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

TH1F* MonitoringGui::makeRunPacketPlot(unsigned int pomIndex) {
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

TH1F* MonitoringGui::makeRunPacketPlot() {
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

TH1F* MonitoringGui::makeRunDroppedPlot(unsigned int pomIndex) {
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

TH1F* MonitoringGui::makeRunDroppedPlot() {
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

void MonitoringGui::toggleSpecific() {
	std::cout << "\nDAQonite - Toggle specific" << std::endl;

	draw();
}

void MonitoringGui::pageBackward() {
	if (fPage_num == 0) { fPage_num = 2; }
	else if (fPage_num == 1) { fPage_num = 0; }
	else if (fPage_num == 2) { fPage_num = 1; }
	else { std::cout << "DAQonite - Error: Wrong GUI page number!" << std::endl; }

	// Only draw if we are not receiving packets. Need to setup a mutex do deal with this correctly
	if (fPackets_received == 0) { draw(); }
}

void MonitoringGui::pageForward() {
	if (fPage_num == 0) { fPage_num = 1; }
	else if (fPage_num == 1) { fPage_num = 2; }
	else if (fPage_num == 2) { fPage_num = 0; }
	else { std::cout << "DAQonite - Error: Wrong GUI page number!" << std::endl; }

	// Only draw if we are not receiving packets. Need to setup a mutex do deal with this correctly
	if (fPackets_received == 0) { draw(); }
}

void MonitoringGui::startRun(unsigned int type, unsigned int run, TString fileName) {
	fMode = true;

	fRun_num = run;
	fRun_type = type;
	fRun_total_packets = 0;
	fRun_total_dropped = 0;
	fRun_file = fileName;	
	fRun_updates = 0;

	// Only draw if we are not receiving packets. Need to setup a mutex do deal with this correctly
	if (fPackets_received == 0) { draw(); }
}

void MonitoringGui::stopRun() {
	fMode = false;

	// Reset the run variables
	fRun_num = -1;
	fRun_type = -1;
	fRun_total_packets = 0;
	fRun_total_dropped = 0;
	fRun_file = "";
	fRun_updates = 0;

	// Reset the packet variables
	for (int clbNum = 0; clbNum<fNum_clbs; clbNum++) {
		fOptical_packets[clbNum] = 0;
		fOptical_seq[clbNum] = 0;
		fOptical_dropped[clbNum] = 0;
		fMonitoring_packets[clbNum] = 0;
		fMonitoring_seq[clbNum] = 0;
		fMonitoring_dropped[clbNum] = 0;
	}

	// Only draw if we are not receiving packets. Need to setup a mutex do deal with this correctly
	if (fPackets_received == 0) { draw(); }
}

void MonitoringGui::update() {
	updatePlots();
	draw();
}