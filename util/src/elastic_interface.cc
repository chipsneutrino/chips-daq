/**
 * ElasticInterface - Interface to elasticsearch for logging and monitoring
 */

#include <util/config.h>

#include "backtrace_on_terminate.h"
#include "elastic_interface.h"

ElasticInterface g_elastic; ///< Global instance of this class

ElasticInterface::ElasticInterface()
    : Logging {}
    , mode_(ELASTIC)
    , index_work_(index_service_)
    , log_counter_(0)
{
    builder_["commentStyle"] = "None";
    builder_["indentation"] = ""; // If you want whitespace-less output
}

ElasticInterface::~ElasticInterface()
{
    stop_and_join();
}

void ElasticInterface::init(const std::string& process_name)
{
    // elasticlient requires a vector of elasticsearch nodes, for now we just add the one
    client_list_.clear();
    client_list_.push_back(g_config.lookupString("elastic_search.url"));

    // TODO: move these settings to Logging
    const bool print_debug { false };

    const std::uint32_t index_threads { g_config.lookupU32("elastic_search.n_index_threads") };

    // Get the application name and pid
    process_name_ = process_name;
    pid_ = getpid();

    if (print_debug) // if print_debug, then set the elasticlient logging callback function
    {
        elasticlient::setLogFunction(elasticlientCallback);
    }

    for (std::uint32_t threadCount = 0; threadCount < index_threads; threadCount++) // start indexing threads
    {
        index_threads_.create_thread(boost::bind(&ElasticInterface::runThread, this));
    }
}

void ElasticInterface::stop_and_join()
{
    index_service_.stop();
    index_threads_.join_all();
}

void ElasticInterface::log(Severity level, const std::string& unit, const std::string& message)
{
    index_service_.post(boost::bind(&ElasticInterface::logWork, this, level, unit, message, timestamp()));
}

void ElasticInterface::state(std::string process, std::string state)
{
    index_service_.post(boost::bind(&ElasticInterface::stateWork, this, process, state, timestamp()));
}

void ElasticInterface::document(std::string index, Json::Value document)
{
    index_service_.post(boost::bind(&ElasticInterface::documentWork, this, index, document, timestamp()));
}

void ElasticInterface::value(std::string index, float value)
{
    index_service_.post(boost::bind(&ElasticInterface::valueWork, this, index, value, timestamp()));
}

void ElasticInterface::pom(pom_data data)
{
    index_service_.post(boost::bind(&ElasticInterface::pomWork, this, data));
}

void ElasticInterface::channel(channel_data data)
{
    index_service_.post(boost::bind(&ElasticInterface::channelWork, this, data));
}

void ElasticInterface::run(int run_num, int run_type)
{
    index_service_.post(boost::bind(&ElasticInterface::runWork, this, run_num, run_type));
}

void ElasticInterface::logWork(Severity level, std::string unit, std::string message, long timestamp)
{
    if (suppress()) // Should we suppress this log?
        return;

    Json::Value document; // Populate daqlog JSON document
    document["timestamp"] = timestamp; // Milliseconds since epoch timestamp
    document["process"] = process_name_; // Process name
    document["pid"] = pid_; // Process ID
    document["unit"] = unit; // Unit name
    document["severity"] = level; // severity level
    document["message"] = message; // log message

    if (mode() == ELASTIC) // only ELASTIC mode
    {
        index("daqlog", document); // Index to elasticsearch
    } else if (mode_ == FILE_LOG) { // Log to File
        work_mutex_.lock();
        std::ofstream file;
        file.open(file_name_, std::ios_base::app);
        file << Json::writeString(builder_, document) << "\n";
        file.close();
        work_mutex_.unlock();
    }
}

void ElasticInterface::stateWork(std::string process, std::string state, long timestamp)
{
    Json::Value document; // Populate daqstate JSON document
    document["timestamp"] = timestamp; // Milliseconds since epoch timestamp
    document["process"] = process; // Process name
    document["state"] = state; // Process state keyword

    if (mode() == ELASTIC) // only ELASTIC mode
    {
        index("daqstate", document); // Index to elasticsearch
    }
}

void ElasticInterface::documentWork(std::string name, Json::Value document, long timestamp)
{
    if (mode() == ELASTIC) // only ELASTIC mode
    {
        document["timestamp"] = timestamp; // add timestamp

        index(name, document); // Index to elasticsearch
    }
}

void ElasticInterface::valueWork(std::string name, float value, long timestamp)
{
    if (mode() == ELASTIC) // only ELASTIC mode
    {
        Json::Value document; // populate JSON document
        document["timestamp"] = timestamp; // timestamp
        document["value"] = value; // monitoring value

        index(name, document); // Index to elasticsearch
    }
}

