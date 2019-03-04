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
		 * @param seqNumber Packet sequence number
		 */	
		void addMonitoringPacket(unsigned int pomID, unsigned int hits[30], 
								 float temp, float humidity, unsigned int seqNumber);		

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
		TH1F* makeRunPacketPlot(unsigned int pomIndex);
		TH1F* makeRunPacketPlot();
		TH1F* makeRunDroppedPlot(unsigned int pomIndex);
		TH1F* makeRunDroppedPlot();

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

		// Combined Plots
		TH1F*				fTotal_rate_plot;	///< Total rate plot across all channels
		TH1F*				fPacket_rate_plot;	///< Total packet rate plot across all channels
		TH2F*				fRate_map_plot;		///< Heat map of hit rates across all channels
		TH1F*				fAvg_temp_plot;		///< Total packet rate plot across all channels
		TH1F*				fAvg_humidity_plot;	///< Total packet rate plot across all channels
		TH1F*				fTotal_packet_plot;	///< Total optical packets received in the current run
		TH1F*				fTotal_dropped_plot;///< Total dropped optical packets in the current run

		// Specific Plots
		std::vector<TH1F*> 	fChannel_rate_plots;///< Individual channel rate plots
		std::vector<TH1F*> 	fCLB_temp_plots;	///< Individual CLB temperature plots
		std::vector<TH1F*> 	fCLB_humidity_plots;///< Individual CLB humidity plots
		std::vector<TH1F*>	fCLB_packet_plots;	///< Individual CLB received optical packet plots
		std::vector<TH1F*>	fCLB_dropped_plots;	///< Individual CLB optical dropped packet plots

		// Important variables
		int 				fPage_num;			///< Current page being displayed in the GUI
		bool 				fMode;				///< false = Monitoring, True = Running

		// Current run variables
		int					fRun_num;			///< The current run number
		int					fRun_type;			///< The current run type
		unsigned int 		fRun_total_packets;	///< Total number of optical and monitoring packets in the run
		unsigned int 		fRun_total_dropped;	///< Total number of dropped packets in the run
		TString				fRun_file;			///< The output .root file for this run

		// Window variables
		int 				fUpdate_rate;		///< The rate the plots will be updated
		int 				fRun_updates;		///< Number of updates in this run
		int 				fUpdate_packets;	///< The number of monitoring packets in this update window

		// Monitoring variables
		int 				fPackets_received;	///< Number of monitoring packets received
		int 				fNum_updates;		///< Number of GUI updates
		int					fNum_refresh;		///< Number of times plots have been refreshed
		int					fActive_clbs;		///< The number of CLBs we have received hits from
		int					fActive_channels;	///< The number of channels we have received hits from
		int					fOdd_channels;		///< The number of channels that are behaving oddly
		bool 				fNon_config_data;	///< Are we receiving data from non-config CLBs?

		std::vector<std::vector<unsigned int> > fRate_array;///< Array holding number of hits in window
		std::vector<float> 				fTemp_array;		///< Array holding the most recent temperature for each CLB
		std::vector<float> 				fHumidity_array;	///< Array holding the most recent humidity for each CLB

		// Packet variables (Only apply to the current run)
		std::vector<unsigned int> 		fOptical_packets;	///< CLB optical packets received
		std::vector<unsigned int>		fOptical_seq;		///< CLB optical packet sequence numbers
		std::vector<unsigned int>		fOptical_dropped;	///< CLB optical packets dropped

		std::vector<unsigned int>		fMonitoring_packets;///< CLB monitoring packets received
		std::vector<unsigned int>		fMonitoring_seq;	///< CLB monitoring packet sequence numbers
		std::vector<unsigned int>		fMonitoring_dropped;///< CLB monitoring packets dropped

		// Configuration variables
		std::string 					fConfig_file;		///< Configuration file
		int 							fNum_clbs;			///< Number of CLBs from "clb_number"
		std::vector<unsigned int> 		fCLB_eids; 			///< eIDs of the CLBs
		int 							fTotal_num_channels;///< Total number of active channels
};

#endif
