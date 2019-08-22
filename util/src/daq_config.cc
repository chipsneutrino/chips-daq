/**
 * DAQConfig - Reads the config file for use in the DAQ
 */

#include "daq_config.h"

/// Create a DAQConfig
DAQConfig::DAQConfig(const char * config) 
	: file_name_(config)
	, configured_(false)
{
	num_controllers_ = 0;
	enabled_controllers_ = 0;
	enabled_channels_ = 0;
	is_nano_enabled_ = false;
	configs_.clear();
	
	loadConfig(); // Load the configuration from file
}
 
/// Print the summary of the DAQConfig
void DAQConfig::printConfig() 
{
	std::cout << "\n~~~~~~~~~ DAQ Config ~~~~~~~~~" << std::endl;
	std::cout << "Plan: " 			<< file_name_ 			<<
				 ", Miners:" 		<< enabled_controllers_ <<
				 ", Pickaxes:"		<< enabled_channels_ 	<< std::endl;
	for (int i = 0; i<num_controllers_; i++) {
		std::cout << "Miner" 		<< i << "(" << configs_[i].enabled_ << ")" << 
					": ID(" 		<< configs_[i].eid_ <<
					"), Type(" 		<< configs_[i].type_ <<
					"), Shaft(" 	<< configs_[i].ipAsString() <<
		 			"), Pickaxes(" 	<< configs_[i].ch_enabled_.count() <<
					"), Headlamp(" 	<< configs_[i].nano_enabled_ << ")" << std::endl;
	}
	std::cout << std::endl;
}

/// Load a new configuration from file
void DAQConfig::loadConfig(const char * config)
{
	// Reset some of the variables that we allow to change
	enabled_controllers_ = 0;
	enabled_channels_ = 0;
	is_nano_enabled_ = false;

	file_name_ = config; // Set the new configuration file name

	loadConfig(); // Load the new configuration from file
}

/// Read the configuration text file specified by file_name_
void DAQConfig::loadConfig() 
{
	std::ifstream input_file;
	input_file.open(file_name_.c_str()); // Open the file

	if (!input_file.is_open()) { // Check it is open
		g_elastic.log(ERROR, "DAQConfig could not open config file!");
		return;
	}

	std::string line;
	while (getline(input_file, line)) { // Read the file line-by-line
		parseLine(line);
	}

	configured_ = true; // Set that this DAQConfig has been configured
	printConfig();	// Print the configuration

	return;
}

