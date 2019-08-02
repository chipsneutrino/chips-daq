/**
 * ElasticInterface - Interface to elasticsearch for logging and monitoring
 * 
 */

#include "elastic_interface.h"

ElasticInterface g_elastic; ///< Global instance of this class

ElasticInterface::ElasticInterface()
    : fMode(ELASTIC), fIndex_work(fIndex_service), fLog_counter(0)
{
    // elasticlient requires a vector of elasticsearch nodes, for now we just add the one
    fClient_list.clear();
    fClient_list.push_back(getenv("ELASTIC_CLIENT"));

    fBuilder["commentStyle"] = "None";
    fBuilder["indentation"] = ""; // If you want whitespace-less output
}

ElasticInterface::~ElasticInterface()
{
    fIndex_service.stop();
    fIndex_threads.join_all();
}

void ElasticInterface::init(bool print_logs, bool print_debug, int index_threads)
{
    // Get the application name and pid
    std::ifstream comm("/proc/self/comm");
    getline(comm, fProcess_name);
    fPid = getpid();

    fPrint_logs = print_logs; // stdout settings

    if (print_debug) // if print_debug, then set the elasticlient logging callback function
    {
        elasticlient::setLogFunction(elasticlient_callback);
    }

    for (int threadCount = 0; threadCount < index_threads; threadCount++) // start indexing threads
    {
        fIndex_threads.create_thread(boost::bind(&ElasticInterface::runThread, this));
    }
}

void ElasticInterface::log(severity level, std::string message)
{
    if (fPrint_logs) // Print to stdout if required
    {
        fPrint_mutex.lock();
        fmt::print("LOG ({}): {}\n", level, message);
        fPrint_mutex.unlock();
    }
    fIndex_service.post(boost::bind(&ElasticInterface::logWork, this, level, message, timestamp()));
}

void ElasticInterface::state(std::string process, std::string state)
{
    fIndex_service.post(boost::bind(&ElasticInterface::stateWork, this, process, state, timestamp()));
}

void ElasticInterface::document(std::string index, Json::Value document)
{
    fIndex_service.post(boost::bind(&ElasticInterface::documentWork, this, index, document, timestamp()));
}

void ElasticInterface::value(std::string index, float value)
{
    fIndex_service.post(boost::bind(&ElasticInterface::valueWork, this, index, value, timestamp()));
}

void ElasticInterface::pom(pom_data data)
{
    fIndex_service.post(boost::bind(&ElasticInterface::pomWork, this, data));
}

void ElasticInterface::channel(channel_data data)
{
    fIndex_service.post(boost::bind(&ElasticInterface::channelWork, this, data));
}

void ElasticInterface::run(int run_num, int run_type)
{
    fIndex_service.post(boost::bind(&ElasticInterface::runWork, this, run_num, run_type));
}

void ElasticInterface::logWork(severity level, std::string message, long timestamp)
{
    if (suppress()) // Should we suppress this log?
        return;

    Json::Value document;                // Populate daqlog JSON document
    document["timestamp"] = timestamp;   // Milliseconds since epoch timestamp
    document["process"] = fProcess_name; // Process name
    document["pid"] = fPid;              // Process ID
    document["severity"] = level;        // severity level
    document["message"] = message;       // log message

    if (mode() == ELASTIC) // only ELASTIC mode
    {
        index("daqlog", document); // Index to elasticsearch
    }
    else if (fMode == FILE_LOG)
    { // Log to File
        fWork_mutex.lock();
        std::ofstream file;
        file.open(fFile_name, std::ios_base::app);
        file << Json::writeString(fBuilder, document) << "\n";
        file.close();
        fWork_mutex.unlock();
    }
}

void ElasticInterface::stateWork(std::string process, std::string state, long timestamp) 
{
    Json::Value document;               // Populate daqstate JSON document
    document["timestamp"] = timestamp;  // Milliseconds since epoch timestamp
    document["process"] = process;      // Process name
    document["state"] = state;          // Process state keyword

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
        Json::Value document;              // populate JSON document
        document["timestamp"] = timestamp; // timestamp
        document["value"] = value;         // monitoring value

        index(name, document); // Index to elasticsearch
    }
}

