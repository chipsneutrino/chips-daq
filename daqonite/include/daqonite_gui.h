/*
 * DAQoniteGUI.h
 * Test at a ROOT GUI for DAQonite
 *
 *  Created on: Sep 27, 2018
 *      Author: Josh Tingey
 *       Email: j.tingey.16@ucl.ac.uk
 */

#ifndef DAQONITEGUI_H_
#define DAQONITEGUI_H_

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

		void ToggleMining();
		void Setup();

		void SetCLBOptPort();
		void SetCLBMonPort();
		void SetBBBPort();

	private:

		TGMainFrame 		*fMain;
		TRootEmbeddedCanvas *fEcanvas;

		TGCheckButton 		*fButton_clb_optical;
		TGNumberEntry       *fPort_clb_optical;
		TGCheckButton 		*fButton_clb_monitoring;
		TGNumberEntry       *fPort_clb_monitoring;

		TGCheckButton 		*fButton_bbb_optical;
		TGCheckButton 		*fButton_bbb_monitoring;
		TGNumberEntry       *fPort_bbb;

		TGCheckButton 		*fSaveDataButton;
		TGTextEntry 		*fFileNameEntry;

		TGTextButton		*fMiningButton;
		bool 				fMining;
		TGTextButton		*fExit;

		unsigned int 		optPort;
		unsigned int 		monPort;
		unsigned int 		bbbPort;

};

#endif /* DAQONITEGUI_H_ */
