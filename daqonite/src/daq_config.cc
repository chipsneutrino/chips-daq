/**
 * DAQConfig - Reads the config file for use in the DAQ
 */

#include "daq_config.h"

/// Create a DAQConfig
DAQConfig::DAQConfig(const char * config) : fConf_name(config) {

	fNum_clbs = 0;
	fEnabled_clbs = 0;
	fEnabled_channels = 0;

	fCLB_enabled.clear();
	fCLB_eids.clear();
	fCLB_channels.clear();
	fCLB_channel_eids.clear();
	
	loadConfig();
}

/// Destroy a DAQConfig
DAQConfig::~DAQConfig() {
    // Empty
}
 
/// Print the summary of the DAQConfig
void DAQConfig::printConfig() {
	std::cout << "\n***************** DAQ Config *****************" << std::endl;
	std::cout << "Mining Plan -> " << fConf_name << std::endl;
	std::cout << "Number of Miners -> " << fNum_clbs << std::endl;
	std::cout << "Number of Miners on Shift -> " << fEnabled_clbs << std::endl;
	std::cout << "Number of Pickaxes being used -> " << fEnabled_channels << std::endl;
	for (int i = 0; i<fNum_clbs; i++) {
		std::cout << "Miner " << i << " -> ID Badge = " << fCLB_eids[i] <<
		 			  ", Pickaxes = " << fCLB_channels[i].count() << std::endl;
	}
	std::cout << "**********************************************\n" << std::endl;
}

/// Print a short summary of the DAQConfig
void DAQConfig::printShortConfig() {
	std::cout << "\n***************** DAQ Config *****************" << std::endl;
	std::cout << "Mining Plan -> " << fConf_name << std::endl;
	std::cout << "Number of Miners -> " << fNum_clbs << std::endl;
	std::cout << "Number of Miners on Shift -> " << fEnabled_clbs << std::endl;
	std::cout << "Number of Pickaxes being used -> " << fEnabled_channels << std::endl;
	std::cout << "**********************************************\n" << std::endl;
}

/// Read the configuration text file specified by fConf_name
void DAQConfig::loadConfig() {

	// Open the file and test opening:
	std::ifstream input_file;
	input_file.open(fConf_name.c_str());

	if (!input_file.is_open()) {
		throw std::runtime_error("DAQConfig Error: Could not open config file!");
	}

	// Read the file
	std::string line;
	while (getline(input_file, line)) {
		parseLine(line);
	}

	return;
}

/// Parse the line from the file, filling the vectors with found info
void DAQConfig::parseLine(std::string &line) {

	// Remove comments from the line 
	ignoreComments(line);

	// Remove whitespace from the line
	removeWhitespace(line);

	// Check if we can use this line
	if (!(isGoodLine(line))) { return; }

	int num_dots, clb_num, channel_num;
	std::string config, value;

	std::string config_line = line;
	extractConfig(config_line, num_dots, clb_num, channel_num, config, value);

	//std::cout << num_dots << "," << clb_num << "," << channel_num << "," << config << "," << value << std::endl; 

	// We need to stream the value to different types depending on the config
	std::istringstream ss(value);
	
	// Assign value
	if (num_dots == 0) {

		// Get how many CLBs there are in this config file
		if (config.compare("clb_number") == 0) {
			if (!(ss >> fNum_clbs)) { 
				std::cerr << "Error: " << value << " should be a int" << std::endl; 
			}
			setupVectors();
		}

	} else if (num_dots == 1) {

		// Check this CLB is enabled
		if (config.compare("enabled") == 0) {
			bool enabled = false;
			if (!(ss >> enabled)) { 
				std::cerr << "Error: " << value << " should be int (0 or 1)" << std::endl;
			}

			if (enabled) {
				fEnabled_clbs++;
				fCLB_enabled[clb_num] = true;
			}
		}			

		// Add the CLBs eID
		if (config.compare("eid") == 0) {
			if (!(ss >> fCLB_eids[clb_num])) { 
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
			fCLB_channels[clb_num] = b;

			for (int channel = 0; channel < 30; channel++) {
				if ((int)b[channel] == 1) { fEnabled_channels += 1; }
			}
		}
		
	} else if (num_dots == 2) {

		// Add the channel eID
		if (config.compare("id") == 0) {
			if (!(ss >> std::hex >> fCLB_channel_eids[clb_num][channel_num])) {
				std::cerr << "Error: " << config << " = " << value << " should be hex" << std::endl;
			}			
		} 

	} else {
		std::cout << "DAQConfig Error: Invalid config line!" << std::endl;
	}

	return;
}

/// Strip out comments preceded by // or # from the text being parsed
void DAQConfig::ignoreComments(std::string &line) {
	if (line.find("//") != line.npos)
		line.erase(line.find("//"));
	if (line.find("#") != line.npos)
		line.erase(line.find("#"));
	return;
}

/// Erase all the whitespace from the line
void DAQConfig::removeWhitespace(std::string &line) {
	line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
	line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());
	return;
}

/// Check whether a line could be trying to set a value
bool DAQConfig::isGoodLine(std::string line) {
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
void DAQConfig::extractConfig(std::string line, int &numDots, int &clbNum, int &channelNum, std::string &config, std::string &value) {

	// All the split locations
	size_t dot_1, ch, dot_2, equals;

	// First find the value
	equals = line.find("=");
	value = line.substr(equals + 1, (line.npos - equals - 1));

	// Find the non CLB specific "clb_number" line
	if ((dot_1 = line.find(".")) == line.npos) { 
		numDots = 0;
		clbNum = 0;
		channelNum = 0;
		config = line.substr(0, equals);
		return;
	}

	// Find the CLB specific general settings
	if ((dot_2 = line.find(".", dot_1 + 1)) == line.npos) { 
		numDots = 1;
		clbNum = std::stoi(line.substr(3, (dot_1 - 3)));
		channelNum = 0;
		config = line.substr(dot_1 + 1, (equals - dot_1 - 1));
		return;
	}

	// We now have a channel specific setting
	ch = line.find("h", dot_1);
	numDots = 2;
	clbNum = std::stoi(line.substr(3, (dot_1 - 3)));
	channelNum = std::stoi(line.substr(ch + 1, (dot_2 - ch - 1)));
	config = line.substr(dot_2 + 1, (equals - dot_2 - 1));
	return;	
}

/// Setup all the vectors given the number of CLBs in the config file
void DAQConfig::setupVectors() {

	// Just need placeholders to fill the vectors will set later
	std::bitset<32> temp_bitset;
	std::array<unsigned int, 31> temp_eids;

	// Add all CLBs
	for (int i=0; i<fNum_clbs; i++) {
		fCLB_enabled.push_back(false);
		fCLB_eids.push_back(0);
		fCLB_channels.push_back(temp_bitset);
		fCLB_channel_eids.push_back(temp_eids);
	}
}