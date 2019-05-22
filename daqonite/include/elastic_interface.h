/**
 * ElasticInterface - Interfaces with elasticsearch for logging and monitoring
 * 
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef ELASTIC_INTERFACE_H_
#define ELASTIC_INTERFACE_H_

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>

#include <cpr/response.h>
#include <elasticlient/client.h>
#include <elasticlient/logging.h>
#include <json/json.h>

#include <boost/thread.hpp>

///< Define the elasticsearch address
#define CLIENT "http://localhost:9200/"

/// Enum for describing the different logging severity levels
enum severity { TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL };

/// Enum for describing the different logging modes
enum log_mode { ELASTIC,
    FILE_LOG };

/// Callback for elasticlient logs
inline void elasticlient_callback(elasticlient::LogLevel logLevel, const std::string& msg)
{
    if (logLevel != elasticlient::LogLevel::DEBUG) {
        std::cout << "LOG " << (unsigned)logLevel << ": " << msg << std::endl;
    }
}

class ElasticInterface {
public:
    /// Create a ElasticInterface
    ElasticInterface();

    /// Destroy a ElasticInterface
    ~ElasticInterface();

    /**
		 * Initialises the elasticsearch interface
		 * Sets up the process specific bits we need for logging 
         * and monitoring.
		 * 
		 * @param processName   name of the process
         * @param stdoutPrint   print logs to stdout
         * @param commsLog      print elasticlient log message
		 */
    void init(std::string processName, bool stdoutPrint, bool commsLog);

    /**
		 * Indexes a "daqlog" index document to elasticsearch
		 * Creates a log message and PUTS it to elasticsearch
         * Document ID is created by elasticsearch
         * 
         * "_index":            "daqlog"
         * "_type":             "standard"
         * 
         * "@timestamp"         indexing timestamp
         * "severity":          level
         * "process":           process sending the log
         * "pid":               pid of process sending the log
         * "message":           message
		 * 
		 * @param level         severity level of log
		 * @param message       log Message
		 */
    void log(severity level, std::string message);

    /**
		 * Indexes a "daqmon" index document to elasticsearch of type "pommon"
		 * Creates a message and PUTS it to elasticsearch
         * Document ID is created by elasticsearch
         * 
         * "_index":            "daqmon"
         * "_type":             "pommon"
         * 
         * "@timestamp"         indexing timestamp
         * "run_num":           current run number if any
         * "pom_id":            planar optical module id
         * "packet_time":       timestamp of monitoring packet in ms
         * "temperature":       POM temperature
         * "humidity":          POM humidity
         * "hits":              hits for all channels
         * "message":           optional message
		 */
    void monitoringPacket(int& run_num, int& pom_id, long& timestamp,
        int& temperature, int& humidity,
        std::string& message, int* hits);

    /**
		 * Indexes a document to elasticsearch of given type
		 * Creates a message and PUTS it to elasticsearch
         * Document ID is created by elasticsearch
         * 
         * "_index":            index
         * "_type":             type
         * 
         * "@timestamp"         indexing timestamp
         * "value":             value given
		 */
    void monitoringValue(std::string index, std::string type, float value);

private:
    /**
		 * Initialise file logging
		 * Opens a logging file and writes reason for switching
         * 
         * @param error         error that caused switch to file logging
         * @param writeLog      write the current log to file
		 */
    void initFile(std::string error, bool writeLog);

    /**
		 * Generate a filename for the .txt log file
		 * Uses the current time for generation
		 */
    void generateFilename();

    // Client
    elasticlient::Client fClient; ///< The ElasticSearch client as provided by elasticlient library
    log_mode fMode; ///< What logging mode are we in {ELASTIC, FILE_LOG}
    std::string fFile_name; ///< file name used when in FILE_LOG mode

    // Settings
    bool fStdoutPrint; ///< Should we print logs to stdout?

    // Messaging
    Json::StreamWriterBuilder fBuilder; ///< Json writer to stream json object to string
    boost::mutex fMutex; ///< Mutex to keep everything thread safe
    Json::Value fMonitor_message; ///< Json monitoring message used to send monitoring data to elasticsearch
    Json::Value fLog_message; ///< Json log message used to send logs to elasticsearch
};

extern ElasticInterface g_elastic; ///< Global instance of this class

#endif