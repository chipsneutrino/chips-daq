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
		 * Adds a monitoring packet from a clb to the monitoring
		 * 
		 * @param pomID The POM ID of the hits
		 * @param timeStamp_ms Timestamp in ms for the monitoring packet
		 * @param hits Array holding the number of hits on each channel
		 * @param temp CLB temperature in Celsius
		 * @param humidity CLB humidity in RH
		 */	
		void addMonitoringPacket(unsigned int pomID, unsigned int timeStamp_ms, unsigned int hits[30],
								 float temp, float humidity);		

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
		 * Update all the monitoring variables ready for them to be drawn to the GUI display
		 * This fills the plots with the contents of fRateArray, calculating all the bits
		 * we need
		 */		
		void update();

		/** 
		 * When plots are full empty them
		 * Deletes and created new plots that have reached PLOTLENGTH in order to keep 
		 * showing new monitoring data
		 */			
		void refreshPlots();

		/// Draws the most recent version of everything, includes a lock on this process
		void draw();

		/// Draws the appropriate plots to the canvases	
		void drawPlots();

		/// Updates the labels with new values
		void drawLabels();

		/// Update buttons when clicked
		void drawDirectionButtons();

		// Draw the CHIPS logo on all canvases
		void drawLogo(TCanvas* canvas);

		// ROOT Hist plot makers
		TH1F* makeTotalRatePlot(unsigned int pomIndex, unsigned int channel);
		TH1F* makeTotalRatePlot();
		TH1F* makePacketRatePlot();
		TH2F* makeHeatMapPlot();
		TH1F* makeTemperaturePlot(unsigned int pomIndex);
		TH1F* makeTemperaturePlot();
		TH1F* makeHumidityPlot(unsigned int pomIndex);
		TH1F* makeHumidityPlot();

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

		// Combined Plots
		TH1F*				fTotalRatePlot;		///< Total rate plot across all channels
		TH1F*				fPacketRatePlot;	///< Total packet rate plot across all channels
		TH2F*				fRateHeatMapPlot;	///< Heat map of hit rates across all channels
		TH1F*				fAvgTempPlot;		///< Total packet rate plot across all channels
		TH1F*				fAvgHumidityPlot;	///< Total packet rate plot across all channels

		// Specific Plots
		std::vector<TH1F*> 	fChannelRatePlots;	///< Individual channel rate plots
		std::vector<TH1F*> 	fCLBTempPlots;		///< Individual CLB temperature plots
		std::vector<TH1F*> 	fCLBHumidityPlots;	///< Individual CLB humidity plots

		// Important variables
		int 				fPageNum;			///< Current page being displayed in the GUI
		bool 				fMode;				///< false = Monitoring, True = Running

		// Current run variables
		int					fRunNumber;			///< The current run number
		int					fRunType;			///< The current run type
		unsigned int		fRunStartTime;		///< The current run start time
		int 				fRunPackets;		///< The number of monitoring packets received in the current run
		TString				fRunFile;			///< The output .root file for this run

		// Window variables
		int 				fWindowPackets;		///< The number of monitoring packets in this window
		unsigned int 		fWindowCLBID;		///< The CLB ID used for the windows
		unsigned int 		fWindowStartTime;	///< The start time of the current window in ms

		// Monitoring variables
		int 				fPacketsReceived;	///< Number of monitoring packets received
		int 				fNumUpdates;		///< Number of GUI updates
		int					fNumRefresh;		///< Number of times plots have been refreshed
		int					fActiveCLBs;		///< The number of CLBs we have received hits from
		int					fActiveChannels;	///< The number of channels we have received hits from
		int					fOddChannels;		///< The number of channels that are behaving oddly
		bool 				fNonConfigData;		///< Are we receiving data from non-config CLBs?

		// Configuration variables
		int fNumCLBs;										///< Number of CLBs from "clb_number"
		std::vector<unsigned int> fCLBeIDs; 				///< eIDs of the CLBs
		std::vector<unsigned int> fCLBTypes;				///< Plane types for the CLBs

		int fTotalNumChannels;								///< Total number of active channels
		std::vector<std::bitset<32> > fActiveChannelsArr;	///< Which channels are active

		std::vector<std::vector<unsigned int> > fRateArray;	///< Array holding number of hits in window
		std::vector<float> fTempArray;						///< Array holding the most recent temperature for each CLB
		std::vector<float> fHumidityArray;					///< Array holding the most recent humidity for each CLB
};

#endif
