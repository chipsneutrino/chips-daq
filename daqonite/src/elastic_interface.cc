/**
 * ElasticInterface - Interfaces with elasticsearch for logging and monitoring
 * 
 * TODO: Implement file logging if elasticsearch unavailable
 * TODO: Supression of repeating logs
 * TODO: Retry connection to elasticsearch
 * TODO: How do I want to use the configuration for monitoring/alerting
 */

#include "elastic_interface.h"

ElasticInterface g_elastic; ///< Global instance of this class

/// Create a ElasticInterface
ElasticInterface::ElasticInterface()
    : fClient({ CLIENT })
    , fMode(ELASTIC)
{
    fBuilder["indentation"] = ""; // If you want whitespace-less output
}

/// Destroy a ElasticInterface
ElasticInterface::~ElasticInterface()
{
    // Empty
}

/// Initialises the elasticsearch interface
void ElasticInterface::init(std::string processName, bool stdoutPrint, bool commsLog)
{
    srand(time(NULL));

    fStdoutPrint = stdoutPrint;

    //initFile("test", false);

    fLog_message["process"] = processName; // Process name
    fLog_message["pid"] = getpid(); // Process ID

    if (commsLog) {
        elasticlient::setLogFunction(elasticlient_callback);
    }
}

/// Indexes a "daqlog" index document to elasticsearch
void ElasticInterface::log(severity level, std::string message)
{

    fMutex.lock();

    // Get time since epoch in ms
    long value_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::time_point_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now())
            .time_since_epoch())
                        .count();

    // Populate JSON message
    fLog_message["@timestamp"] = value_ms; // ms timestamp since epoch
    fLog_message["severity"] = level; // severity level
    fLog_message["message"] = message; // log message

    // Print to stdout if required
    if (fStdoutPrint) {
        std::cout << "LOG (" << level << "): " << message << std::endl;
    }

    if (fMode == ELASTIC) { // Log to Elasticsearch
        try {

            cpr::Response response = fClient.index("daqlog", "standard", "",
                Json::writeString(fBuilder, fLog_message));

            // check response
            if (response.status_code != 201) {
                std::cout << "ElasticInterface::log Error: " << response.text << std::endl;
            }

        } catch (std::runtime_error& e) {
            initFile(e.what(), true);
        }
    } else if (fMode == FILE_LOG) { // Log to File
        std::ofstream file;
        file.open(fFile_name, std::ios_base::app);
        file << Json::writeString(fBuilder, fLog_message) << "\n";
        file.close();
    }

    fMutex.unlock();
}

/// Indexes a "daqmon" index document to elasticsearch of type "pommon"
void ElasticInterface::monitoringPacket(int& run_num, int& pom_id, long& timestamp,
    int& temperature, int& humidity,
    std::string& message, int* hits)
{

    fMutex.lock();

    // Only send monitoring packet if the elasticsearch client is up
    if (fMode == ELASTIC) {

        // Get time since epoch in ms
        long value_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::time_point_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now())
                .time_since_epoch())
                            .count();

        // Populate JSON message
        fMonitor_message["@timestamp"] = value_ms; // ms timestamp since epoch
        fMonitor_message["run_num"] = run_num; // run number
        fMonitor_message["pom_id"] = pom_id; // planar optical module ID
        fMonitor_message["packet_time"] = timestamp; // timestamp from the monitoring packet
        fMonitor_message["temperature"] = temperature; // planar optical module temperature
        fMonitor_message["humidity"] = humidity; // planar optical module humidity
        fMonitor_message["message"] = message; // optional message
        for (int i = 0; i < 30; i++) {
            fMonitor_message["hits"][i] = hits[i]; // hits on each channel
        }

        // Index message to Elasticsearch
        try {

            cpr::Response response = fClient.index("daqmon", "pommon", "",
                Json::writeString(fBuilder, fMonitor_message));

            // Check response
            if (response.status_code != 201) {
                std::cout << "ElasticInterface::monitoringPacket: " << response.text << std::endl;
            }

        } catch (std::runtime_error& e) {
            initFile(e.what(), false);
        }
    }

    fMutex.unlock();
}

/// Indexes a document to elasticsearch of given type
void ElasticInterface::monitoringValue(std::string index, std::string type, float value)
{

    fMutex.lock(); // lock mutex

    // Only send monitoring packet if the elasticsearch client is up
    if (fMode == ELASTIC) {

        // Get time since epoch in ms
        long value_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::time_point_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now())
                .time_since_epoch())
                            .count();

        // Populate JSON message
        Json::Value message;
        fMonitor_message["@timestamp"] = value_ms; // ms timestamp since epoch
        fMonitor_message["value"] = value; // monitoring value

        // Index message to Elasticsearch
        try {

            cpr::Response response = fClient.index(index, type, "",
                Json::writeString(fBuilder, message));

            // Check response
            if (response.status_code != 201) {
                std::cout << "ElasticInterface::monitoringPacket: " << response.text << std::endl;
            }

        } catch (std::runtime_error& e) {
            initFile(e.what(), false);
        }
    }

    fMutex.unlock(); // unlock mutex
}

/// Initialise file logging
void ElasticInterface::initFile(std::string error, bool writeLog)
{
    // Switch to file logging mode
    fMode = FILE_LOG;

    // Generate a log file name
    generateFilename();

    std::ofstream file;
    file.open(fFile_name);

    // Write reason for switching
    file << "Reason for file logging: " << error << "\n";

    // Write current log to file
    if (writeLog) {
        file << Json::writeString(fBuilder, fLog_message) << "\n";
    }

    // Close the file
    file.close();
}

/// Generate a filename for the .txt log file
void ElasticInterface::generateFilename()
{
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "../data/%d-%m-%Y-%H-%M-%S-log.txt", timeinfo);
    fFile_name = std::string(buffer);
}