/**
 * MonitoringServer - Reads monitoring packets and forwards them to elasticsearch or file
 */

#include "monitoring_server.h"

/// Create a MonitoringServer
MonitoringServer::MonitoringServer(std::string config_file,
                                   bool save_elastic, bool save_file, float sample_frac)
    : fSave_elastic(save_elastic), fSave_file(save_file), fSample_frac(sample_frac), fSignal_set(fIO_service, SIGINT), fCLB_socket(fIO_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), CLBMONPORT)), fBBB_socket(fIO_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), BBBMONPORT))
{
    // Initialise the random number generator
    srand((unsigned)time(NULL));

    if (fSave_file)
    {
        // Open the monitoring file to save data to
        std::string fileName = generateFilename();
        g_elastic.log(INFO, "MonitoringServer: Opening monitoring file {}", fileName);

        fFile = new TFile(fileName.c_str(), "RECREATE");
        if (!fFile)
        {
            g_elastic.log(FATAL, "MonitoringServer: Could not open fFile");
            throw std::runtime_error("MonitoringServer: Could not open fFile");
        }

        setupTree();
    }

    // Setup CLB socket
    boost::asio::ip::udp::socket::receive_buffer_size option_clb(33554432);
    fCLB_socket.set_option(option_clb);
    workCLBSocket();

    // Setup BBB socket
    boost::asio::ip::udp::socket::receive_buffer_size option_bbb(33554432);
    fBBB_socket.set_option(option_bbb);
    workBBBSocket();

    // Start working signals
    workSignals();

    // Start the BOOST io_service
    fIO_service.run();
}

/// Destroy a MonitoringServer
MonitoringServer::~MonitoringServer()
{
    // Empty
}

/// Generate a filename for the ROOT output file
std::string MonitoringServer::generateFilename()
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
void MonitoringServer::setupTree()
{
    if (fFile != NULL)
    {
        fCLB_tree = new TTree("clb_tree", "clb_tree");
        if (!fCLB_tree)
        {
            g_elastic.log(FATAL, "MonitoringServer: Could not create 'clb_tree'");
            throw std::runtime_error("MonitoringServer: Could not create 'clb_tree'");
        }
    }
    else
    {
        g_elastic.log(FATAL, "MonitoringServer: Could not create 'clb_tree' as TFile does not exist");
        throw std::runtime_error("MonitoringServer: Could not create 'clb_tree' as TFile does not exist");
    }

    fCLB_tree->Branch("timestamp_ms", &fMon_data.timestamp, "fMon_data.timestamp/l");
    fCLB_tree->Branch("pom", &fMon_data.pom, "fMon_data.pom/i");
    fCLB_tree->Branch("run", &fMon_data.run, "fMon_data.run/i");
    fCLB_tree->Branch("rate_veto", &fMon_data.rate_veto, "fMon_data.rate_veto/b");
    fCLB_tree->Branch("temperature", &fMon_data.temperature, "fMon_data.temperature/s");
    fCLB_tree->Branch("humidity", &fMon_data.humidity, "fMon_data.humidity/s");
    fCLB_tree->Branch("rates", &fMon_rates.rates, "fMon_rates.rates[30]/f");
}

// Work/Handle the CLB monitoring socket
void MonitoringServer::workCLBSocket()
{
    fCLB_socket.async_receive(boost::asio::buffer(&fCLB_buffer[0], BUFFERSIZE),
                              boost::bind(&MonitoringServer::handleCLBSocket, this,
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred));
}

