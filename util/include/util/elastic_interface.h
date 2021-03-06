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

#include <cpr/cpr.h>
#include <cpr/response.h>
#include <elasticlient/bulk.h>
#include <elasticlient/client.h>
#include <elasticlient/logging.h>
#include <fmt/format.h>
#include <json/json.h>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <util/logging.h>
#include <util/timestamp.h>

/// Enum for describing the different logging modes
enum log_mode {
    ELASTIC,
    FILE_LOG
};

/// Struct for the general POM monitoring data
struct pom_data {
    long timestamp;
    int pom;
    short temperature;
    short humidity;
    bool sync;
};

/// Struct for the channel specific monitoring data
struct channel_data {
    long timestamp;
    int pom;
    std::array<float, 30> rate;
    std::bitset<32> veto;
};

#define MAX_LOG_RATE 100
#define MAX_ELASTIC_ATTEMPTS 1

/// Callback for elasticlient logs
inline void elasticlientCallback(elasticlient::LogLevel logLevel, const std::string& msg)
{
    std::cout << "LOG (" << (unsigned)logLevel << "): " << msg << std::endl;
}

class ElasticInterface : protected Logging {
public:
    /// Create a ElasticInterface
    ElasticInterface();

    /// Destroy a ElasticInterface
    ~ElasticInterface();

    void init(const std::string& process_name);

    void stop_and_join();

    /// Unlinks the lock file at exit
    static void close(std::string path);

    /**
     * Adds logWork() work to indexing io_service
     * Timestamp is added when called to maintain ordering
     * Takes ~50 microseconds (~20 without print)
     * @param level         severity level of log
     * @param unit          component from which the message comes
     * @param message       message to log
     */
    void log(Severity level, const std::string& unit, const std::string& message);

    /**
     * Adds stateWork() work to indexing io_service
     * Timestamp is added when called to maintain ordering
     * Takes ~20 microseconds
     * @param process       process name
     * @param state         process state
     */
    void state(std::string process, std::string state);

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

    /**
     * Adds runWork() work to indexing io_service
     * Takes ~20 microseconds
     * @param run_num       run number
     * @param run_type      run type
     */
    void run(int run_num, int run_type);

private:
    /**
     * Indexes "daqlog" document to elasticsearch database
     * @param level         severity level of log
     * @param message       log Message
     * @param timestamp     timestamp when work was posted
     */
    void logWork(Severity level, std::string unit, std::string message, long timestamp);

    /**
     * Indexes "daqstate" document to elasticsearch database
     * @param process       process name
     * @param state         process state
     * @param timestamp     timestamp when work was posted
     */
    void stateWork(std::string process, std::string state, long timestamp);

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

    /**
     * PUTS the _ingest/pipeline/info, to set the run number/type
     * Takes ~100000 microseconds
     * @param run_num       run number
     * @param run_type      run type
     */
    void runWork(int run_num, int run_type);

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
        const auto now { utc_timestamp::now() };
        return static_cast<long>(1e3 * now.combined_secs());
    }

    /// Gets the current ElasticInterface mode
    inline log_mode mode()
    {
        work_mutex_.lock();
        log_mode mode = mode_;
        work_mutex_.unlock();
        return mode;
    }

    /// Sets the ElasticInterface mode
    inline void set_mode(log_mode mode)
    {
        work_mutex_.lock();
        mode_ = mode;
        work_mutex_.unlock();
    }

    // General
    log_mode mode_; ///< Logging mode {ELASTIC, FILE_LOG}
    std::vector<std::string> client_list_; ///< List of elasticsearch clients
    Json::StreamWriterBuilder builder_; ///< Json writer to stream json value to string

    // Indexing
    boost::asio::io_service index_service_; ///< Indexing io_service
    boost::asio::io_service::work index_work_; ///< Work for the indexing io_service
    boost::thread_group index_threads_; ///< Group of indexing threads to do the work
    boost::mutex work_mutex_; ///< Mutex for work inside indexing io_service

    // Settings
    std::string process_name_; ///< Process name for using in log messages
    int pid_; ///< Process pid for using in log messages
    std::string file_name_; ///< file name used when in FILE_LOG mode

    // Log suppression
    int log_counter_; ///< Log counter
    std::chrono::time_point<std::chrono::system_clock> timer_start_; ///< Suppression window start time
};

extern ElasticInterface g_elastic; ///< Global instance of this class
