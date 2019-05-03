/**
 * ElasticInterface - Interfaces with elasticsearch for logging and monitoring
 */

#include "elastic_interface.h"

ElasticInterface g_elastic; ///< Global instance of this class

/// Create a ElasticInterface
ElasticInterface::ElasticInterface() :
    fClient({CLIENT}), fRand_count(0) {
    fBuilder["indentation"] = ""; // If you want whitespace-less output
}

/// Destroy a ElasticInterface
ElasticInterface::~ElasticInterface() {
    // Empty
}

/// Initialises the elasticsearch interface
void ElasticInterface::init(std::string processName, bool stdoutPrint) {
    srand(time(NULL));
    fPid = getpid();

    fStdoutPrint = stdoutPrint;

    fProcess_name = processName;
    fLog_message["process"] = fProcess_name; // Process name
    fLog_message["pid"] = fPid;              // Process ID
}

/// Indexes a "daqlog" index document to elasticsearch	
void ElasticInterface::log(severity level, std::string message) {
    genID();                            // generate random ID for document
    fLog_message["severity"] = level;   // severity level
    fLog_message["message"] = message;  // log message

    // Print to stdout if required
    if (fStdoutPrint) {
        std::cout << "LOG (" << level << "): " << message << std::endl; 
    }
    
    // index log to elasticsearch
    cpr::Response indexResponse = fClient.index("daqlog", "standard", 
                                                fID, Json::writeString(fBuilder, fLog_message));

    // check response
    if (indexResponse.status_code != 201) { 
        std::cout << "ElasticInterface::log Error: " << indexResponse.status_code << std::endl;
        std::cout << indexResponse.text << std::endl;
        std::cout << message << std::endl;
    }
}

void ElasticInterface::monitoringPacket() {
    // Empty
}

/// Generates a new random ID (length=IDLENGTH), stored in fPid
void ElasticInterface::genID() {
    static const char alphanum[] =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < IDLENGTH; ++i) { fID[i] = alphanum[(rand()+fRand_count) % (sizeof(alphanum) - 1)]; }
    fRand_count++;
}