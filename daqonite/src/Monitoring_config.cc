/**
 * Monitoring_config - Reads the config file for use in monitoring
 */

#include "Monitoring_config.h"
#include "DAQ_handler.h"

Monitoring_config::Monitoring_config(const char * config) : fConfName(config) {
	fMap.clear();

	fNumCLBs = 0;
	fCLBeIDs.clear();
	fCLBTypes.clear();

	fTotalNumChannels = 0;
	fActiveChannels.clear();
	
	this->loadConfig();
}

Monitoring_config::~Monitoring_config() {
    // Empty
}
 
void Monitoring_config::printConfig() {
	std::cout << "\n**************** DAQonite - Print Config ****************" << std::endl;
	std::cout << "Mining Plan -> " << fConfName << std::endl;
	std::cout << "Number of Miners on Shift (CLBs) -> " << fNumCLBs << std::endl;
	std::cout << "Number of Pickaxes (PMT Channels) -> " << fTotalNumChannels << std::endl;
	for (int i = 0; i<fNumCLBs; i++) {
		std::cout << "Miner " << i << " -> ID Badge = " << fCLBeIDs[i] <<
		 			 ", Type = " << fCLBTypes[i] << ", Pickaxes = " <<
					  fActiveChannels[i].count() << std::endl;
	}
	std::cout << "*********************************************************\n" << std::endl;
}

void Monitoring_config::loadConfig() {
	// Open the file and test opening:
	std::ifstream inFile;
	inFile.open(fConfName.c_str());

	if (!inFile.is_open()) {
		throw std::runtime_error("DAQonite: Error: Could not open config file!");
	}

	// Read the file
	std::string line;
	Int_t lineNum = 0;
	while (getline(inFile, line)) {
		this->ignoreComments(line);
		this->parseLine(line, ++lineNum);
	}
	this->setFromMap();

	return;
}

/// Erases comments beginning with // or #
void Monitoring_config::ignoreComments(std::string &str) {
	if (str.find("//") != str.npos)
		str.erase(str.find("//"));
	if (str.find("#") != str.npos)
		str.erase(str.find("#"));
	return;
}

const Bool_t Monitoring_config::isBlankLine(std::string str) {
	if (str.find_first_not_of(' ') == str.npos) {
		return true;
	} else {
		return false;
	}
}

const Bool_t Monitoring_config::isGoodLine(std::string str) {
	Bool_t haveEquals = false;
	Bool_t haveLHS = false;
	Bool_t haveRHS = false;

	// Look for an equals sign
	if (str.find("=") == str.npos) {
		//std::cout << "No \"=\" sign found in string: " << std::endl << str << std::endl;
	} else {
		haveEquals = true;
	}

	// Look for text on the LHS of = sign:
	std::string tempStr = str;
	tempStr.erase(0, tempStr.find_first_not_of("\t "));
	if (tempStr[0] != '0') {
		haveLHS = true;
	}

	// Look for text on RHS of = sign:
	tempStr = str;
	for (unsigned int rhs = tempStr.find("=") + 1; rhs < tempStr.length(); ++rhs) {
		if (tempStr[rhs] != '\t' && tempStr[rhs] != ' ')
			haveRHS = true;
	}

	return (haveEquals && haveLHS && haveRHS);
}

void Monitoring_config::extractPair(std::string &lhs, std::string &rhs, std::string str) {
	unsigned int splitPos = str.find("=");

	// Get left hand side of = sign and strip whitespace
	lhs = str.substr(0, splitPos);
	lhs.erase(std::remove(lhs.begin(), lhs.end(), ' '), lhs.end());
	lhs.erase(std::remove(lhs.begin(), lhs.end(), '\t'), lhs.end());

	// And the other side
	rhs = str.substr(splitPos + 1);
	rhs.erase(std::remove(rhs.begin(), rhs.end(), ' '), rhs.end());
	rhs.erase(std::remove(rhs.begin(), rhs.end(), '\t'), rhs.end());

	return;
}

