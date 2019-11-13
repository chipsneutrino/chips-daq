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
#include <boost/asio.hpp>

#include <util/elastic_interface.h>

// Enum for the different types of controller
enum ControllerType {CLB, BBB};

// Enum for the different types of relay
enum RelayType {MC, EC, DANOUT};

struct ControllerConfig {
	// General variables
	std::string config_name_	= "";		///< Name of config file

	// Controller wide variables
	bool enabled_				= false;		///< Is it enabled?
	bool hv_enabled_			= true;			///< Should the HV be enabled?
	ControllerType type_		= CLB;			///< Electronic type
	int eid_					= 0;			///< Electronic ID
	long mac_					= 0;			///< MAC address
	long ip_					= 0;			///< IP address
	int port_					= 56014;		///< Slow-control port (default CLB port)
	bool relay_control_			= false;		///< Should we use the relay to control power?
	RelayType relay_type_		= EC;			///< Type of relay used to power this CLB
	long relay_ip_				= 0;			///< Relay IP address
	int relay_port_				= 0;			///< Relay port
	int relay_chp_				= 0;			///< Relay channel positive
	int relay_chn_				= 0;			///< Relay channel neutral
	unsigned long data_ip_		= 3232241153;	///< Data server IP address (192.168.22.1)
	int data_port_				= 56015;		///< Data server port (CLB optical port)
	int data_window_			= 100;   		///< Data window duration (microseconds)
	bool veto_enabled_			= true;			///< Should we enable the high-rate veto?
	int veto_value_				= 100;			///< Number of hits per timeslice to veto on
	bool nano_enabled_			= false;		///< Is nanobeacon enabled?
	int nano_voltage_			= 0;			///< Nano voltage (mv)
	std::bitset<32> ch_enabled_;				///< Enabled channels (default is all disabled)

	// Channel specific variables
	unsigned ch_id_[31]			= {};			///< Channel electronic ID
	int ch_hv_[31] 				= {};			///< Channel high voltage
	int ch_th_[31] 				= {};			///< Channel threshold 

	ControllerConfig()
	{
		std::fill_n(ch_id_, 31, 0);		// Set all eid's to zero unless specified
		std::fill_n(ch_hv_, 31, 0);		// Set HV values to zero unless specified
		std::fill_n(ch_th_, 31, 43);	// Set all threshold values to the default
	}

	std::string ipAsString() 
	{ 
		return boost::asio::ip::address_v4(ip_).to_string(); 
	}

	std::string relayIpAsString() 
	{ 
		return boost::asio::ip::address_v4(relay_ip_).to_string(); 
	}

	std::string dataIpAsString() 
	{ 
		return boost::asio::ip::address_v4(data_ip_).to_string(); 
	}

	int numEnabledChannels() 
	{ 
		return ch_enabled_.count(); 
	}
};

class DAQConfig {
public:
	/// Create a DAQConfig
	DAQConfig(const char * config);

	/// Destroy a DAQConfig
	~DAQConfig() {};

	/// Print a summary of the DAQConfig
	void printConfig();

	/// Load a new configuration from file
	void loadConfig(const char * config);

	std::string file_name_;                 ///< Configuration file path

	int num_controllers_;					///< Total number of controller from "clb_number"
	int enabled_controllers_;				///< Total number of "enabled" controllers
	int enabled_channels_;					///< Total number of "enabled" channels
	bool is_nano_enabled_;					///< Do any of the controllers have an enabled nanobeacon?

	std::vector<ControllerConfig> configs_;	///< Vector of controller configs

private:
	/// Read the configuration text file specified by file_name_
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

	bool configured_; ///< Has the DAQConfig already been configured from a config file?
};
