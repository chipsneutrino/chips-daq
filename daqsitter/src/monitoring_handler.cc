/**
 * MonitoringHandler - Reads monitoring packets and forwards them to elasticsearch or file
 */

#include "monitoring_handler.h"

/// Create a MonitoringHandler
MonitoringHandler::MonitoringHandler(std::string config_file, bool save_elastic, 
                                     bool save_file, float sample_frac)
    : save_elastic_(save_elastic)
    , save_file_(save_file)
    , sample_frac_(sample_frac)
    , n_threads_(4)
    , mode_(false)
    , io_service_{ new boost::asio::io_service }
    , clb_socket_(*io_service_, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), CLBMONPORT))
    , bbb_socket_(*io_service_, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), BBBMONPORT))
{
    // Initialise the random number generator
    srand((unsigned)time(NULL));

    if (save_file_)
    {
        // Open the monitoring file to save data to
        std::string fileName = generateFilename();
        g_elastic.log(INFO, "MonitoringHandler Opening monitoring file {}", fileName);

        file_ = new TFile(fileName.c_str(), "RECREATE");
        if (!file_)
        {
            g_elastic.log(FATAL, "MonitoringHandler Could not open file_");
            throw std::runtime_error("MonitoringHandler Could not open file_");
        }

        setupTree();
    }

    // Setup CLB socket
    boost::asio::ip::udp::socket::receive_buffer_size option_clb(33554432);
    clb_socket_.set_option(option_clb);
    workCLBSocket();

    // Setup BBB socket
    boost::asio::ip::udp::socket::receive_buffer_size option_bbb(33554432);
    bbb_socket_.set_option(option_bbb);
    workBBBSocket();
}

void MonitoringHandler::run()
{
    // Setup the thread group and call io_service.run() in each
    g_elastic.log(INFO, "Monitoring Handler starting I/O service on {} threads", n_threads_);
    for (int i = 0; i < n_threads_; ++i) {
        thread_group_.create_thread(boost::bind(&MonitoringHandler::runThread, this));
    }

    // Wait for all the threads to finish
    thread_group_.join_all();

    g_elastic.log(INFO, "Monitoring Handler finished.");
}

/// Generate a filename for the ROOT output file
std::string MonitoringHandler::generateFilename()
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "../data/%d-%m-%Y-%H-%M-%S-monitoring.root", timeinfo);
    return std::string(buffer);
}

/// Setup the ROOT file TTree with the needed branches
void MonitoringHandler::setupTree()
{
    if (file_ != NULL)
    {
        clb_tree_ = new TTree("clb_tree", "clb_tree");
        if (!clb_tree_)
        {
            g_elastic.log(FATAL, "MonitoringHandler Could not create 'clb_tree'");
            throw std::runtime_error("MonitoringHandler Could not create 'clb_tree'");
        }
    }
    else
    {
        g_elastic.log(FATAL, "MonitoringHandler Could not create 'clb_tree' as TFile does not exist");
        throw std::runtime_error("MonitoringHandler Could not create 'clb_tree' as TFile does not exist");
    }

    clb_tree_->Branch("timestamp", &pom_data_.timestamp, "pom_data_.timestamp/l");
    clb_tree_->Branch("pom", &pom_data_.pom, "pom_data_.pom/i");
    clb_tree_->Branch("temperature", &pom_data_.temperature, "pom_data_.temperature/s");
    clb_tree_->Branch("humidity", &pom_data_.humidity, "pom_data_.humidity/s");
    clb_tree_->Branch("sync", &pom_data_.sync, "pom_data_.sync/b");
    clb_tree_->Branch("rate", &channel_data_.rate, "channel_data_.rate[30]/f");
    clb_tree_->Branch("veto", &channel_data_.veto, "channel_data_.veto[32]/b");
}

// Work/Handle the CLB monitoring socket
void MonitoringHandler::workCLBSocket()
{
    clb_socket_.async_receive(boost::asio::buffer(&clb_buffer_[0], BUFFERSIZE),
                              boost::bind(&MonitoringHandler::handleCLBSocket, this,
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred));
}

