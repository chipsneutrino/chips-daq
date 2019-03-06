/**
 * MonitoringGui - The ROOT monitoring GUI for DAQonite
 */

#include "monitoring_gui.h"

#define PLOTLENGTH 50
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
	fConfig.printConfig();
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
	fRun_file = "";
	fRun_updates = 0;

	// Monitoring variables
	fNum_updates = 0;
	fNum_refresh = 0;

	// Hit label variables
	fActive_clbs = 0;
	fActive_channels = 0;
	fOdd_channels = 0;
	fNon_config_data = false;

	// Packet label variables
	fAvg_packet_rate = 7000;
	fAvg_seq_num = 0;
	fLow_packet_rate = 10000;
	fHigh_packet_rate = 0;
	fLow_seq_num = 10;
	fHigh_seq_num = 0;

	// Temp/Humidity label variables
	fAvg_temp = 30;
	fAvg_humidity = 10;
	fLow_temp = 1000;
	fHigh_temp = 0;
	fLow_humidity = 1000;
	fHigh_humidity = 0;

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

		// Increment the total run packet counter
		fRun_total_packets ++;

		// Increment the number of packets received for this clb in this update window
		fPacket_array[clbIndex] += 1;

		fAvg_seq_num = (0.001*seqNumber) + ((1-0.001)*fAvg_seq_num);
		if (seqNumber<fLow_seq_num) fLow_seq_num = seqNumber;
		if (seqNumber>fHigh_seq_num) fHigh_seq_num = seqNumber;

	} else if (it == fCLB_eids.end()) {
		fNon_config_data = true;
	}
}

void MonitoringGui::addMonitoringPacket(unsigned int pomID, unsigned int hits[30], 
										float temp, float humidity) {
	// Search for this pomID in the config CLB IDs
	std::vector<unsigned int>::iterator it = std::find(fCLB_eids.begin(), fCLB_eids.end(), pomID);

	if (it != fCLB_eids.end()) {
		int clbIndex = std::distance(fCLB_eids.begin(), it);

		// This is a valid CLB. Therefore, add the hits...
		for (int channel = 0; channel < 30; channel ++) fRate_array[clbIndex][channel] += hits[channel];

		// Increment the number of packets received for this clb in this update window
		fPacket_array[clbIndex] += 1;

		// Temp/Humidity label variables
		fAvg_temp = (0.001*temp) + ((1-0.001)*fAvg_temp);
		fAvg_humidity = (0.001*humidity) + ((1-0.001)*fAvg_humidity);
		if (temp<fLow_temp) fLow_temp = temp;
		if (temp>fHigh_temp) fHigh_temp = temp;
		if (humidity<fLow_humidity) fLow_humidity = humidity;
		if (humidity>fHigh_humidity) fHigh_humidity = humidity;

	} else {
		fNon_config_data = true;
	}
}

void MonitoringGui::setupArrays() {
	fRate_array.clear();
	fPacket_array.clear();

	// Add a clean channel vector into the rate array for each CLB
	for (int clbNum = 0; clbNum<fNum_clbs; clbNum++) {
		std::vector<unsigned int> channelVec(PMTSPERPOM);
		fRate_array.push_back(channelVec);
		fPacket_array.push_back(0);
	}

	resetArrays();
}

void MonitoringGui::setupPlots() {

	// Make the individual total plots
	fTotal_rate_plot = makeTotalRatePlot();
	fPacket_rate_plot = makePacketRatePlot();
	fRate_map_plot = makeHeatMapPlot();

	for (int clbNum = 0; clbNum<fNum_clbs; clbNum++) {

		// Make the CLB specific packet plots
		fCLB_packet_plots.push_back(makePacketRatePlot(fCLB_eids[clbNum]));

		// Make the channel specific rate plots
		for (int channel=0; channel<PMTSPERPOM; channel++) {
			fChannel_rate_plots.push_back(makeTotalRatePlot(fCLB_eids[clbNum], channel));
		}
	}
}

void MonitoringGui::resetArrays() {
	for (int clb_num = 0; clb_num<fNum_clbs; clb_num++) {
		fPacket_array[clb_num] = 0;
		for(int channel = 0; channel<PMTSPERPOM; channel++) {
			fRate_array[clb_num][channel] = 0;
		}
	}
}

