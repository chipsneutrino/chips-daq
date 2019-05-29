/**
 * ElasticInterface - Interfaces with elasticsearch for logging and monitoring
 * 
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#pragma once

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
#include <fmt/format.h>
#include <json/json.h>

#include <boost/thread.hpp>
#include <boost/asio.hpp>

/// Enum for describing the different logging severity levels
enum severity {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

/// Enum for describing the different logging modes
enum log_mode {
    ELASTIC,
    FILE_LOG
};

#define MAX_LOG_RATE 5
#define MAX_ATTEMPTS 3

/// Callback for elasticlient logs
inline void elasticlient_callback(elasticlient::LogLevel logLevel, const std::string& msg)
{
    std::cout << "LOG (" << (unsigned)logLevel << "): " << msg << std::endl;
}

class ElasticInterface {
public:
    /// Create a ElasticInterface
    ElasticInterface();

    /// Destroy a ElasticInterface
    ~ElasticInterface();

    /**
		 * Initialises the ElasticInterface
		 * 
         * @param print_logs    print logs to stdout
         * @param print_debug   print debug messages to stdout
         * @param index_threads number of threads to use for indexing
		 */
    void init(bool print_logs, bool print_debug, int index_threads);

    /// Nice pretty-print formatting using fmt.
    template <typename S, typename... Args>
    inline void log(severity level, const S& format_str, const Args&... args)
    {
        log(level, fmt::format(format_str, args...));
    }

    /**
		 * Indexes a "daqlog" index document to elasticsearch
		 * Checks for suppression and creates JSON document
         * Adds indexing work to PUT it to elasticsearch
		 * 
		 * @param level         severity level of log
		 * @param message       log Message
		 */
    void log(severity level, std::string message);

    /**
		 * Indexes a document to elasticsearch of given type
		 * Adds indexing work to PUT it to elasticsearch
         * 
         * @param index         name of the elasticsearch index
         * @param document      Json::Value document ready to be indexed
		 */
    void doc(std::string index, Json::Value document);

    /**
		 * Indexes a single value document to elasticsearch of given type
		 * Creates the JSON message and adds indexing work to PUT it to elasticsearch
         * 
         * @param index         name of the elasticsearch index
         * @param value         value for this document
		 */
    void val(std::string index, float value);    

    /**
		 * Indexes a "daqmon" document and "daqhits" documents to elasticsearch
		 * Creates the JSON messages and adds indexing work to PUT them to elasticsearch
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
         * @param add_time      should elasticsearch add "indextime" timestamp
		 */
    void index(std::string index, Json::Value document, bool add_time);

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

    // General
    log_mode fMode;                             ///< Logging mode {ELASTIC, FILE_LOG}
    std::vector<std::string> fClient_list;      ///< List of elasticsearch clients
    Json::StreamWriterBuilder fBuilder;         ///< Json writer to stream json value to string

    // Indexing
    boost::asio::io_service fIndex_service;     ///< Indexing io_service
    boost::asio::io_service::work fIndex_work;  ///< Work for the indexing io_service
    boost::thread_group fIndex_threads;         ///< Group of indexing threads to do the work
    boost::mutex fMutex;                        ///< Mutex for posting to indexing io_service

    // Settings
    std::string fProcess_name;                  ///< Process name for using in log messages
    int fPid;                                   ///< Process pid for using in log messages
    std::string fFile_name;                     ///< file name used when in FILE_LOG mode
    bool fPrint_logs;                           ///< Should we print logs to stdout?

    // Log suppression
    int fLog_counter;                                                ///< Log counter
    std::chrono::time_point<std::chrono::system_clock> fTimer_start; ///< Suppression window start time
};

extern ElasticInterface g_elastic; ///< Global instance of this class