void MonitoringHandler::handleCLBSocket(boost::system::error_code const &error, std::size_t size) // ~30 microseconds
{

    if (!error)
    {

        // Shall we skip this packet?
        if (((float)rand() / RAND_MAX) > sample_frac_)
        {
            workCLBSocket();
            return;
        }

        // Check the packet is of the correct size
        if (size != clb_mon_size)
        {
            g_elastic.log(WARNING, "MonitoringHandler CLB socket invalid packet size");
            workCLBSocket();
            return;
        }

        // Cast the beggining of the packet to the CLBCommonHeader
        CLBCommonHeader const &header = *static_cast<CLBCommonHeader const *>(static_cast<void const *>(&clb_buffer_[0]));

        // Check the type of the packet is monitoring from the CLBCommonHeader
        if (getType(header).first != MONI)
        {
            g_elastic.log(WARNING, "MonitoringHandler CLB socket incorrect packet type (expected {}, got {})", getType(header).first, MONI);
            workCLBSocket();
            return;
        }

        // Check the timestamp of the packet is valid
        if (!validTimeStamp(header))
        {
            g_elastic.log(WARNING, "MonitoringHandler CLB socket invalid timestamp");
            workCLBSocket();
            return;
        }

        // Cast the next section of the packet to the monitoring hits
        MONHits const &hits =
            *static_cast<MONHits const *>(static_cast<void const *>(&clb_buffer_[0] + sizeof(CLBCommonHeader)));

        // Cast the next section of the packet to the SCData struct
        SCData const &scData =
            *static_cast<SCData const *>(static_cast<void const *>(&clb_buffer_[0] + sizeof(CLBCommonHeader) + sizeof(MONHits)));

        // Fill the mon_data
        pom_data_.timestamp = header.timeStamp().inMilliSeconds();
        pom_data_.pom = header.pomIdentifier();
        pom_data_.temperature = scData.temp();
        pom_data_.humidity = scData.humidity();
        pom_data_.sync = validTimeStamp(header);

        // Fill the rate_data
        channel_data_.timestamp = header.timeStamp().inMilliSeconds();
        channel_data_.pom = header.pomIdentifier();
        channel_data_.veto = hits.vetoBitset();
        float rate_scale = 1000000 / scData.duration(); // Window length in microseconds
        for (int i = 0; i < 30; ++i)
        {
            channel_data_.rate[i] = (float)hits.hit(i) * rate_scale;
        }

        // If we are saving to ROOT file, fill the TTree
        if (save_file_ && clb_tree_ != NULL)
        {
            clb_tree_->Fill();
        }

        // Save the monitoring data to elasticsearch
        if (save_elastic_)
        {
            g_elastic.pom(pom_data_);
            g_elastic.channel(channel_data_);
        }
    }
    else
    {
        g_elastic.log(WARNING, "MonitoringHandler CLB socket packet error");
    }

    workCLBSocket();
}

// Work/Handle the BBB monitoring socket
void MonitoringHandler::workBBBSocket()
{
    bbb_socket_.async_receive(boost::asio::buffer(&bbb_buffer_[0], BUFFERSIZE),
                              boost::bind(&MonitoringHandler::handleBBBSocket, this,
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred));
}

void MonitoringHandler::handleBBBSocket(boost::system::error_code const &error, std::size_t size)
{
    if (!error)
    {
        // Shall we skip this packet?
        if (((float)rand() / RAND_MAX) > sample_frac_)
        {
            workBBBSocket();
            return;
        }
    }
    else
    {
        g_elastic.log(WARNING, "MonitoringHandler BBB socket packet error");
    }
    workBBBSocket();
}

void MonitoringHandler::runThread()
{
    io_service_->run();
}

void MonitoringHandler::handleConfigCommand()
{
    g_elastic.log(INFO, "DAQsitter: Config");
}

void MonitoringHandler::handleStartDataCommand()
{
    g_elastic.log(INFO, "DAQsitter: Starting Data");
    // Check we are not running
    if (mode_ != true) {
        // Set the mode to monitoring
        mode_ = true;

    } else {
        g_elastic.log(INFO, "Monitoring Handler already running");
    }
}

void MonitoringHandler::handleStopDataCommand()
{
    g_elastic.log(INFO, "DAQsitter: Stopping Data");
    // Check we are actually running
    if (mode_ == true) {
        // Set the mode to not monitoring
        mode_ = false;

    } else {
        g_elastic.log(INFO, "Monitoring Handler already not running");
    }
}

void MonitoringHandler::handleStartRunCommand(RunType which)
{
    g_elastic.log(INFO, "DAQsitter: Starting Run");
}

void MonitoringHandler::handleStopRunCommand()
{
    g_elastic.log(INFO, "DAQsitter: Stopping Run");
}

void MonitoringHandler::handleExitCommand()
{
    handleStopRunCommand();
    io_service_->stop();
}

