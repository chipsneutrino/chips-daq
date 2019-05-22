/**
 * MonitoringConfig - Reads the config file for use in monitoring
 */

#include "monitoring_config.h"

MonitoringConfig::MonitoringConfig(const char* config)
    : fConf_name(config)
{
    fMap.clear();

    fNum_clbs = 0;
    fCLB_eids.clear();
    fCLB_types.clear();

    fTotal_num_channels = 0;
    fActive_channels.clear();

    this->loadConfig();
}

MonitoringConfig::~MonitoringConfig()
{
    // Empty
}

void MonitoringConfig::printConfig()
{
    std::cout << "\n**************** DAQonite - Print Config ****************" << std::endl;
    std::cout << "Mining Plan -> " << fConf_name << std::endl;
    std::cout << "Number of Miners on Shift (CLBs) -> " << fNum_clbs << std::endl;
    std::cout << "Number of Pickaxes (PMT Channels) -> " << fTotal_num_channels << std::endl;
    for (int i = 0; i < fNum_clbs; i++) {
        std::cout << "Miner " << i << " -> ID Badge = " << fCLB_eids[i] << ", Type = " << fCLB_types[i] << ", Pickaxes = " << fActive_channels[i].count() << std::endl;
    }
    std::cout << "*********************************************************\n"
              << std::endl;
}

void MonitoringConfig::loadConfig()
{
    // Open the file and test opening:
    std::ifstream input_file;
    input_file.open(fConf_name.c_str());

    if (!input_file.is_open()) {
        throw std::runtime_error("DAQonite: Error: Could not open config file!");
    }

    // Read the file
    std::string line;
    int lineNum = 0;
    while (getline(input_file, line)) {
        this->ignoreComments(line);
        this->parseLine(line, ++lineNum);
    }
    this->setFromMap();

    return;
}

/// Erases comments beginning with // or #
void MonitoringConfig::ignoreComments(std::string& str)
{
    if (str.find("//") != str.npos)
        str.erase(str.find("//"));
    if (str.find("#") != str.npos)
        str.erase(str.find("#"));
    return;
}

const bool MonitoringConfig::isBlankLine(std::string str)
{
    if (str.find_first_not_of(' ') == str.npos) {
        return true;
    } else {
        return false;
    }
}

const bool MonitoringConfig::isGoodLine(std::string str)
{
    bool have_equals = false;
    bool have_lhs = false;
    bool have_rhs = false;

    // Look for an equals sign
    if (str.find("=") == str.npos) {
        //std::cout << "No \"=\" sign found in string: " << std::endl << str << std::endl;
    } else {
        have_equals = true;
    }

    // Look for text on the LHS of = sign:
    std::string temp_str = str;
    temp_str.erase(0, temp_str.find_first_not_of("\t "));
    if (temp_str[0] != '0') {
        have_lhs = true;
    }

    // Look for text on RHS of = sign:
    temp_str = str;
    for (unsigned int rhs = temp_str.find("=") + 1; rhs < temp_str.length(); ++rhs) {
        if (temp_str[rhs] != '\t' && temp_str[rhs] != ' ')
            have_rhs = true;
    }

    return (have_equals && have_lhs && have_rhs);
}

void MonitoringConfig::extractPair(std::string& lhs, std::string& rhs, std::string str)
{
    unsigned int split_pos = str.find("=");

    // Get left hand side of = sign and strip whitespace
    lhs = str.substr(0, split_pos);
    lhs.erase(std::remove(lhs.begin(), lhs.end(), ' '), lhs.end());
    lhs.erase(std::remove(lhs.begin(), lhs.end(), '\t'), lhs.end());

    // And the other side
    rhs = str.substr(split_pos + 1);
    rhs.erase(std::remove(rhs.begin(), rhs.end(), ' '), rhs.end());
    rhs.erase(std::remove(rhs.begin(), rhs.end(), '\t'), rhs.end());

    return;
}

