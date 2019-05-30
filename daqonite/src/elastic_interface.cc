/**
 * ElasticInterface - Interfaces with elasticsearch for logging and monitoring
 * 
 * Notes on elasticsearch
 * - Elasticsearch assigns the document ID randomly
 * - Elasticsearch adds an "indextime" timestamp to every document
 * 
 * 
 * 
 * ElasticInterface::index takes ~50 milliseconds
 * ElasticInterface::indexHits takes ~ 100 milliseconds for the 30 monhits documents
 * 
 * log() from the external view takes ~0.1 milliseconds
 * doc() from the external view takes ~0.05 milliseconds
 * val() from the external view takes ~0.01 milliseconds
 * mon() from the external view takes ~0.05 milliseconds
 * 
 * Timing done with the following code:
 * std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
 * std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
 * auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
 * std::cout << duration << std::endl;  
 */

#include "elastic_interface.h"

ElasticInterface g_elastic; ///< Global instance of this class

ElasticInterface::ElasticInterface() : fMode(ELASTIC), fIndex_work(fIndex_service), fLog_counter(0)
{
    // elasticlient requires a vector of elasticsearch nodes, for now we just add the one
    fClient_list.clear();
    fClient_list.push_back(getenv("ELASTIC_CLIENT"));

    // Set options for JSON string builder
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
        fIndex_threads.create_thread(boost::bind(&ElasticInterface::indexThread, this));
    }
}

void ElasticInterface::log(severity level, std::string message)
{
    fMutex.lock();

    if (suppress()) // suppress logs
        return;

    logInLock(level, message);

    fMutex.unlock();
}

void ElasticInterface::doc(std::string index, Json::Value document, bool timestamp_now)
{
    fMutex.lock();

    if (fMode == ELASTIC) // Only index documents if in ELASTIC mode
    {
        if (timestamp_now) {
            document["timestamp"] = timestamp(); // Add timestamp now rather than elasticsearch indextime
        }

        // Post indexing to the indexing io_service
        fIndex_service.post(boost::bind(&ElasticInterface::index, this, index, document, !timestamp_now));
    }

    fMutex.unlock();    
}

void ElasticInterface::val(std::string index, float value, bool timestamp_now)
{
    fMutex.lock();

    if (fMode == ELASTIC) // Only index values if in ELASTIC mode
    {
        Json::Value document;      // Populate JSON document
        if (timestamp_now) {
            document["timestamp"] = timestamp(); // Add timestamp now rather than elasticsearch indextime
        }
        document["value"] = value; // monitoring value

        // Post indexing to the indexing io_service
        fIndex_service.post(boost::bind(&ElasticInterface::index, this, index, document, !timestamp_now));
    }

    fMutex.unlock();
}

void ElasticInterface::mon(long timestamp, int pom_id, int run_num,
                           int temperature, int humidity, bool rate_veto, 
                           std::array<float, 30> rates)
{
    fMutex.lock();

    if (fMode == ELASTIC) // Only index monitoring packets if in ELASTIC mode
    {
        Json::Value document;                   // Populate daqmon JSON document
        document["timestamp"]   = timestamp;    // timestamp from the monitoring packet
        document["pom"]         = pom_id;       // planar optical module ID
        document["run"]         = run_num;      // run number
        document["temperature"] = temperature;  // planar optical module temperature
        document["humidity"]    = humidity;     // planar optical module humidity
        document["rate_veto"]   = rate_veto;    // High rate veto bool

        // Post indexing to the indexing io_service
        fIndex_service.post(boost::bind(&ElasticInterface::index, this, "daqmon", document, false));

        // Post the bulk indexing to the indexing io_service
        fIndex_service.post(boost::bind(&ElasticInterface::indexHits, this, timestamp, pom_id, rates));
    }

    fMutex.unlock();
}

void ElasticInterface::indexThread()
{
    fIndex_service.run();
}

