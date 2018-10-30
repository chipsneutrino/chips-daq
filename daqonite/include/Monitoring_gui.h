/**
 * Monitoring_gui - The ROOT monitoring GUI for DAQonite
 * 
 * This class provides a ROOT GUI implementation for monitoring the data streams.
 * It displays various monitoring plots and allows you to look at specific channels.
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef MONITORING_GUI_H_
#define MONITORING_GUI_H_

#include <TQObject.h>
#include <RQ_OBJECT.h>
#include <TApplication.h>
#include <TGClient.h>
#include <TROOT.h>

#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>
#include <TG3DLine.h>

#include "TF1.h"
#include "TH2F.h"
#include "TCanvas.h"
#include <TStyle.h>
#include "TColor.h"
#include "TImage.h"

#include <bitset>

#include "Monitoring_config.h"

class TGWindow;
class TGMainFrame;
class TRootEmbeddedCanvas;
class TFile;
class TTree;

class Monitoring_gui {

	RQ_OBJECT("Monitoring_gui");

	public:

		/** 
		 * Create a Monitoring_gui object
		 * Creates the monitoring GUI object and builds the ROOT TApplication GUI.
		 */		
		Monitoring_gui();

		/// Destroys the Monitoring_gui object
		virtual ~Monitoring_gui();

		/** 
		 * Adds CLB optical hits to be included in monitoring
		 * Adds hits for a specific POM and channel to be included in the monitoring,
		 * if hits have not yet been received from this POM, the POM is added.
		 * 
		 * @param pomID The POM ID of the hits
		 * @param channel The channel ID of the hits
		 * @param hits The number of hits
		 */	
		void addHits(unsigned int pomID, unsigned int channel, unsigned int hits);

		/** 
		 * Adds a CLB header
		 * Adds a header showing a montitoring packet has been received. This includes the
		 * time information that is used in the monitoring GUI.
		 * 
		 * @param pomID The POM ID
		 * @param time_ms The timestamp in ms for the monitoring packet header
		 */	
		void addHeader(unsigned int pomID, unsigned int time_ms);

		/** 
		 * Toggles the drawing of POM/Channel specific plots
		 * This is called when the fSpecifyButton is Clicked() in the GUI
		 */	
		void toggleSpecific();

		/** 
		 * Changes the current page 'backwards' 
		 * This is called when the fBackButton is Clicked() in the GUI
		 */	
		void pageBackward();

		/** 
		 * Changes the current page 'forwards' 
		 * This is called when the fForwardButton is Clicked() in the GUI
		 */	
		void pageForward();

		/** 
		 * Modifies the GUI to display that a new run has started
		 * This is called when startRun() is called in the DAQ_handler. It sets everything
		 * to display correctly for the new run given the input parameters.
		 * 
		 * @param type The type of run
		 * @param run The run number
		 * @param fileName The output filename
		 */	
		void startRun(unsigned int type, unsigned int run, TString fileName);

		/** 
		 * Stops the current run 
		 * Resets everything and modifies the GUI for a non-running state
		 */			
		void stopRun();

	private:

		/** 
		 * Setup the monitoring arrays from the configuration variables
		 */		
		void setupArrays();

		/** 
		 * Setup the plots from the configuration variables
		 */		
		void setupPlots();

		/** 
		 * Clears the hit array for a specific POM
		 * Clears the fRateArray for a specific POM, used at the end of monitoring windows
		 * to reset the hit counts on all channels for the next monitoring window.
		 * 
		 * @param pomIndex The POM index in fRateArray to be cleared
		 */			
		void clearPomRates(unsigned int pomIndex);

		/** 
		 * Update the plots
		 * This fills the plots with the contents of fRateArray, calculating all the bits
		 * we need
		 */		
		void updatePlots();

		/** 
		 * When plots are full empty them
		 * Deletes and created new plots that have reached PLOTLENGTH in order to keep 
		 * showing new monitoring data
		 */			
		void refreshPlots();

		/// Draws the appropriate plots to the canvases	
		void drawPlots(); 		// Draws the appropriate plots to the canvases

		/// Updates the labels with new values
		void drawLabels();	// Update the labels and change their colour status

		/// Update buttons when clicked
		void drawDirectionButtons();

		// Draw the CHIPS logo on all canvases
		void drawLogo();

		// ROOT Hist plot makers
		TH1F* makeTotalRatePlot(unsigned int pomID, unsigned int channel);
		TH1F* makeTotalRatePlot();
		TH1F* makePacketRatePlot();
		TH2F* makeHeatMapPlot();

		// The main frame
		TGMainFrame*		fMainFrame;			///< The ROOT GUI main frame that holds everything

		TCanvas*			fCanvas1;			///< The top canvas in the GUI
		TCanvas*			fCanvas2;			///< The middle canvas in the GUI
		TCanvas*			fCanvas3;			///< The bottom canvas in the GUI

		TGLabel*			fRunStatusLabel;	///< The run status label in the GUI
		TGLabel*			fRunTypeLabel;		///< The run type label in the GUI
		TGLabel*			fRunNumLabel;		///< The run number label in the GUI
		TGLabel*			fRunTimeLabel;		///< The elapsed run time label in the GUI
		TGLabel*			fRunPacketsLabel;	///< The collected monitoring packets label in the GUI
		TGLabel*			fRunFileLabel;		///< The save file name label in the GUI

		TGTextButton* 		fBackButton;		///< The 'Backwards' button in the GUI
		TGTextButton* 		fForwardButton;		///< The 'Forwards' button in the GUI

		TGNumberEntry*		fPomIDEntry;		///< The POM ID number selection in the GUI
		TGNumberEntry*		fChannelEntry;		///< The Channel ID number selection in the GUI
		TGCheckButton*		fSpecifyButton;		///< The toggle specific button in the GUI

		TGLabel*			fFactLabel1;		///< Fact 1 label in the GUI
		TGLabel*			fFactLabel2;		///< Fact 2 label in the GUI
		TGLabel*			fFactLabel3;		///< Fact 3 label in the GUI
		TGLabel*			fFactLabel4;		///< Fact 4 label in the GUI

		TH1F*				fTotalRatePlot;		///< Total rate plot across all channels
		std::vector<TH1F*> 	fChannelRatePlots;	///< Individual channel rate plots
		TH1F*				fPacketRatePlot;	///< Total packet rate plot across all channels
		TH2F*				fRateHeatMapPlot;	///< Heat map of hit rates across all channels

		int 				fPageNum;			///< Current page being displayed in the GUI

		// Total monitoring values
		int 				fPacketsReceived;	///< Number of monitoring packets received
		int 				fNumUpdates;		///< Number of GUI updates
		int					fNumRefresh;		///< Number of times plots have been refreshed
		bool 				fRunning;			///< Is data collection happening?

		int					fRunNumber;			///< The current run number
		int					fStartTime;			///< The current run start time
		int					fRunType;			///< The current run type
		int					fActiveCLBs;		///< The number of CLBs we have received hits from
		int					fActiveChannels;	///< The number of channels we have received hits from
		int					fOddChannels;		///< The number of channels that are behaving oddly
		TString				fRunFile;			///< The output .root file for this run

		// Window monitoring values
		bool 				fModifyPlots;		///< Do we need to modify the plots?
		int 				fWindowPackets;		///< The number of monitoring packets in this window
		unsigned int 		fStartPomID;		///< The first POM ID
		unsigned int 		fStartTime_ms;		///< The first start time in ms
		bool 				fNonConfigData;		///< Are we receiving data from non-config CLBs?

		// Configuration variables
		int fNumCLBs;										///< Number of CLBs from "clb_number"
		std::vector<unsigned int> fCLBeIDs; 				///< eIDs of the CLBs
		std::vector<unsigned int> fCLBTypes;				///< Plane types for the CLBs

		int fTotalNumChannels;								///< Total number of active channels
		std::vector<std::bitset<32> > fActiveChannelsArr;	///< Which channels are active

		std::vector<std::vector<unsigned int> > fRateArray;	///< Array holding number of hits in window
};

#endif
