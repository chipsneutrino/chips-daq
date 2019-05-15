/**
 * DAQConfig - Reads the config file for use in the DAQ
 * 
 * We can then make checks against the real data to flag up problems
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef DAQ_CONFIG_H_
#define DAQ_CONFIG_H_

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <bitset>
#include <fstream>
#include <stdexcept>
#include <iomanip>

class DAQConfig {
    public:

		/// Create a DAQConfig
        DAQConfig(const char * config);

        /// Destroy a DAQConfig
        ~DAQConfig();

        /// Print the summary of the DAQConfig
        void printConfig();

		// Return fConf_name
		std::string getConfigName() {
			return fConf_name;
		}

		/// Returns fNum_clbs
		int getNumCLBs() {
			return fNum_clbs;
		}

		/// Returns fEnabled_clbs
		int getNumEnabledCLBs() {
			return fEnabled_clbs;
		}

		/// Returns fEnabled_channels
		int getNumEnabledChannels() {
			return fEnabled_channels;
		}

		/// Returns fCLB_enabled
		std::vector<bool> getEnabledCLBs() {
			return fCLB_enabled;
		}	

		/// Returns fCLB_eids
		std::vector<unsigned int> getCLBeIDs() {
			return fCLB_eids;
		}	

		/// Returns fCLB_channels
		std::vector< std::bitset<32> > getActiveChannels() {
			return fCLB_channels;
		}

		/// Returns fCLB_channel_eids
		std::vector< std::array<unsigned int, 31> > getChanneleIDs() {
			return fCLB_channel_eids;
		}

    private:
		/// Read the configuration text file specified by fConf_name
		void loadConfig();

		/**
		 * Parse the line from the file, filling the vectors with found info
		 * @param line String to parse
		 */
		void parseLine(std::string &line);

		/**
		 * Strip out comments preceded by // or # from the text being parsed
		 * @param line String to remove comments from
		 */
		void ignoreComments(std::string &line);

		/**
		 * Erase all the whitespace from the line
		 * @param line String to remove whitespace from
		 */
		void removeWhitespace(std::string &line);

		/**
		 * Check whether a line could be trying to set a value
		 * @param line Line to check
		 * @return True if str if of the form "TEXT1=TEXT2" (i.e. an = sign and text either side)
		 */
		bool isGoodLine(std::string line);

		/**
		 * Extract the details from the line
		 * @param line String to extract info from
		 * @param numDots Number of dots found in the string
		 * @param clbNum CLB number found from string
		 * @param channelNum Channel number found from string
		 * @param config Config setting found from string
		 * @param value Value of config setting found from string
		 */		
		void extractConfig(std::string line, int &numDots, int &clbNum, int &channelNum, 
						   std::string &config, std::string &value);

		// Setup all the vectors given the number of CLBs in the config file
		void setupVectors();

		std::string fConf_name;                   						///< Path of a dat file containing all the configuration parameters

		int fNum_clbs;													///< Total number of CLBs from "clb_number"
		int fEnabled_clbs;												///< Total number of "enabled" CLBs
		int fEnabled_channels;											///< Total number of "enabled" channels

		std::vector<bool> fCLB_enabled;									///< Is the CLB enabled
		std::vector<unsigned int> fCLB_eids; 							///< eIDs of the CLBs
		std::vector< std::bitset<32> > fCLB_channels;					///< Which channels are active
		std::vector< std::array<unsigned int, 31> > fCLB_channel_eids;	///< eIDs for all the CLB channels 
};

#endif