void MonitoringConfig::parseLine(std::string str, int lineNum)
{
    if (!(this->isGoodLine(str))) {
        return;
    }

    std::string lhs, rhs;
    this->extractPair(lhs, rhs, str);
    this->addToMap(lhs, rhs);
    return;
}

void MonitoringConfig::addToMap(std::string lhs, std::string rhs)
{
    if (fMap.find(lhs) == fMap.end()) {
        fMap[lhs] = rhs;
    }
    return;
}

void MonitoringConfig::extractConfigType(int& num_dots, std::string& clb_num, std::string& channel_num, std::string& config, std::string str)
{

    // first remove whitespace from the string
    std::string config_string = str;
    config_string.erase(std::remove(config_string.begin(), config_string.end(), ' '), config_string.end());
    config_string.erase(std::remove(config_string.begin(), config_string.end(), '\t'), config_string.end());

    size_t split_pos_1, split_pos_2;
    if ((split_pos_1 = config_string.find(".")) == config_string.npos) {
        // This is a non CLB specific config line
        num_dots = 0;
        return;
    }

    // Get the clb number and erase whitespace
    clb_num = config_string.substr(3, 1);
    if ((split_pos_2 = config_string.find(".", split_pos_1 + 1)) == config_string.npos) {
        // This is a general CLB config line
        config = config_string.substr(split_pos_1 + 1);
        num_dots = 1;
        channel_num = "";
        return;
    }

    // This is a channel specific config line
    // Get the channel number and erase whitespace
    channel_num = config_string.substr(split_pos_1 + 3, split_pos_2 - split_pos_1 - 3);

    // Set the channel
    config = config_string.substr(split_pos_2 + 1);
    num_dots = 2;
    return;
}

void MonitoringConfig::setFromMap()
{
    std::map<std::string, std::string>::const_iterator itr = fMap.begin();

    // Loop through the map, checking if any of the keys correspond to things we can set
    for (; itr != fMap.end(); ++itr) {
        int num_dots;
        std::string clb_num, channel_num, config;
        extractConfigType(num_dots, clb_num, channel_num, config, (*itr).first);

        //std::cout << num_dots << "-" << clb_num << "-" << channel_num << "-" << config << std::endl;
        //std::cout << (*itr).second << std::endl;

        if (num_dots == 0) {

            if ((*itr).first.compare("clb_number") == 0) {
                std::stringstream ss(itr->second);
                int val = 0.0;
                if (!(ss >> val)) {
                    std::cerr << "Error: " << (*itr).second << " = " << (*itr).second << " should be a double" << std::endl;
                }
                fNum_clbs = val;
            }

        } else if (num_dots == 1) {

            if (config.compare("eid") == 0) {
                std::stringstream ss(itr->second);
                int val = 0.0;
                if (!(ss >> val)) {
                    std::cerr << "Error: " << (*itr).second << " = " << (*itr).second << " should be a double" << std::endl;
                }
                fCLB_eids.push_back(val);
            } else if (config.compare("hid_type") == 0) {
                std::stringstream ss(itr->second);
                int val = 0.0;
                if (!(ss >> val)) {
                    std::cerr << "Error: " << (*itr).second << " = " << (*itr).second << " should be a double" << std::endl;
                }
                fCLB_types.push_back(val);
            } else if (config.compare("ch_enabled") == 0) {
                std::stringstream ss;
                ss << std::hex << itr->second;
                unsigned binarySet;
                if (!(ss >> binarySet)) {
                    std::cerr << "Error: " << (*itr).second << " = " << (*itr).second << " should be a Hex (32 bit)" << std::endl;
                }
                std::bitset<32> b(binarySet);
                fActive_channels.push_back(b);

                for (int channel = 0; channel < 30; channel++) {
                    if ((int)b[channel] == 1) {
                        fTotal_num_channels += 1;
                    }
                }
            }

        } else if (num_dots == 2) {

            // TODO: Store the actual channel eIDs and use to check for hits...

        } else {
            std::cout << "DAQonite: Error: Invalid config line!" << std::endl;
        }
    }
    fMap.clear();
}