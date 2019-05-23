/**
 * ElasticInterface - Interfaces with elasticsearch for logging and monitoring
 */

#include "elastic_interface.h"

ElasticInterface g_elastic; ///< Global instance of this class

/// Create a ElasticInterface
ElasticInterface::ElasticInterface() : fMode(ELASTIC), fLog_counter(0), fClient({getenv("ELASTIC_CLIENT")})
{
    fBuilder["commentStyle"] = "None";
    fBuilder["indentation"] = ""; // If you want whitespace-less output
    fDebug = false;
}

/// Destroy a ElasticInterface
ElasticInterface::~ElasticInterface()
{
    // Empty
}

/// Initialises the elasticsearch interface
void ElasticInterface::init(std::string processName, bool stdoutPrint, bool debug)
{
    // Setup the procees name and pid
    fLog_message["process"] = processName; // Process name
    fLog_message["pid"] = getpid();        // Process ID

    // stdout settings
    fDebug = debug;
    fStdoutPrint = stdoutPrint;

    if (fDebug)
    {
        elasticlient::setLogFunction(elasticlient_callback);
    }
}

void ElasticInterface::log(severity level, std::string message)
{
    fMutex.lock();
    monitoringLog(level, message);
    fMutex.unlock();
}

/// Indexes a "daqlog" document
void ElasticInterface::monitoringLog(severity level, std::string message)
{
    // Check for suppression
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
            return;
        }
        else if (fLog_counter > MAX_LOG_RATE)
        {
            int tempRate = fLog_counter;
            fLog_counter = 0;
            monitoringLog(WARNING, "ElasticInterface supressed a rate of: " + std::to_string(tempRate));
        }
        else
        {
            fLog_counter = 0;
        }
    }

    // Get time since epoch in ms
    long value_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::time_point_cast<std::chrono::milliseconds>(
                            std::chrono::high_resolution_clock::now())
                            .time_since_epoch())
                        .count();

    // Populate JSON message
    fLog_message["@timestamp"] = value_ms; // ms timestamp since epoch
    fLog_message["severity"] = level;      // severity level
    fLog_message["message"] = message;     // log message

    // Print to stdout if required
    if (fStdoutPrint)
    {
        std::cout << "LOG (" << level << "): " << message << std::endl;
    }

    if (fMode == ELASTIC)
    { // Log to Elasticsearch
        try
        {

            cpr::Response response = fClient.index("daqlog", "_doc", "",
                                                   Json::writeString(fBuilder, fLog_message));

            // check response
            if (response.status_code != 201)
            {
                std::cout << "LOG (4): ElasticInterface::log Error: " << response.status_code << std::endl;
                if (fDebug)
                {
                    std::cout << response.text << std::endl;
                }
            }
        }
        catch (std::runtime_error &e)
        {
            initFile(e.what(), true);
        }
    }
    else if (fMode == FILE_LOG)
    { // Log to File
        std::ofstream file;
        file.open(fFile_name, std::ios_base::app);
        file << Json::writeString(fBuilder, fLog_message) << "\n";
        file.close();
    }
}

void ElasticInterface::packet(int &run_num, int &pom_id, long &timestamp,
                              int &temperature, int &humidity,
                              std::string &message, int *hits)
{

    fMutex.lock();
    monitoringPacket(run_num, pom_id, timestamp, temperature, humidity, message, hits);
    fMutex.unlock();
}

/// Indexes a "daqmon" document
void ElasticInterface::monitoringPacket(int &run_num, int &pom_id, long &timestamp,
                                        int &temperature, int &humidity,
                                        std::string &message, int *hits)
{
    // Only send monitoring packet if the elasticsearch client is up
    if (fMode == ELASTIC)
    {

        // Get time since epoch in ms
        long value_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::time_point_cast<std::chrono::milliseconds>(
                                std::chrono::high_resolution_clock::now())
                                .time_since_epoch())
                            .count();

        // Populate JSON message
        fMonitor_message["@timestamp"] = value_ms;     // ms timestamp since epoch
        fMonitor_message["run_num"] = run_num;         // run number
        fMonitor_message["pom_id"] = pom_id;           // planar optical module ID
        fMonitor_message["packet_time"] = timestamp;   // timestamp from the monitoring packet
        fMonitor_message["temperature"] = temperature; // planar optical module temperature
        fMonitor_message["humidity"] = humidity;       // planar optical module humidity
        fMonitor_message["message"] = message;         // optional message
        for (int i = 0; i < 30; i++)
        {
            fMonitor_message["hits"][i] = hits[i]; // hits on each channel
        }

        // Index message to Elasticsearch
        try
        {

            cpr::Response response = fClient.index("daqmon", "_doc", "",
                                                   Json::writeString(fBuilder, fMonitor_message));

            // Check response
            if (response.status_code != 201)
            {
                monitoringLog(ERROR, "MonitoringPacket Error: " + std::to_string(response.status_code));
                if (fDebug)
                {
                    std::cout << response.text << std::endl;
                }
            }
        }
        catch (std::runtime_error &e)
        {
            initFile(e.what(), false);
        }
    }
}

void ElasticInterface::value(std::string &index, float &value)
{
    fMutex.lock();
    monitoringValue(index, value);
    fMutex.unlock();
}

/// Indexes a document to elasticsearch
void ElasticInterface::monitoringValue(std::string &index, float &value)
{

    // Only send monitoring packet if the elasticsearch client is up
    if (fMode == ELASTIC)
    {

        // Get time since epoch in ms
        long value_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::time_point_cast<std::chrono::milliseconds>(
                                std::chrono::high_resolution_clock::now())
                                .time_since_epoch())
                            .count();

        // Populate JSON message
        Json::Value message;
        fMonitor_message["@timestamp"] = value_ms; // ms timestamp since epoch
        fMonitor_message["value"] = value;         // monitoring value

        // Index message to Elasticsearch
        try
        {

            cpr::Response response = fClient.index(index, "_doc", "",
                                                   Json::writeString(fBuilder, message));

            // Check response
            if (response.status_code != 201)
            {
                monitoringLog(ERROR, "monitoringValue Error: " + std::to_string(response.status_code));
                if (fDebug)
                {
                    std::cout << response.text << std::endl;
                }
            }
        }
        catch (std::runtime_error &e)
        {
            initFile(e.what(), false);
        }
    }
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
    if (writeLog)
    {
        file << Json::writeString(fBuilder, fLog_message) << "\n";
    }

    // Close the file
    file.close();
}

/// Generate a filename for the .txt log file
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