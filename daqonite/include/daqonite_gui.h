/*
 * daqonite_gui.h
 * Test at a ROOT GUI for DAQonite
 *
 *  Created on: Sep 27, 2018
 *      Author: Josh Tingey
 *       Email: j.tingey.16@ucl.ac.uk
 */

#ifndef DAQONITE_GUI_H_
#define DAQONITE_GUI_H_

#include <TQObject.h>
#include <RQ_OBJECT.h>

#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>
#include <TG3DLine.h>

#include "TH2F.h"
#include "TCanvas.h"
#include "TF1.h"
#include <TRandom.h>
#include <TROOT.h>
#include <TStyle.h>
#include "TColor.h"
#include "TImage.h"

class TGWindow;
class TGMainFrame;
class TRootEmbeddedCanvas;
class TFile;
class TTree;

class DAQoniteGUI {
	RQ_OBJECT("DAQoniteGUI");
	public:
		DAQoniteGUI(const TGWindow*p, UInt_t w, UInt_t h);
		virtual ~DAQoniteGUI();

		void addHits(unsigned int pomID, unsigned int channel, unsigned int hits);
		void addHeader(UInt_t pomID, UInt_t time_ms);

		void toggleSpecific();
		void pageBackward();
		void pageForward();

		void startRun(unsigned int type, unsigned int run, TString fileName);
		void stopRun();

	private:

		void addPom(unsigned int pomID, unsigned int pomIndex);
		void clearPomRates(unsigned int pomIndex);

		void updatePlots(); 	// Updates and fills all active plots
		void modifyPlots(); 	// Modifies plots if needed
		void refreshPlots();	// When we get to PLOTLENGTH clear all plots

		void drawPlots(); 		// Draws the appropriate plots to the canvases
		void drawLabels();	// Update the labels and change their colour status

		TH1F* makeTotalRatePlot(unsigned int pomID, unsigned int channel);
		TH1F* makeTotalRatePlot();
		TH1F* makePacketRatePlot();
		TH2F* makeHeatMapPlot();

		// The main frame
		TGMainFrame*		fMainFrame;

		TCanvas*			fCanvas1;
		TCanvas*			fCanvas2;
		TCanvas*			fCanvas3;

		TGLabel*			fRunStatusLabel;
		TGLabel*			fRunTypeLabel;
		TGLabel*			fRunNumLabel;
		TGLabel*			fRunTimeLabel;
		TGLabel*			fRunPacketsLabel;
		TGLabel*			fRunFileLabel;

		TGTextButton* 		fBackButton;
		TGTextButton* 		fForwardButton;

		TGNumberEntry*		fPomIDEntry;
		TGNumberEntry*		fChannelEntry;
		TGCheckButton*		fSpecifyButton;

		TGLabel*			fFactLabel1;
		TGLabel*			fFactLabel2;
		TGLabel*			fFactLabel3;
		TGLabel*			fFactLabel4;

		TH1F*				fTotalRatePlot;
		std::vector<TH1F*> 	fChannelRatePlots;
		TH1F*				fPacketRatePlot;
		TH2F*				fRateHeatMapPlot;	

		int 				fPageNum;

		// Total monitoring values
		UInt_t 				fPacketsReceived;
		UInt_t 				fNumUpdates;
		UInt_t				fNumRefresh;
		bool 				fRunning;

		UInt_t				fRunNumber;
		UInt_t				fStartTime;
		UInt_t				fRunType;
		UInt_t				fActiveChannels;
		UInt_t				fOddChannels;
		TString				fRunFile;

		// Window monitoring values
		bool 				fModifyPlots;
		UInt_t 				fWindowPackets;
		UInt_t 				fStartPomID;
		UInt_t 				fStartTime_ms;		

		// Storage Vectors
		std::vector<unsigned int> fActivePOMs;
		std::vector< std::vector<unsigned int> > fRateArray;
};

#endif /* DAQONITE_GUI_H_ */