bool ElasticInterface::suppress()
{
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
            return true;
        }
        else if (fLog_counter > MAX_LOG_RATE)
        {
            logInLock(WARNING, fmt::format("ElasticInterface supressed a rate of: {}", fLog_counter));
            fLog_counter = 0;
        }
        else
        {
            fLog_counter = 0;
        }
    }

    return false;
}

void ElasticInterface::logInLock(severity level, std::string message) 
{
    Json::Value document;                // Populate daqlog JSON document
    document["timestamp"] = timestamp(); // Milliseconds since epoch timestamp
    document["process"] = fProcess_name; // Process name
    document["pid"] = fPid;              // Process ID
    document["severity"] = level;        // severity level
    document["message"] = message;       // log message

    if (fPrint_logs) // Print to stdout if required
    {
        std::cout << "LOG (" << level << "): " << message << std::endl;
    }

    if (fMode == ELASTIC) // Only index if in ELASTIC mode
    {
        // Post indexing to the indexing io_service
        fIndex_service.post(boost::bind(&ElasticInterface::index, this, "daqlog", document, false));
    }
    else if (fMode == FILE_LOG)
    { // Log to File
        std::ofstream file;
        file.open(fFile_name, std::ios_base::app);
        file << Json::writeString(fBuilder, document) << "\n";
        file.close();
    }
}

void ElasticInterface::index(std::string index, Json::Value document, bool add_time)
{
    elasticlient::Client client(fClient_list);  /// Create the elasticsearch client

    // So we need elasticsearch to at an "indextime" timestamp?
    std::string pipeline;
    if (add_time) 
    {
        pipeline = "?pipeline=indextime";
    } else {
        pipeline = "";
    }

    // Now and again the client will not respond, therefore, we try a few times
    for (int attempt=0; attempt<MAX_ATTEMPTS; attempt++) 
    {
        try
        {
            cpr::Response res = client.index(index, "_doc", pipeline, Json::writeString(fBuilder, document));
            if (res.status_code == 201)
            {
                return;
            } else {
                std::cout << res.text << std::endl; // Just print to stdout
            }
        }
        catch(const std::runtime_error &e) {}
    }

    initFile("Exceeded MAX_ATTEMPTS in index()"); // Init file logging as elasticsearch unreachable
}

void ElasticInterface::indexHits(long timestamp, int pom_id, std::array<float,30> rates)
{
    // Now and again the client will not respond, therefore, we try a few times
    std::shared_ptr<elasticlient::Client> client = std::make_shared<elasticlient::Client>(fClient_list);
    elasticlient::Bulk bulkIndexer(client); /// Create the elasticsearch client bulk indexer

    // Populate the bulk data
    elasticlient::SameIndexBulkData data("daqhits", 30);
    for (int i=0; i<30; i++)
    {
        Json::Value document;
        document["timestamp"] = timestamp;     // timestamp from the monitoring packet
        document["pom"] = pom_id;           // planar optical module ID
        document["channel"] = i;
        document["rate"] = rates[i];
        data.indexDocument("_doc", "", Json::writeString(fBuilder, document));
    }

    for (int attempt=0; attempt<MAX_ATTEMPTS; attempt++) 
    {
        try
        {
            size_t errors = bulkIndexer.perform(data);
            if (errors == 0)
            {
                return;
            }
        }
        catch(const std::runtime_error &e) {}
    }

    initFile("Exceeded MAX_ATTEMPTS in bulkIndex()"); // Init file logging as elasticsearch unreachable
}

void ElasticInterface::initFile(std::string error)
{
    fMode = FILE_LOG; // Switch to file logging mode

    generateFilename(); // Generate a log file name

    // Write reason for switching to FILE_LOG mode to the file
    std::ofstream file;
    file.open(fFile_name);
    file << "Reason for file logging: " << error << "\n";
    file.close();
}

void ElasticInterface::generateFilename()
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "../data/%d-%m-%Y-%H-%M-%S-log.txt", timeinfo);
    fFile_name = std::string(buffer);
}