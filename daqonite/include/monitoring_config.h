/**
 * MonitoringConfig - Reads the config file for use in monitoring
 * 
 * We can then make checks against the real data to flag up problems
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef MONITORING_CONFIG_H_
#define MONITORING_CONFIG_H_

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <bitset>
#include <fstream>

class MonitoringConfig {
    public:
		/**
		 * Create a MonitoringConfig
		 * @param config Path of a .opt file containing all the CLB configuration parameters
		 */
        MonitoringConfig(const char * config);

        /// Destroy a MonitoringConfig
        ~MonitoringConfig();

        /// Print the summary of the config file to stdout
        void printConfig();

		/// Returns the number of CLBs
		int getNumCLBs() {
			return fNum_clbs;
		}

		/// Returns the CLB electronic IDs
		std::vector<unsigned int> getCLBeIDs() {
			return fCLB_eids;
		}

		/// Returns the CLB plane types
		std::vector<unsigned int> getCLBTypes() {
			return fCLB_types;
		}		

		/// Returns the total number of active channels
		int getTotalNumChannels() {
			return fTotal_num_channels;
		}

		std::vector< std::bitset<32> > getActiveChannels() {
			return fActive_channels;
		}

    private:
		/// Read the configuration text file specified by fConf_name
		void loadConfig();

		/**
		 * Strip out comments preceded by // or # from the text being parsed
		 * @param str String to remove comments from
		 */
		void ignoreComments(std::string &str);

		/**
		 * Check whether a line could be trying to set a value
		 * @param str Line to check
		 * @return True if str if of the form "TEXT1=TEXT2" (i.e. an = sign and text either side)
		 */
		const bool isGoodLine(std::string str);

		/**
		 * Check for strings containing only whitespace
		 * @param str String to check
		 * @return True if string is blank or just whitespace
		 */
		const bool isBlankLine(std::string str);

		/**
		 * Extract key and value from configuration file line
		 * @param lhs String to extract the key to
		 * @param rhs String to extract the value to
		 * @param str String in the format "key=value"
		 */
		void extractPair(std::string &lhs, std::string &rhs, std::string str);

		/**
		 * Check if a line is valid, then extract the key and value from it and add
		 * them to fMap
		 * @param str String to parse
		 * @param lineNum Line number in the file
		 */
		void parseLine(std::string str, int lineNum);

		/**
		 * Add a key and value to fMap
		 * @param lhs Key string
		 * @param rhs Value string
		 */
		void addToMap(std::string lhs, std::string rhs);


		/**
		 * Extract the type of configuration line and which CLB/channel it corresponds to.
		 * @param numDots Int to extract the number of dots in the line to
		 * @param clbNum Int to extract the CLB number to
		 * @param channelNum Int to extract the channel number if required to
		 * @param config String to extract the config command to
		 * @param The string to extract the info from
		 */
		void extractConfigType(int &numDots, std::string &clbNum, std::string &channelNum, std::string &config, std::string str);

		/// Iterate through the map and use it to set all the variables it specifies
		void setFromMap();

		std::string fConf_name;                   			///< Path of a dat file containing all the configuration parameters
		std::map<std::string, std::string> fMap;   			///< Map to store key names and their values to set

		int fNum_clbs;										///< Number of CLBs from "clb_number"
		std::vector<unsigned int> fCLB_eids; 				///< eIDs of the CLBs
		std::vector<unsigned int> fCLB_types;				///< Plane types for the CLBs

		int fTotal_num_channels;							///< Total number of active channels
		std::vector< std::bitset<32> > fActive_channels;	///< Which channels are active
};

#endif