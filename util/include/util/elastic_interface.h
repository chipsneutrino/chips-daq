/**
 * ElasticInterface - Interface to elasticsearch for logging and monitoring
 * 
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#pragma once

#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>

#include <cpr/response.h>
#include <elasticlient/bulk.h>
#include <elasticlient/client.h>
#include <elasticlient/logging.h>
#include <fmt/format.h>
#include <json/json.h>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

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

/// Struct for the general POM monitoring data
struct pom_data
{
    long timestamp;
    int pom;
    short temperature;
    short humidity;
    bool sync;
};

/// Struct for the channel specific monitoring data
struct channel_data
{
    long timestamp;
    int pom;
    std::array<float, 30> rate;
    std::bitset<32> veto;
};

#define MAX_LOG_RATE 20
#define MAX_ATTEMPTS 3

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
		 * Initialises the ElasticInterface
		 * 
         * @param print_logs    print logs to stdout
         * @param print_debug   print debug messages to stdout
         * @param index_threads number of threads to use for indexing
		 */
    void init(bool print_logs, bool print_debug, int index_threads);

    /// Nice pretty-print formatting using fmt for logging
    template <typename S, typename... Args>
    inline void log(severity level, const S &format_str, const Args &... args)
    {
        log(level, fmt::format(format_str, args...));
    }

    /**
		 * Adds logWork() work to indexing io_service
         * Timestamp is added when called to maintain ordering
         * Takes ~50 microseconds (~20 without print)
		 * @param level         severity level of log
		 * @param message       log Message
		 */
    void log(severity level, std::string message);

    /**
		 * Adds stateWork() work to indexing io_service
         * Timestamp is added when called to maintain ordering
         * Takes ~20 microseconds
		 * @param process       process name
		 * @param pid           process pid
         * @param state         process state
		 */
    void state(std::string process, int pid, std::string state);

    /**
		 * Adds documentWork() work to indexing io_service
         * Timestamp is added when called to maintain ordering
         * Takes ~20 microseconds
         * @param index         name of the elasticsearch index
         * @param document      Json::Value document ready to be indexed
		 */
    void document(std::string index, Json::Value document);

    /**
		 * Adds valueWork() work to indexing io_service
         * Timestamp is added when called to maintain ordering
         * Takes ~20 microseconds
         * @param index         name of the elasticsearch index
         * @param value         value for this document
		 */
    void value(std::string index, float value);

    /**
		 * Adds pomWork() work to indexing io_service
         * Takes ~20 microseconds
         * @param data          POM monitoring data
		 */
    void pom(pom_data data);

    /**
		 * Adds channelWork() work to indexing io_service
         * Takes ~20 microseconds
         * @param rates         channel monitoring data
		 */
    void channel(channel_data data);

private:
    /**
		 * Indexes "daqlog" document to elasticsearch database
		 * @param level         severity level of log
		 * @param message       log Message
         * @param timestamp     timestamp when work was posted
		 */
    void logWork(severity level, std::string message, long timestamp);

    /**
		 * Indexes "daqstate" document to elasticsearch database
		 * @param process       process name
		 * @param pid           process pid
         * @param state         process state
         * @param timestamp     timestamp when work was posted
		 */
    void stateWork(std::string process, int pid, std::string state, long timestamp);

    /**
		 * Indexes JSON document to elasticsearch database
		 * @param index         name of the elasticsearch index
		 * @param document      Json::Value document ready to be indexed
         * @param timestamp     timestamp when work was posted
		 */
    void documentWork(std::string name, Json::Value document, long timestamp);

    /**
		 * Indexes value to elasticsearch database
         * @param index         name of the elasticsearch index
         * @param value         value for this document
         * @param timestamp     timestamp when work was posted
		 */
    void valueWork(std::string name, float value, long timestamp);

    /**
		 * Indexes "monpom" document to elasticsearch database
         * @param data          POM monitoring data
		 */
    void pomWork(pom_data data);

    /**
		 * Indexes "monchannel" document to elasticsearch
         * Takes ~100000 microseconds
         * @param rates         channel monitoring data
		 */
    void channelWork(channel_data data);

    /// Calls run() in an indexing thread
    void runThread();

    /// Checks if we need to suppress this log
    bool suppress();

    /**
		 * Indexes a single document to elasticsearch
         * Takes ~75000 microseconds
         * @param index         name of index
         * @param document      JSON document
		 */
    void index(std::string index, Json::Value document);

    /**
		 * Initialise file logging
		 * Opens a logging file and writes reason for switching
         * @param error         error that caused switch to file logging
		 */
    void initFile(std::string error);

    /// Gets current time in milliseconds since the epoch
    inline long timestamp()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    /// Gets the current ElasticInterface mode
    inline log_mode mode()
    {
        fWork_mutex.lock();
        log_mode mode = fMode;
        fWork_mutex.unlock();
        return mode;
    }

    /// Sets the ElasticInterface mode
    inline void set_mode(log_mode mode)
    {
        fWork_mutex.lock();
        fMode = mode;
        fWork_mutex.unlock();
    }

    // General
    log_mode fMode;                        ///< Logging mode {ELASTIC, FILE_LOG}
    std::vector<std::string> fClient_list; ///< List of elasticsearch clients
    Json::StreamWriterBuilder fBuilder;    ///< Json writer to stream json value to string

    // Indexing
    boost::asio::io_service fIndex_service;    ///< Indexing io_service
    boost::asio::io_service::work fIndex_work; ///< Work for the indexing io_service
    boost::thread_group fIndex_threads;        ///< Group of indexing threads to do the work
    boost::mutex fPrint_mutex;                 ///< Mutex for stdout printing
    boost::mutex fWork_mutex;                  ///< Mutex for work inside indexing io_service

    // Settings
    std::string fProcess_name; ///< Process name for using in log messages
    int fPid;                  ///< Process pid for using in log messages
    std::string fFile_name;    ///< file name used when in FILE_LOG mode
    bool fPrint_logs;          ///< Should we print logs to stdout?

    // Log suppression
    int fLog_counter;                                                ///< Log counter
    std::chrono::time_point<std::chrono::system_clock> fTimer_start; ///< Suppression window start time
};

extern ElasticInterface g_elastic; ///< Global instance of this class