void Monitoring_config::parseLine(std::string str, Int_t lineNum) {
	if (!(this->isGoodLine(str))) {
		return;
	}

	std::string lhs, rhs;
	this->extractPair(lhs, rhs, str);
	this->addToMap(lhs, rhs);
	return;
}

void Monitoring_config::addToMap(std::string lhs, std::string rhs) {
	if (fMap.find(lhs) == fMap.end()) {
		fMap[lhs] = rhs;
	}
	return;
}

void Monitoring_config::extractConfigType(int &numDots, std::string &clbNum, std::string &channelNum, std::string &config, std::string str) {

	// first remove whitespace from the string
	std::string configString = str;
	configString.erase(std::remove(configString.begin(), configString.end(), ' '), configString.end());
	configString.erase(std::remove(configString.begin(), configString.end(), '\t'), configString.end());

	size_t splitPos1, splitPos2;
	if ((splitPos1 = configString.find(".")) == configString.npos) { 
		// This is a non CLB specific config line
		numDots = 0;
		return;
	}

	// Get the clb number and erase whitespace 
	clbNum = configString.substr(3, 1);
	if ((splitPos2 = configString.find(".", splitPos1 + 1)) == configString.npos) { 
		// This is a general CLB config line
		config = configString.substr(splitPos1 + 1);
		numDots = 1;
		channelNum = "";
		return;
	}

	// This is a channel specific config line
	// Get the channel number and erase whitespace 
	channelNum = configString.substr(splitPos1 + 3, splitPos2 - splitPos1 - 3);

	// Set the channel 
	config = configString.substr(splitPos2 + 1);
	numDots = 2;
	return;
}

void Monitoring_config::setFromMap() {
	std::map<std::string, std::string>::const_iterator itr = fMap.begin();

	// Loop through the map, checking if any of the keys correspond to things we can set
	for (; itr != fMap.end(); ++itr) {
		int numDots;
		std::string clbNum, channelNum, config;
		extractConfigType(numDots, clbNum, channelNum, config, (*itr).first);

		//std::cout << numDots << "-" << clbNum << "-" << channelNum << "-" << config << std::endl; 
		//std::cout << (*itr).second << std::endl;

		if (numDots == 0) {

			if ((*itr).first.compare("clb_number") == 0) {
				std::stringstream ss(itr->second);
				int val = 0.0;
				if (!(ss >> val)) {
					std::cerr << "Error: " << (*itr).second << " = " << (*itr).second << " should be a double" << std::endl;
				}
				fNumCLBs = val;
			}

		} else if (numDots == 1) {

			if (config.compare("eid") == 0) {
				std::stringstream ss(itr->second);
				int val = 0.0;
				if (!(ss >> val)) {
					std::cerr << "Error: " << (*itr).second << " = " << (*itr).second << " should be a double" << std::endl;
				}
				fCLBeIDs.push_back(val);				
			} else if (config.compare("hid_type") == 0) {
				std::stringstream ss(itr->second);
				int val = 0.0;
				if (!(ss >> val)) {
					std::cerr << "Error: " << (*itr).second << " = " << (*itr).second << " should be a double" << std::endl;
				}
				fCLBTypes.push_back(val);					
			} else if (config.compare("ch_enabled") == 0) {
				std::stringstream ss;
				ss << std::hex << itr->second;
	    		unsigned binarySet;
				if (!(ss >> binarySet)) {
					std::cerr << "Error: " << (*itr).second << " = " << (*itr).second << " should be a Hex (32 bit)" << std::endl;
				}			
    			std::bitset<32> b(binarySet);
    			fActiveChannels.push_back(b);

				for (int channel = 0; channel < 30; channel++) {
					if ((int)b[channel] == 1) { fTotalNumChannels += 1; }
				}
			}

		} else if (numDots == 2) {

			// TODO: Store the actual channel eIDs and use to check for hits...

		} else {
			std::cout << "DAQonite: Error: Invalid config line!" << std::endl;
		}

	}
	fMap.clear();

	printConfig();

}