/// Parse the line from the file, filling the ControllerConfig's with info
void DAQConfig::parseLine(std::string &line) 
{
	ignoreComments(line); // Remove comments from the line 

	removeWhitespace(line); // Remove whitespace from the line

	if (!(isGoodLine(line))) { return; } // Check if we can use this line

	int num_dots, controller_num, channel_num;
	std::string config, value;

	std::string config_line = line;
	extractConfig(config_line, num_dots, controller_num, channel_num, config, value);

	std::istringstream ss(value); // Stream to convert types
	
	if (num_dots == 0) {

		// Get how many controllers there are in this config file
		if (config.compare("pom_number") == 0) {
			int temp_num_controllers;
			if (!(ss >> temp_num_controllers)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be an int", value);
			}

			if (!configured_)
			{
				num_controllers_ = temp_num_controllers;
				setupVectors();
			}
			else 
			{
				if (temp_num_controllers != num_controllers_)
				{
					g_elastic.log(ERROR, "DAQConfig reconfig file needs to be of same structure as initial config file!");
				}
			}

			// Set the config file name in each of the configurations
			for (int i=0; i<num_controllers_; i++) configs_[i].config_name_ = file_name_;
		}

	} else if (num_dots == 1) {

		// Check this controller is enabled
		if (config.compare("enabled") == 0) {
			bool enabled;
			if (!(ss >> enabled)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be bool (0 or 1)", value);
			}

			if (enabled)
			{
				enabled_controllers_++;
				configs_[controller_num].enabled_ = true;
			}
			else {
				configs_[controller_num].enabled_ = false;
			}
		}	

		// Get the controllers electronic type
		if (config.compare("type") == 0) {
			std::string type;
			if (!(ss >> type)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be string (CLB or BBB)", value);
			}

			if (type == "CLB")
			{
				configs_[controller_num].type_ = CLB;
			}
			else if (type == "BBB")
			{
				configs_[controller_num].type_ = BBB;
			}
			else 
			{
				g_elastic.log(WARNING, "DAQControl error: ({}) should be string (CLB or BBB)", value);
			}
		}		

		// Add the controllers eID
		else if (config.compare("eid") == 0) {
			int temp_eid;
			if (!(ss >> temp_eid)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be int", value);
			}	

			if (!configured_)
			{
				configs_[controller_num].eid_ = temp_eid;
			}
			else
			{
				if (temp_eid != configs_[controller_num].eid_)
				{
					g_elastic.log(ERROR, "DAQConfig reconfig file needs to have same controllers as initial config file!");
				}
			}
		} 

		// Add the controllers MAC address
		else if (config.compare("mac") == 0) {
			if (!(ss >> std::hex >> configs_[controller_num].mac_)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be hex", value);
			}	
		} 

		// Add the controllers IP address
		else if (config.compare("ip") == 0) {
			if (!(ss >> configs_[controller_num].ip_)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be int", value); 
			}			
		}

		// Add the controllers slow-control port
		else if (config.compare("port") == 0) {
			if (!(ss >> configs_[controller_num].port_)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be int", value);  
			}			
		}

		// Should we use the relay to control the power?
		else if (config.compare("relay_control") == 0) {
			if (!(ss >> configs_[controller_num].relay_control_)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be bool (0 or 1)", value);  
			}			
		}

		// Get the relay type
		if (config.compare("relay_type") == 0) {
			std::string type;
			if (!(ss >> type)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be string (MC, EC, DANOUT)", value);
			}

			if (type == "MC")
			{
				configs_[controller_num].relay_type_ = MC;
			}
			else if (type == "EC")
			{
				configs_[controller_num].relay_type_ = EC;
			}
			else if (type == "DANOUT")
			{
				configs_[controller_num].relay_type_ = DANOUT;
			}
			else 
			{
				g_elastic.log(WARNING, "DAQControl error: ({}) should be string (MC, EC, DANOUT)", value);
			}
		}

		// Add the controllers relay IP address
		else if (config.compare("relay_ip") == 0) {
			if (!(ss >> configs_[controller_num].relay_ip_)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be int", value);  
			}			
		}

		// Add the controllers relay port
		else if (config.compare("relay_port") == 0) {
			if (!(ss >> configs_[controller_num].relay_port_)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be int", value);  
			}			
		}

		// Add the controllers relay chp
		else if (config.compare("relay_chp") == 0) {
			if (!(ss >> configs_[controller_num].relay_chp_)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be int", value);  
			}			
		}

		// Add the controllers relay chn
		else if (config.compare("relay_chn") == 0) {
			if (!(ss >> configs_[controller_num].relay_chn_)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be int", value);  
			}			
		}

		// Add the controllers data IP address
		else if (config.compare("data_ip") == 0) {
			if (!(ss >> configs_[controller_num].data_ip_)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be int", value); 
			}			
		}

		// Add the controllers data port
		else if (config.compare("data_port") == 0) {
			if (!(ss >> configs_[controller_num].data_port_)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be int", value); 
			}			
		}

		// Add the controllers data window duration
		else if (config.compare("data_window") == 0) {
			if (!(ss >> configs_[controller_num].data_window_)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be int", value); 
			}			
		}

		// Check if the nanobeacon should be enabled
		else if (config.compare("nano_enabled") == 0) {
			bool enabled = false;
			if (!(ss >> enabled)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be bool (0 or 1)", value);
			}

			if (enabled) 
			{
				is_nano_enabled_ = true;
				configs_[controller_num].nano_enabled_ = true;
			}
			else 
			{
				configs_[controller_num].nano_enabled_ = false;
			}
		}

		// Add the nanobeacon voltage
		else if (config.compare("nano_voltage") == 0) {
			if (!(ss >> configs_[controller_num].nano_voltage_)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be int", value);
			}	
		} 	
		
		// Add which channels are active
		else if (config.compare("ch_enabled") == 0) {
			unsigned binarySet;
			if (!(ss >> std::hex >> binarySet)) { 
				g_elastic.log(WARNING, "DAQControl error: ({}) should be hex (32 bit)", value);
			}			
			std::bitset<32> b(binarySet);
			configs_[controller_num].ch_enabled_ = b;

			for (int channel = 0; channel < 30; channel++) {
				if ((int)b[channel] == 1) { enabled_channels_ += 1; }
			}
		}

	} else if (num_dots == 2) {

		// Add the channel eID
		if (config.compare("id") == 0) {
			if (!(ss >> std::hex >> configs_[controller_num].ch_id_[channel_num])) {
				g_elastic.log(WARNING, "DAQControl error: ({}) should be hex", value);
			}			
		} 

		// Add the channel volatages
		else if (config.compare("hv") == 0) {
			if (!(ss >> configs_[controller_num].ch_hv_[channel_num])) {
				g_elastic.log(WARNING, "DAQControl error: ({}) should be int", value);
			}			
		} 

		// Add the channel thresholds
		else if (config.compare("th") == 0) {
			if (!(ss >> configs_[controller_num].ch_th_[channel_num])) {
				g_elastic.log(WARNING, "DAQControl error: ({}) should be int", value);
			}			
		} 

	} else {
		g_elastic.log(WARNING, "DAQControl error: Invalid config line!");
	}

	return;
}

