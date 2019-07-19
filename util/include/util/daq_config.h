/**
 * DAQConfig - Reads the config file for use in the DAQ
 * 
 * We can then make checks against the real data to flag up problems
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#pragma once

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <bitset>
#include <fstream>
#include <stdexcept>

// Enum for the different types of controller
enum ControllerType {CLB, BBB};

struct ControllerConfig {
	// Controller wide variables
	ControllerType type_		= CLB;			///< What type of controller is it?
	bool enabled_ 				= false;		///< Is the Controller enabled
	unsigned int eid_ 			= 0;			///< Controller electronic ID
	unsigned int ip_ 			= 0;			///< Controller IP address on the DAQ network			
	unsigned int server_ip_ 	= 3232238337;	///< The DAQ server IP address (192.168.11.1)
	unsigned int window_dur_ 	= 1000;			///< Duration of the controller reporting window (ms)
	bool nano_enabled_			= false;		///< Should the nanobeacon be enabled?
	int nano_voltage_			= 0.0;			///< The nanobeacon voltage in mv

	// Channel specific variables
	std::bitset<32> chan_enabled_;				///< Is the channel enabled
	unsigned int chan_eid_[31] 	= {};			///< The channel electronic ID
	unsigned int chan_hv_[31] 	= {};			///< Channel high voltage setting
};

class DAQConfig {
public:

	/// Create a DAQConfig
	DAQConfig(const char * config);

	/// Destroy a DAQConfig
	~DAQConfig();

	/// Print the summary of the DAQConfig
	void printConfig();

	/// Print a short summary of the DAQConfig
	void printShortConfig();

	std::string conf_name_;                 ///< Path of a .dat file containing all the configuration parameters

	int num_controllers_;					///< Total number of controller from "clb_number"
	int enabled_controllers_;				///< Total number of "enabled" controllers
	int enabled_channels_;					///< Total number of "enabled" channels
	bool is_nano_enabled_;					///< Do any of the controllers have an enabled nanobeacon?

	std::vector<ControllerConfig> configs_;	///< Vector of controller configs

private:
	/// Read the configuration text file specified by conf_name_
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
	 * @param controllerNum controller number found from string
	 * @param channelNum Channel number found from string
	 * @param config Config setting found from string
	 * @param value Value of config setting found from string
	 */		
	void extractConfig(std::string line, int &numDots, int &controllerNum, int &channelNum, 
						std::string &config, std::string &value);

	/// Setup all the vectors given the number of controllers in the config file
	void setupVectors();
};