void ElasticInterface::pomWork(pom_data data)
{
    if (mode() == ELASTIC) // only ELASTIC mode
    {
        Json::Value document;                       // populate daqmon JSON document
        document["timestamp"] = data.timestamp;     // timestamp from the monitoring packet
        document["pom"] = data.pom;                 // planar optical module ID
        document["temperature"] = data.temperature; // planar optical module temperature
        document["humidity"] = data.humidity;       // planar optical module humidity
        document["sync"] = data.sync;               // is planar optical module time synced?

        index("monpom", document); // Index to elasticsearch
    }
}

void ElasticInterface::channelWork(channel_data data)
{
    if (mode() == ELASTIC) // only ELASTIC mode
    {
        // Now and again the client will not respond, therefore, we try a few times
        std::shared_ptr<elasticlient::Client> client = std::make_shared<elasticlient::Client>(fClient_list);
        elasticlient::Bulk bulkIndexer(client); /// Create the elasticsearch client bulk indexer

        // Populate the bulk data
        elasticlient::SameIndexBulkData bulk("monchannel", 30);
        for (int i = 0; i < 30; i++)
        {
            Json::Value document;
            document["timestamp"] = data.timestamp; // timestamp from the monitoring packet
            document["pom"] = data.pom;             // planar optical module ID
            document["channel"] = i;
            document["eid"] = 0;
            document["rate"]["rate"] = data.rate[i];
            document["rate"]["veto"] = (bool)data.veto[i];
            bulk.indexDocument("_doc", "", Json::writeString(fBuilder, document));
        }

        for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++)
        {
            try
            {
                size_t errors = bulkIndexer.perform(bulk);
                if (errors == 0)
                {
                    return;
                }
            }
            catch (const std::runtime_error &e)
            {
            }
        }

        initFile("Exceeded MAX_ATTEMPTS in bulkIndex()"); // Init file logging as elasticsearch unreachable
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

    cpr::Response res = cpr::Put(cpr::Url{fClient_list[0] + "_ingest/pipeline/info"},
                                 cpr::Header{{"Content-Type", "application/json"}},
                                 cpr::Body{Json::writeString(fBuilder, body)});

    if (res.status_code == 200)
    {
        return;
    }
    else
    {
        fPrint_mutex.lock();
        fmt::print("{}\n", res.text);
        fPrint_mutex.unlock();
    }
}

void ElasticInterface::runThread()
{
    fIndex_service.run();
}

bool ElasticInterface::suppress()
{
    fWork_mutex.lock();
    if (fLog_counter == 0)
    {
        fTimer_start = std::chrono::system_clock::now();
        fLog_counter++;
    }
    else if (fLog_counter < MAX_LOG_RATE)
    {
        fLog_counter++;
    }
    else
    {
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        int diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - fTimer_start).count();
        if (diff < 1000)
        {
            fLog_counter++;
            fWork_mutex.unlock();
            return true;
        }
        else if (fLog_counter > MAX_LOG_RATE)
        {
            fPrint_mutex.lock();
            fmt::print("ElasticInterface suppressed a rate of: {}\n", fLog_counter);
            fPrint_mutex.unlock();
            fLog_counter = 0;
        }
        else
        {
            fLog_counter = 0;
        }
    }

    fWork_mutex.unlock();
    return false;
}

void ElasticInterface::index(std::string index, Json::Value document)
{
    elasticlient::Client client(fClient_list); /// Create the elasticsearch client

    // Now and again the client will not respond, therefore, we try a few times
    for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++)
    {
        try
        {
            cpr::Response res = client.index(index, "_doc", "?pipeline=info", Json::writeString(fBuilder, document));
            if (res.status_code == 201)
            {
                return;
            }
            else
            {
                fPrint_mutex.lock();
                fmt::print("{}\n", res.text);
                fPrint_mutex.unlock();
            }
        }
        catch (const std::runtime_error &e)
        {
        }
    }

    initFile("Exceeded MAX_ATTEMPTS in index()"); // Init file logging as elasticsearch unreachable
}

void ElasticInterface::initFile(std::string error)
{
    fWork_mutex.lock();

    fMode = FILE_LOG; // Switch to file logging mode

    // Generate a log file name
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "../data/%d-%m-%Y-%H-%M-%S-log.txt", timeinfo);
    fFile_name = std::string(buffer);

    // Write reason for switching to FILE_LOG mode to the file
    std::ofstream file;
    file.open(fFile_name);
    file << "Reason for file logging: " << error << "\n";
    file.close();

    fWork_mutex.unlock();
}