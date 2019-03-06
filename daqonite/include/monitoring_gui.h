/**
 * MonitoringGui - The ROOT monitoring GUI for DAQonite
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

#include "monitoring_config.h"

class TGWindow;
class TGMainFrame;
class TRootEmbeddedCanvas;
class TFile;
class TTree;

class MonitoringGui {

	RQ_OBJECT("MonitoringGui");

	public:

		/** 
		 * Create a MonitoringGui object
		 * Creates the monitoring GUI object and builds the ROOT TApplication GUI.
		 */		
		MonitoringGui(int updateRate, std::string configFile);

		/// Destroys the MonitoringGui object
		virtual ~MonitoringGui();

		/** 
		 * Adds a optical packet from a clb to the monitoring
		 * 
		 * @param pomID The POM ID of the packet
		 * @param seqNumber Packet sequence number
		 */	
		void addOpticalPacket(unsigned int pomID, unsigned int seqNumber);		

		/** 
		 * Adds a monitoring packet from a clb to the monitoring
		 * 
		 * @param pomID The POM ID of the packet
		 * @param hits Array holding the number of hits on each channel
		 * @param temp CLB temperature in Celsius
		 * @param humidity CLB humidity in RH
		 */	
		void addMonitoringPacket(unsigned int pomID, unsigned int hits[30], 
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

		/** 
		 * Updates everything and then draws to canvases
		 */			
		void update();		

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
		 * Resets the hit and packet arrays for the next window
		 */			
		void resetArrays();

		/** 
		 * Update all the monitoring variables ready for them to be drawn to the GUI display
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

		/// Draws the most recent version of everything, includes a lock on this process
		void draw();

		/// Draws the appropriate plots to the canvases	
		void drawPlots();

		/// Updates the labels with new values
		void drawLabels();

		/// Update buttons when clicked
		void drawDirectionButtons();

		// ROOT Hist plot makers
		TH1F* makeTotalRatePlot(unsigned int pomIndex, unsigned int channel);
		TH1F* makeTotalRatePlot();
		TH1F* makePacketRatePlot(unsigned int pomIndex);
		TH1F* makePacketRatePlot();
		TH2F* makeHeatMapPlot();

		// The main frame
		TGMainFrame*		fMain_frame;		///< The ROOT GUI main frame that holds everything

		TCanvas*			fCanvas_1;			///< The top canvas in the GUI
		TCanvas*			fCanvas_2;			///< The middle canvas in the GUI
		TCanvas*			fCanvas_3;			///< The bottom canvas in the GUI

		TGLabel*			fRun_status_label;	///< The run status label in the GUI
		TGLabel*			fRun_type_label;	///< The run type label in the GUI
		TGLabel*			fRun_num_label;		///< The run number label in the GUI
		TGLabel*			fRun_time_label;	///< The elapsed run time label in the GUI
		TGLabel*			fRun_packets_label;	///< The collected monitoring packets label in the GUI
		TGLabel*			fRun_file_label;	///< The save file name label in the GUI

		TGTextButton* 		fBack_button;		///< The 'Backwards' button in the GUI
		TGTextButton* 		fForward_button;	///< The 'Forwards' button in the GUI

		TGNumberEntry*		fPom_id_entry;		///< The POM ID number selection in the GUI
		TGNumberEntry*		fChannel_entry;		///< The Channel ID number selection in the GUI
		TGCheckButton*		fSpecify_button;	///< The toggle specific button in the GUI

		TGLabel*			fFact_label_1;		///< Fact 1 label in the GUI
		TGLabel*			fFact_label_2;		///< Fact 2 label in the GUI
		TGLabel*			fFact_label_3;		///< Fact 3 label in the GUI
		TGLabel*			fFact_label_4;		///< Fact 4 label in the GUI

		// Hit rate plots and hit array that keeps track of everything in the window
		TH1F*				fTotal_rate_plot;					///< Total rate plot across all channels
		TH2F*				fRate_map_plot;						///< Heat map of hit rates across all channels
		std::vector<TH1F*> 	fChannel_rate_plots;				///< Individual channel rate plots
		std::vector<std::vector<unsigned int> > fRate_array;	///< Array holding number of hits in window

		// Packet rate plot and the packet array that keeps track of everything in the window
		TH1F*				fPacket_rate_plot;					///< Total packet rate plot across all channels
		std::vector<TH1F*>	fCLB_packet_plots;					///< Individual CLB received optical packet plots
		std::vector<unsigned int> fPacket_array;				///< Array holding the number of packets per CLB in this window

		// Important variables
		int 				fPage_num;			///< Current page being displayed in the GUI
		bool 				fMode;				///< false = Monitoring, True = Running
		int 				fUpdate_rate;		///< The rate the plots will be updated
		int 				fNum_updates;		///< Number of GUI updates, used for refreshing the plots
		int					fNum_refresh;		///< Number of times plots have been refreshed, used for refreshing the plots

		// Run Variables
		int					fRun_num;			///< The current run number
		int					fRun_type;			///< The current run type
		unsigned int 		fRun_total_packets;	///< Total number of optical and monitoring packets in the run
		TString				fRun_file;			///< The output .root file for this run
		int 				fRun_updates;		///< Number of updates in this run

		// Hit label variables
		int					fActive_clbs;		///< The number of CLBs we have received hits from
		int					fActive_channels;	///< The number of channels we have received hits from
		int					fOdd_channels;		///< The number of channels that are behaving oddly
		bool 				fNon_config_data;	///< Are we receiving data from non-config CLBs?

		// Packet label variables
		float				fAvg_packet_rate;	///< Rolling accumulator average of total packet rate
		float 				fAvg_seq_num;		///< Rolling accumulator average of sequence num (run only)
		float				fLow_packet_rate;	///< Lowest packet rate
		float				fHigh_packet_rate; 	///< Highest packet rate
		unsigned int 		fLow_seq_num;		///< Lowest sequence number (run only)
		unsigned int 		fHigh_seq_num;		///< Highest sequence number (run only)

		// Temp/Humidity label variables
		float				fAvg_temp;			///< Rolling accumulator average of temperature
		float 				fAvg_humidity;		///< Rolling accumulator average of humidity
		float				fLow_temp;			///< Lowest temp
		float				fHigh_temp; 		///< Highest temp
		float 				fLow_humidity;		///< Lowest humidity
		float				fHigh_humidity;		///< Highest humidity

		// Configuration variables
		std::string 					fConfig_file;		///< Configuration file
		int 							fNum_clbs;			///< Number of CLBs from "clb_number"
		std::vector<unsigned int> 		fCLB_eids; 			///< eIDs of the CLBs
		int 							fTotal_num_channels;///< Total number of active channels
};

#endif