/// Strip out comments preceded by // or # from the text being parsed
void DAQConfig::ignoreComments(std::string &line) 
{
	if (line.find("//") != line.npos)
		line.erase(line.find("//"));
	if (line.find("#") != line.npos)
		line.erase(line.find("#"));
	return;
}

/// Erase all the whitespace from the line
void DAQConfig::removeWhitespace(std::string &line) 
{
	line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
	line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());
	return;
}

/// Check whether a line could be trying to set a value
bool DAQConfig::isGoodLine(std::string line) 
{
	bool have_content = false;
	bool have_equals = false;
	bool have_lhs = false;
	bool have_rhs = false;

	// Look for non blank line
	if (line.find_first_not_of(' ') != line.npos) { have_content = true; }

	// Look for an equals sign
	if (line.find("=") != line.npos) { have_equals = true; }

	// Look for text on the LHS of = sign:
	std::string temp_line = line;
	temp_line.erase(0, temp_line.find_first_not_of("\t "));
	if (temp_line[0] != '0') { have_lhs = true; }

	// Look for text on RHS of = sign:
	temp_line = line;
	for (unsigned int rhs = temp_line.find("=") + 1; rhs < temp_line.length(); ++rhs) {
		if (temp_line[rhs] != '\t' && temp_line[rhs] != ' ') { have_rhs = true; }
	}

	return (have_content && have_equals && have_lhs && have_rhs);
}

/// Extract the details from the line
void DAQConfig::extractConfig(std::string line, int &numDots, int &controllerNum, int &channelNum, std::string &config, std::string &value) 
{
	// All the split locations
	size_t dot_1, ch, dot_2, equals;

	// First find the value
	equals = line.find("=");
	value = line.substr(equals + 1, (line.npos - equals - 1));

	// Find the non controller specific "pom_number" line
	if ((dot_1 = line.find(".")) == line.npos) { 
		numDots = 0;
		controllerNum = 0;
		channelNum = 0;
		config = line.substr(0, equals);
		return;
	}

	// Find the controller specific general settings
	if ((dot_2 = line.find(".", dot_1 + 1)) == line.npos) { 
		numDots = 1;
		controllerNum = std::stoi(line.substr(3, (dot_1 - 3)));
		channelNum = 0;
		config = line.substr(dot_1 + 1, (equals - dot_1 - 1));
		return;
	}

	// We now have a channel specific setting
	ch = line.find("h", dot_1);
	numDots = 2;
	controllerNum = std::stoi(line.substr(3, (dot_1 - 3)));
	channelNum = std::stoi(line.substr(ch + 1, (dot_2 - ch - 1)));
	config = line.substr(dot_2 + 1, (equals - dot_2 - 1));
	return;	
}

/// Setup all the vectors given the number of controllers in the config file
void DAQConfig::setupVectors() 
{
	ControllerConfig default_config; // Add the default configuration to the vector
	for (int i=0; i<num_controllers_; i++) configs_.push_back(default_config);
}