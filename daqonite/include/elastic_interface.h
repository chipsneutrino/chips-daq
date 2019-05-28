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
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <array>

#include <cpr/response.h>
#include <elasticlient/client.h>
#include <elasticlient/logging.h>
#include <elasticlient/bulk.h>
#include <json/json.h>

#include <boost/thread.hpp>
#include <boost/asio.hpp>

/// Enum for describing the different logging severity levels
enum severity
{
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

/// Enum for describing the different logging modes
enum log_mode
{
    ELASTIC,
    FILE_LOG
};

#define NUM_THREADS 100
#define MAX_LOG_RATE 5
#define MAX_TRIES 3

/// Callback for elasticlient logs
inline void elasticlient_callback(elasticlient::LogLevel logLevel, const std::string &msg)
{
    std::cout << "LOG (" << (unsigned)logLevel << "): " << msg << std::endl;
}

class ElasticInterface
{
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
         * @param debug         print debug messages
		 */
    void init(std::string processName, bool stdoutPrint, bool debug);

    /**
		 * Indexes a "daqlog" index document to elasticsearch
		 * Creates the JSON message and PUTS it to elasticsearch
         * Document ID is created by elasticsearch
		 * 
		 * @param level         severity level of log
		 * @param message       log Message
		 */
    void log(severity level, std::string message);

    /**
		 * Indexes a "daqmon" document and "daqhits" documents to elasticsearch
		 * Creates the JSON messages and PUTS them to elasticsearch
         * Document IDs are created by elasticsearch
         * 
         * @param timestamp     monitoring packet timestamp
         * @param pom_id        POM electronic ID
         * @param run_num       run number
         * @param temperature   monitoring packet temperature
         * @param humidity      monitoring packet humidity
         * @param rate_veto     was a high rate veto active in this packet?
         * @param rates         array of hit rates
		 */
    void mon(long timestamp, int pom_id, int run_num,
             int temperature, int humidity, bool rate_veto, 
             std::array<float, 30> rates);

    /**
		 * Indexes a document to elasticsearch of given type
		 * Creates the JSON message and PUTS it to elasticsearch
         * Document ID is created by elasticsearch
         * 
         * @param index         name of the elasticsearch index
         * @param value         value for this document
		 */
    void value(std::string index, float value);

private:
    /**
		 * Binded to thread creation
		 * Allows us to modify how the worker thread operates and what it does
		 */
    void indexThread();

    /**
		 * Checks if we need to suppress this log
		 */
    bool suppress();

    /**
		 * Indexes a "daqlog" index document to elasticsearch
		 * Creates the JSON message and PUTS it to elasticsearch
         * Document ID is created by elasticsearch
		 * 
		 * @param level         severity level of log
		 * @param message       log Message
		 */
    void logInLock(severity level, std::string message);

    /**
		 * Indexes a single document to elasticsearch
         * Will route the document through "indextime" pipeline if no timestamp
         * 
         * @param index         name of index
         * @param document      JSON document
         * @param timestamp     do documents contain timestamp?
		 */
    void index(std::string index, Json::Value document, bool timestamp);

    /**
		 * Does a bulk indexing of monhits to elasticsearch
         * 
         * @param timestamp     monitoring packet timestamp
         * @param pom_id        POM electronic ID
         * @param rates         std::array of hit rates
		 */
    void indexHits(long timestamp, int pom_id, std::array<float,30> rates);

    /**
		 * Initialise file logging
		 * Opens a logging file and writes reason for switching
         * 
         * @param error         error that caused switch to file logging
		 */
    void initFile(std::string error);

    /**
		 * Generate a filename for the .txt log file
		 * Uses the current time for generation
		 */
    void generateFilename();

    // Control
    boost::asio::io_service fIndex_service; ///< BOOST io_service
    boost::asio::io_service::work fIndex_work; ///< Work for the io_service
    boost::thread_group fIndex_threads;   ///< Group of threads to do the work
    boost::mutex fMutex_e;               ///< Mutex for external threads

    log_mode fMode; ///< What logging mode are we in {ELASTIC, FILE_LOG}
    
    std::vector<std::string> fClient_list; ///< List of clients
    //elasticlient::Client fClient;       ///< The ElasticSearch client as provided by elasticlient library
    Json::StreamWriterBuilder fBuilder; ///< Json writer to stream json object to string
    Json::Value fLog_message;           ///< Json log message used to send logs to elasticsearch

    // Settings
    std::string fProcess_name;
    int fPid;
    std::string fFile_name; ///< file name used when in FILE_LOG mode
    bool fStdoutPrint;      ///< Should we print logs to stdout?

    // Log suppression
    int fLog_counter;                                                ///< Number of logs counter
    std::chrono::time_point<std::chrono::system_clock> fTimer_start; ///< Suppression window start time
};

extern ElasticInterface g_elastic; ///< Global instance of this class

#endif