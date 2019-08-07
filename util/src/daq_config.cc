/**
 * DAQConfig - Reads the config file for use in the DAQ
 */

#include "daq_config.h"

/// Create a DAQConfig
DAQConfig::DAQConfig(const char * config) 
	: conf_name_(config)
	, configured_(false)
{
	num_controllers_ = 0;
	enabled_controllers_ = 0;
	enabled_channels_ = 0;
	is_nano_enabled_ = false;

	configs_.clear();
	
	loadConfig();
}

/// Destroy a DAQConfig
DAQConfig::~DAQConfig() 
{
    // Empty
}
 
/// Print the summary of the DAQConfig
void DAQConfig::printConfig() 
{
	std::cout << "\n***************** DAQ Config *****************" << std::endl;
	std::cout << "Mining Plan -> " << conf_name_ << std::endl;
	std::cout << "Number of Miners -> " << num_controllers_ << std::endl;
	std::cout << "Number of Miners on Shift -> " << enabled_controllers_ << std::endl;
	std::cout << "Number of Pickaxes being used -> " << enabled_channels_ << std::endl;
	for (int i = 0; i<num_controllers_; i++) {
		std::cout << "Miner " << i << "(" << configs_[i].enabled_ << ")" << 
					" -> ID = " << configs_[i].eid_ <<
					", Address = " << configs_[i].ip_ <<
		 			", Pickaxes = " << configs_[i].chan_enabled_.count() <<
					", Nanobeacon = " << configs_[i].nano_enabled_ << std::endl;
	}
	std::cout << "**********************************************\n" << std::endl;
}

/// Print a short summary of the DAQConfig
void DAQConfig::printShortConfig() 
{
	std::cout << "\n***************** DAQ Config *****************" << std::endl;
	std::cout << "Mining Plan -> " << conf_name_ << std::endl;
	std::cout << "Number of Miners -> " << num_controllers_ << std::endl;
	std::cout << "Number of Miners on Shift -> " << enabled_controllers_ << std::endl;
	std::cout << "Number of Pickaxes being used -> " << enabled_channels_ << std::endl;
	std::cout << "**********************************************\n" << std::endl;
}

/// Load a new configuration from file
void DAQConfig::loadConfig(const char * config)
{
	// Reset some of the variables that we allow to change
	enabled_controllers_ = 0;
	enabled_channels_ = 0;
	is_nano_enabled_ = false;

	conf_name_ = config; // Set the new configuration file name

	loadConfig(); // Load the new configuration
}

/// Read the configuration text file specified by conf_name_
void DAQConfig::loadConfig() 
{
	// Open the file and test opening:
	std::ifstream input_file;
	input_file.open(conf_name_.c_str());

	if (!input_file.is_open()) {
		throw std::runtime_error("DAQConfig Error: Could not open config file!");
	}

	// Read the file
	std::string line;
	while (getline(input_file, line)) {
		parseLine(line);
	}

	configured_ = true; // Set that this DAQConfig has been configured

	printConfig();	// Print the configuration

	return;
}

/// Parse the line from the file, filling the vectors with found info
void DAQConfig::parseLine(std::string &line) 
{
	// Remove comments from the line 
	ignoreComments(line);

	// Remove whitespace from the line
	removeWhitespace(line);

	// Check if we can use this line
	if (!(isGoodLine(line))) { return; }

	int num_dots, controller_num, channel_num;
	std::string config, value;

	std::string config_line = line;
	extractConfig(config_line, num_dots, controller_num, channel_num, config, value);

	// We need to stream the value to different types depending on the config
	std::istringstream ss(value);
	
	// Assign value
	if (num_dots == 0) {

		// Get how many controllers there are in this config file
		if (config.compare("clb_number") == 0) {
			int temp_num_controllers;
			if (!(ss >> temp_num_controllers)) { 
				std::cerr << "Error: " << value << " should be a int" << std::endl; 
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
					throw std::runtime_error("DAQConfig Error: Reconfig file needs to be of same structure as initial config file!");
				}
			}
		}

	} else if (num_dots == 1) {

		// Check this controller is enabled
		if (config.compare("enabled") == 0) {
			bool enabled = false;
			if (!(ss >> enabled)) { 
				std::cerr << "Error: " << value << " should be int (0 or 1)" << std::endl;
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

		// Add the controllers eID
		else if (config.compare("eid") == 0) {
			int temp_eid;
			if (!(ss >> temp_eid)) { 
				std::cerr << "Error: " << value << " should be int" << std::endl; 
			}	

			if (!configured_)
			{
				configs_[controller_num].eid_ = temp_eid;
			}
			else
			{
				if (temp_eid != configs_[controller_num].eid_)
				{
					throw std::runtime_error("DAQConfig Error: Reconfig file needs to have same controllers as initial config file!");
				}
			}
		} 

		else if (config.compare("ip") == 0) {
			if (!(ss >> configs_[controller_num].ip_)) { 
				std::cerr << "Error: " << value << " should be int" << std::endl; 
			}			
		}
		
		// Add which channels are active
		else if (config.compare("ch_enabled") == 0) {
			unsigned binarySet;
			if (!(ss >> std::hex >> binarySet)) { 
				std::cerr << "Error: " << value << " should be hex (32 bit)" << std::endl; 
			}			
			std::bitset<32> b(binarySet);
			configs_[controller_num].chan_enabled_ = b;

			for (int channel = 0; channel < 30; channel++) {
				if ((int)b[channel] == 1) { enabled_channels_ += 1; }
			}
		}

		// Check if the nanobeacon should be anabled
		else if (config.compare("nano_enabled") == 0) {
			bool enabled = false;
			if (!(ss >> enabled)) { 
				std::cerr << "Error: " << value << " should be int (0 or 1)" << std::endl;
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
				std::cerr << "Error: " << value << " should be int" << std::endl; 
			}	
		} 	
		
	} else if (num_dots == 2) {

		// Add the channel eID
		if (config.compare("id") == 0) {
			if (!(ss >> std::hex >> configs_[controller_num].chan_eid_[channel_num])) {
				std::cerr << "Error: " << config << " = " << value << " should be hex" << std::endl;
			}			
		} 

		// Add the channel volatages
		else if (config.compare("hv") == 0) {
			if (!(ss >> configs_[controller_num].chan_hv_[channel_num])) {
				std::cerr << "Error: " << config << " = " << value << " should be hex" << std::endl;
			}			
		} 

	} else {
		std::cout << "DAQConfig Error: Invalid config line!" << std::endl;
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

	// Find the non controller specific "clb_number" line
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