void MonitoringServer::handleCLBSocket(boost::system::error_code const &error, std::size_t size) // ~30 microseconds
{

    if (!error)
    {

        // Shall we skip this packet?
        if (((float)rand() / RAND_MAX) > fSample_frac)
        {
            workCLBSocket();
            return;
        }

        // Check the packet is of the correct size
        if (size != clb_mon_size)
        {
            g_elastic.log(WARNING, "MonitoringServer: CLB socket invalid packet size");
            workCLBSocket();
            return;
        }

        // Cast the beggining of the packet to the CLBCommonHeader
        CLBCommonHeader const &header = *static_cast<CLBCommonHeader const *>(static_cast<void const *>(&fCLB_buffer[0]));

        // Check the type of the packet is monitoring from the CLBCommonHeader
        if (getType(header).first != MONI)
        {
            g_elastic.log(WARNING, "MonitoringServer: CLB socket incorrect packet type (expected {}, got {})", getType(header).first, MONI);
            workCLBSocket();
            return;
        }

        // Check the timestamp of the packet is valid
        if (!validTimeStamp(header))
        {
            g_elastic.log(WARNING, "MonitoringServer: CLB socket invalid timestamp");
            workCLBSocket();
            return;
        }

        // Cast the next section of the packet to the monitoring hits
        MONHits const &hits =
            *static_cast<MONHits const *>(static_cast<void const *>(&fCLB_buffer[0] + sizeof(CLBCommonHeader)));

        // Cast the next section of the packet to the SCData struct
        SCData const &scData =
            *static_cast<SCData const *>(static_cast<void const *>(&fCLB_buffer[0] + sizeof(CLBCommonHeader) + sizeof(MONHits)));

        // Fill the mon_data
        fMon_data.timestamp = header.timeStamp().inMilliSeconds();
        fMon_data.pom = header.pomIdentifier();
        fMon_data.run = header.runNumber();
        fMon_data.rate_veto = highRate(hits);
        fMon_data.temperature = scData.temp();
        fMon_data.humidity = scData.humidity();

        // Fill the rate_data
        fMon_rates.timestamp = header.timeStamp().inMilliSeconds();
        fMon_rates.pom = header.pomIdentifier();
        float rate_scale = 1000000 / scData.duration(); // Window length in microseconds
        for (int i = 0; i < 30; ++i)
        {
            fMon_rates.rates[i] = (float)hits.hit(i) * rate_scale;
        }

        // If we are saving to ROOT file, fill the TTree
        if (fSave_file && fCLB_tree != NULL)
        {
            fCLB_tree->Fill();
        }

        // Save the monitoring data to elasticsearch
        if (fSave_elastic)
        {
            g_elastic.val("test", 10.0);
            g_elastic.mon(fMon_data);
            g_elastic.rates(fMon_rates);
        }
    }
    else
    {
        g_elastic.log(WARNING, "MonitoringServer: CLB socket packet error");
    }

    workCLBSocket();
}

// Work/Handle the BBB monitoring socket
void MonitoringServer::workBBBSocket()
{
    fBBB_socket.async_receive(boost::asio::buffer(&fBBB_buffer[0], BUFFERSIZE),
                              boost::bind(&MonitoringServer::handleBBBSocket, this,
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred));
}

void MonitoringServer::handleBBBSocket(boost::system::error_code const &error, std::size_t size)
{
    if (!error)
    {
        // Shall we skip this packet?
        if (((float)rand() / RAND_MAX) > fSample_frac)
        {
            workBBBSocket();
            return;
        }
    }
    else
    {
        g_elastic.log(WARNING, "MonitoringServer: BBB socket packet error");
    }
    workBBBSocket();
}

// Work/Handle signals
void MonitoringServer::workSignals()
{
    fSignal_set.async_wait(boost::bind(&MonitoringServer::handleSignals, this,
                                       boost::asio::placeholders::error,
                                       boost::asio::placeholders::signal_number));
}

void MonitoringServer::handleSignals(boost::system::error_code const &error, int signum)
{
    if (!error)
    {
        if (signum == SIGINT)
        {
            std::cout << "\n";

            if (fSave_file && fFile != NULL && fCLB_tree != NULL)
            {
                g_elastic.log(INFO, "MonitoringServer: Closing monitoring file");
                fCLB_tree->Write();
                fFile->Close();
            }

            fIO_service.stop();
            return;
        }

        workSignals();
    }
}