void MonitoringGui::updatePlots() {
	fNum_updates++;

	if (fMode) { fRun_updates ++; }

	// Do we need to clear the plots that have reached PLOTLENGTH and start again?
	if ((fNum_updates % PLOTLENGTH) == 0) { refreshPlots(); }

	// Need variables to track things...
	fActive_clbs 				= 0;
	fActive_channels			= 0;
	fOdd_channels 				= 0;

	int 	total_hits 			= 0;
	int 	total_packets		= 0;
	bool 	clbHits[fNum_clbs];

	// We need to loop through the fRate_array and update the plots and then clear it...
	for (int pomIndex = 0; pomIndex < fNum_clbs; pomIndex++) {
		clbHits[pomIndex] = false;

		int clb_packets = fPacket_array[pomIndex];
		total_packets += clb_packets;
		fCLB_packet_plots[pomIndex]->SetBinContent(fNum_updates-(fNum_refresh*PLOTLENGTH), clb_packets / ((float)fUpdate_rate/1000));

		for (int channelIndex = 0; channelIndex < PMTSPERPOM; channelIndex++) {
			int hits = (int)fRate_array[pomIndex][channelIndex];

			// Add to the total hits for this window
			total_hits += hits;

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
	}

	// Reset the arrays for the next window
	resetArrays();

	for (int clb = 0; clb<fNum_clbs; clb++) {
		if (clbHits[clb] == true) { fActive_clbs++; }
	}

	// Set the total channel rate plot
	fTotal_rate_plot->SetBinContent(fNum_updates-(fNum_refresh*PLOTLENGTH), (float)total_hits / ((float)fUpdate_rate/1000));

	// Set the total packet rate plot
	float total_packet_rate =  total_packets / ((float)fUpdate_rate/1000);
	fPacket_rate_plot->SetBinContent(fNum_updates-(fNum_refresh*PLOTLENGTH), total_packet_rate);
	fAvg_packet_rate = (0.2*total_packet_rate) + ((1-0.2)*fAvg_packet_rate);
	if (total_packet_rate<fLow_packet_rate) fLow_packet_rate = total_packet_rate;
	if (total_packet_rate>fHigh_packet_rate) fHigh_packet_rate = total_packet_rate;
}

void MonitoringGui::refreshPlots() {
	fNum_refresh++;

	// Clear the temp/humidity clb plots and individual channel hit plots
	for(int clb = 0; clb < fNum_clbs; clb++) {

		delete fCLB_packet_plots[clb];
		fCLB_packet_plots[clb] = NULL;
		fCLB_packet_plots[clb] = makePacketRatePlot(fCLB_eids[clb]);

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
}

void MonitoringGui::draw() {
	drawPlots();
	drawLabels();
	drawDirectionButtons();
}

void MonitoringGui::drawPlots() {
	// Canvas 1 (Total rate plot or channel specific rate plot)
	fCanvas_1->cd();
	if (fSpecify_button->IsDown()) {
		fChannel_rate_plots[((int)fPom_id_entry->GetNumber()*PMTSPERPOM) + ((int)fChannel_entry->GetNumber())]->Draw();
	} else { fTotal_rate_plot->Draw(); }
	fCanvas_1->Update();

	// Canvas 2 (Total packet plot or CLB specific packet plot)
	fCanvas_2->cd();
	if (fSpecify_button->IsDown()) { fCLB_packet_plots[(int)fPom_id_entry->GetNumber()]->Draw(); }
	else { fPacket_rate_plot->Draw(); }
	fCanvas_2->Update();	

	// Canvas 3 (Rate heat map)
	fCanvas_3->cd();
	fRate_map_plot->Draw("COLZ");
	fCanvas_3->Update();
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

	} else if (fPage_num ==1) {
		fact1Labeltext = "Avg Temp: ";
		fact1Labeltext += fAvg_temp;
		fFact_label_1->SetBackgroundColor(TColor::Number2Pixel(38));
	} else {
		fact1Labeltext = "Avg Packet Rate: ";
		fact1Labeltext += fAvg_packet_rate;
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

	} else if (fPage_num ==1) {
		fact2Labeltext = "Avg Humidity: ";
		fact2Labeltext += fAvg_humidity;
		fFact_label_2->SetBackgroundColor(TColor::Number2Pixel(38));
	} else {
		fact2Labeltext = "Avg Seq Num: ";
		fact2Labeltext += fAvg_seq_num;
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

	} else if (fPage_num ==1) {
		fact3Labeltext = "Low/High: ";
		fact3Labeltext += fLow_temp;
		fact3Labeltext += "/";
		fact3Labeltext += fHigh_temp;
		fFact_label_3->SetBackgroundColor(TColor::Number2Pixel(38));
	} else {
		fact3Labeltext = "Low/High: ";
		fact3Labeltext += fLow_packet_rate;
		fact3Labeltext += "/";
		fact3Labeltext += fHigh_packet_rate;
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

	} else if (fPage_num ==1) {
		fact4Labeltext = "Low/High: ";
		fact4Labeltext += fLow_humidity;
		fact4Labeltext += "/";
		fact4Labeltext += fHigh_humidity;
		fFact_label_4->SetBackgroundColor(TColor::Number2Pixel(38));
	} else {
		fact4Labeltext = "Low/High: ";
		fact4Labeltext += fLow_seq_num;
		fact4Labeltext += "/";
		fact4Labeltext += fHigh_seq_num;
		fFact_label_4->SetBackgroundColor(TColor::Number2Pixel(38));
	}
	fFact_label_4->SetText(fact4Labeltext);
}

void MonitoringGui::drawDirectionButtons() {
	if (fPage_num == 0) { 
		fBack_button->SetText("<--- Packet Info");
		fForward_button->SetText("Temp/Humidity --->");
	} else if (fPage_num == 1) {
		fBack_button->SetText("<--- Hit Rates");
		fForward_button->SetText("Packet Info --->");		
	} else if (fPage_num == 2) {
		fBack_button->SetText("<--- Temp/Humidity");
		fForward_button->SetText("Hit Rates --->");		
	} else { std::cout << "DAQonite - Error: Wrong GUI page number!" << std::endl; }
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

TH1F* MonitoringGui::makePacketRatePlot(unsigned int pomIndex) {
	TString plotName = "PacketRatePlot_";
	plotName += pomIndex;

	TH1F* packetRatePlot = new TH1F(plotName, plotName, PLOTLENGTH, 0, PLOTLENGTH);
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

void MonitoringGui::toggleSpecific() {
	draw();
}

void MonitoringGui::pageBackward() {
	if (fPage_num == 0) { fPage_num = 2; }
	else if (fPage_num == 1) { fPage_num = 0; }
	else if (fPage_num == 2) { fPage_num = 1; }
	else { std::cout << "DAQonite - Error: Wrong GUI page number!" << std::endl; }
}

void MonitoringGui::pageForward() {
	if (fPage_num == 0) { fPage_num = 1; }
	else if (fPage_num == 1) { fPage_num = 2; }
	else if (fPage_num == 2) { fPage_num = 0; }
	else { std::cout << "DAQonite - Error: Wrong GUI page number!" << std::endl; }
}

void MonitoringGui::startRun(unsigned int type, unsigned int run, TString fileName) {
	fMode = true;

	fRun_num = run;
	fRun_type = type;
	fRun_total_packets = 0;
	fRun_file = fileName;	
	fRun_updates = 0;

	// Reset packet variables
	fAvg_packet_rate = 0;
	fAvg_seq_num = 0;
	fLow_packet_rate = 10000;
	fHigh_packet_rate = 0;
	fLow_seq_num = 10;
	fHigh_seq_num = 0;
}

void MonitoringGui::stopRun() {
	fMode = false;

	// Reset the run variables
	fRun_num = -1;
	fRun_type = -1;
	fRun_total_packets = 0;
	fRun_file = "";
	fRun_updates = 0;

	// Reset packet variables
	fAvg_packet_rate = 0;
	fAvg_seq_num = 0;
	fLow_packet_rate = 10000;
	fHigh_packet_rate = 0;
	fLow_seq_num = 10;
	fHigh_seq_num = 0;
}

void MonitoringGui::update() {
	updatePlots();
	draw();
}