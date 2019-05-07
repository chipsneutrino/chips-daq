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
void ElasticInterface::init(std::string processName, bool stdoutPrint, bool clientLog) {
    srand(time(NULL));
    fPid = getpid();

    fStdoutPrint = stdoutPrint;

    fProcess_name = processName;
    fLog_message["process"] = fProcess_name;    // Process name
    fLog_message["pid"] = fPid;                 // Process ID

    if (clientLog) { elasticlient::setLogFunction(elasticlient_callback);}
}

/// Indexes a "daqlog" index document to elasticsearch	
void ElasticInterface::log(severity level, std::string message) {

    fMutex.lock();      // lock mutex

    fLog_message["severity"] = level;           // severity level
    fLog_message["message"] = message;          // log message

    // Print to stdout if required
    if (fStdoutPrint) { std::cout << "LOG (" << level << "): " << message << std::endl; }
    
    // index log to elasticsearch
    cpr::Response indexResponse = fClient.index("daqlog", "standard", 
                                                genID(), Json::writeString(fBuilder, fLog_message));

    // check response
    if (indexResponse.status_code != 201) { 
        std::cout << "ElasticInterface::log Error: " << indexResponse.status_code << std::endl;
        std::cout << indexResponse.text << std::endl;
    }

    fMutex.unlock();    // unlock mutex
}

void ElasticInterface::monitoringPacket(int &run_num, int &pom_id, int &timestamp, 
                                        int &temperature, int &humidity, 
                                        std::string &message, int* hits) {

    fMutex.lock();      // lock mutex

    fMonitor_message["run_num"] = run_num;          // run number
    fMonitor_message["pom_id"] = pom_id;            // planar optical module ID
    fMonitor_message["timestamp"] = timestamp;      // timestamp from the monitoring packet
    fMonitor_message["temperature"] = temperature;  // planar optical module temperature
    fMonitor_message["humidity"] = humidity;        // planar optical module humidity
    fMonitor_message["message"] = message;          // optional message
    for (int i=0; i<30; i++) { fMonitor_message["hits"][i] = hits[i]; } // hits on each channel

    // index monitoring message to elasticsearch
    cpr::Response indexResponse = fClient.index("pommon", "standard", 
                                                genID(), Json::writeString(fBuilder, fMonitor_message));

    // check response
    if (indexResponse.status_code != 201) { 
        std::cout << "ElasticInterface::monitoringPacket: " << indexResponse.status_code << std::endl;
        std::cout << indexResponse.text << std::endl;
    }

    fMutex.unlock();    // unlock mutex
}

/// Generates a new random ID (length=IDLENGTH), stored in fPid
std::string ElasticInterface::genID() {
    static const char alphanum[] =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz";

    std::string id;
    for (int i = 0; i < IDLENGTH; ++i) { id.push_back(alphanum[(rand()+fRand_count) % (sizeof(alphanum)-1)]); }
    fRand_count++;
    return id;
}