void ElasticInterface::pomWork(pom_data data)
{
    if (mode() == ELASTIC) // only ELASTIC mode
    {
        Json::Value document; // populate daqmon JSON document
        document["timestamp"] = data.timestamp; // timestamp from the monitoring packet
        document["pom"] = data.pom; // planar optical module ID
        document["temperature"] = data.temperature; // planar optical module temperature
        document["humidity"] = data.humidity; // planar optical module humidity
        document["sync"] = data.sync; // is planar optical module time synced?

        index("monpom", document); // Index to elasticsearch
    }
}

void ElasticInterface::channelWork(channel_data data)
{
    if (mode() == ELASTIC) // only ELASTIC mode
    {
        // Now and again the client will not respond, therefore, we try a few times
        std::shared_ptr<elasticlient::Client> client = std::make_shared<elasticlient::Client>(client_list_);
        elasticlient::Bulk bulkIndexer(client); /// Create the elasticsearch client bulk indexer

        // Populate the bulk data
        elasticlient::SameIndexBulkData bulk("monchannel", 30);
        for (int i = 0; i < 30; i++) {
            Json::Value document;
            document["timestamp"] = data.timestamp; // timestamp from the monitoring packet
            document["pom"] = data.pom; // planar optical module ID
            document["channel"] = i;
            document["eid"] = 0;
            document["rate"]["rate"] = data.rate[i];
            document["rate"]["veto"] = (bool)data.veto[i];
            bulk.indexDocument("_doc", "", Json::writeString(builder_, document));
        }

        for (int attempt = 0; attempt < MAX_ELASTIC_ATTEMPTS; attempt++) {
            try {
                size_t errors = bulkIndexer.perform(bulk);
                if (errors == 0) {
                    return;
                }
            } catch (const std::runtime_error& e) {
            }
        }

        //initFile("Exceeded MAX_ELASTIC_ATTEMPTS in bulkIndex()"); // Init file logging as elasticsearch unreachable
    }
}

void ElasticInterface::runWork(int run_num, int run_type)
{
    Json::Value body;
    body["description"] = "Adds additional info to the documents";
    body["processors"][0]["set"]["field"] = "run.num";
    body["processors"][0]["set"]["value"] = run_num;
    body["processors"][1]["set"]["field"] = "run.type";
    body["processors"][1]["set"]["value"] = run_type;
    body["processors"][2]["set"]["field"] = "_source.indextime";
    body["processors"][2]["set"]["value"] = "{{_ingest.timestamp}}";

    cpr::Response res = cpr::Put(cpr::Url { client_list_[0] + "_ingest/pipeline/info" },
        cpr::Header { { "Content-Type", "application/json" } },
        cpr::Body { Json::writeString(builder_, body) });

    if (res.status_code != 200) {
        log(WARNING, "Could not index run to ElasticSearch. Response: {}", res.text);
    }
}

void ElasticInterface::runThread()
{
    index_service_.run();
}

bool ElasticInterface::suppress()
{
    work_mutex_.lock();
    if (log_counter_ == 0) {
        timer_start_ = std::chrono::system_clock::now();
        log_counter_++;
    } else if (log_counter_ < MAX_LOG_RATE) {
        log_counter_++;
    } else {
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        int diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - timer_start_).count();
        if (diff < 1000) {
            log_counter_++;
            work_mutex_.unlock();
            return true;
        } else if (log_counter_ > MAX_LOG_RATE) {
            logWithoutES(WARNING, "ElasticInterface suppressed a rate of: {}", log_counter_);
            log_counter_ = 0;
        } else {
            log_counter_ = 0;
        }
    }

    work_mutex_.unlock();
    return false;
}

void ElasticInterface::index(std::string index, Json::Value document)
{
    elasticlient::Client client(client_list_); /// Create the elasticsearch client

    // Now and again the client will not respond, therefore, we try a few times
    for (int attempt = 0; attempt < MAX_ELASTIC_ATTEMPTS; attempt++) {
        try {
            cpr::Response res = client.index(index, "_doc", "?pipeline=info", Json::writeString(builder_, document));
            if (res.status_code == 201) {
                return;
            } else {
                logWithoutES(WARNING, "Failed to index document (received code {}): '{}'",
                    res.status_code, Json::writeString(builder_, document));
            }
        } catch (const std::runtime_error& e) {
            logWithoutES(WARNING, "Failed to index document: '{}', error: {}",
                Json::writeString(builder_, document), e.what());
        }
    }

    //initFile("Exceeded MAX_ELASTIC_ATTEMPTS in index()"); // Init file logging as elasticsearch unreachable
}

void ElasticInterface::initFile(std::string error)
{
    work_mutex_.lock();

    mode_ = FILE_LOG; // Switch to file logging mode

    // Generate a log file name
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "%d-%m-%Y-%H-%M-%S-log.txt", timeinfo);
    file_name_ = std::string(buffer);

    // Write reason for switching to FILE_LOG mode to the file
    std::ofstream file;
    file.open(file_name_);
    file << "Reason for file logging: " << error << "\n";
    file.close();

    work_mutex_.unlock();
}