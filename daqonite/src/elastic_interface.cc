/**
 * ElasticInterface - Interfaces with elasticsearch for logging and monitoring
 * 
 * TODO: Implement file logging if elasticsearch unavailable
 * TODO: How do I want to use the configuration for monitoring/alerting
 */

#include "elastic_interface.h"

ElasticInterface g_elastic; ///< Global instance of this class

/// Create a ElasticInterface
ElasticInterface::ElasticInterface() :
    fClient({CLIENT}) {
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

    long value_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::time_point_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now()).time_since_epoch()).count();

    fLog_message["@timestamp"] = value_ms;      // ms timestamp
    fLog_message["severity"] = level;           // severity level
    fLog_message["message"] = message;          // log message

    // Print to stdout if required
    if (fStdoutPrint) { std::cout << "LOG (" << level << "): " << message << std::endl; }
    
    // index log to elasticsearch
    cpr::Response indexResponse = fClient.index("daqlog", "standard", "", 
                                                Json::writeString(fBuilder, fLog_message));

    // check response
    if (indexResponse.status_code != 201) { 
        std::cout << "ElasticInterface::log Error: " << indexResponse.status_code << std::endl;
        std::cout << indexResponse.text << std::endl;
    }

    fMutex.unlock();    // unlock mutex
}

/// Indexes a "daqmon" index document to elasticsearch of type "pommon"
void ElasticInterface::monitoringPacket(int &run_num, int &pom_id, long &timestamp, 
                                        int &temperature, int &humidity, 
                                        std::string &message, int* hits) {

    fMutex.lock();      // lock mutex

    long value_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::time_point_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now()).time_since_epoch()).count();

    fMonitor_message["@timestamp"] = value_ms;      // ms timestamp
    fMonitor_message["run_num"] = run_num;          // run number
    fMonitor_message["pom_id"] = pom_id;            // planar optical module ID
    fMonitor_message["packet_time"] = timestamp;    // timestamp from the monitoring packet
    fMonitor_message["temperature"] = temperature;  // planar optical module temperature
    fMonitor_message["humidity"] = humidity;        // planar optical module humidity
    fMonitor_message["message"] = message;          // optional message
    for (int i=0; i<30; i++) { fMonitor_message["hits"][i] = hits[i]; } // hits on each channel

    // index monitoring message to elasticsearch
    cpr::Response indexResponse = fClient.index("daqmon", "pommon", "",
                                                Json::writeString(fBuilder, fMonitor_message));

    // check response
    if (indexResponse.status_code != 201) { 
        std::cout << "ElasticInterface::monitoringPacket: " << indexResponse.status_code << std::endl;
        std::cout << indexResponse.text << std::endl;
    }

    fMutex.unlock();    // unlock mutex
}


/// Indexes a document to elasticsearch of given type
void ElasticInterface::monitoringValue(std::string index, std::string type, float value) {

    fMutex.lock();      // lock mutex

    long value_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::time_point_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now()).time_since_epoch()).count();

    Json::Value message;
    fMonitor_message["@timestamp"] = value_ms;      // ms timestamp
    fMonitor_message["value"] = value;              // monitoring value

    // index monitoring message to elasticsearch
    cpr::Response indexResponse = fClient.index(index, type, "", Json::writeString(fBuilder, message));

    // check response
    if (indexResponse.status_code != 201) { 
        std::cout << "ElasticInterface::monitoringPacket: " << indexResponse.status_code << std::endl;
        std::cout << indexResponse.text << std::endl;
    }

    fMutex.unlock();    // unlock mutex
}