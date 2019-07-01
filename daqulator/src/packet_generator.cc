/**
 * PacketGenerator - Class to simulate UDP packets from the CLBs
 */

#include "packet_generator.h"

PacketGenerator::PacketGenerator(std::string config_file, std::string dataFile,
                                 std::string address, int time_slice_duration,
                                 int runNum, int MTU, int hitR)
    : fConfig(config_file.c_str()), fFile(dataFile.c_str()), fSock_clb_opt(fIO_service, boost::asio::ip::udp::udp::v4()), fSock_clb_mon(fIO_service, boost::asio::ip::udp::udp::v4()), fTimer(fIO_service, boost::posix_time::millisec(time_slice_duration)), fDelta_ts(time_slice_duration), fHit_dist(hitR * time_slice_duration, (hitR * time_slice_duration) / 10), fTemperature_dist(3000, 500), fHumidity_dist(5000, 500)
{

    // Print the configuration
    fConfig.printShortConfig();

    // Set up the CLB optical output
    boost::asio::ip::udp::udp::resolver resolver_clb_opt(fIO_service);
    boost::asio::ip::udp::udp::resolver::query query_clb_opt(boost::asio::ip::udp::udp::v4(), address,
                                                             boost::lexical_cast<std::string>(56015));
    fCLB_opt_endpoint = *resolver_clb_opt.resolve(query_clb_opt);

    // Set up the CLB monitoring output
    boost::asio::ip::udp::udp::resolver resolver_clb_mon(fIO_service);
    boost::asio::ip::udp::udp::resolver::query query_clb_mon(boost::asio::ip::udp::udp::v4(), address,
                                                             boost::lexical_cast<std::string>(56017));
    fCLB_mon_endpoint = *resolver_clb_mon.resolve(query_clb_mon);

    // Setup the generation vectors
    fCLB_opt_headers.reserve(fConfig.num_controllers_);
    fCLB_mon_headers.reserve(fConfig.num_controllers_);
    fWindow_hits.reserve(fConfig.num_controllers_);

    for (int i = 0; i < fConfig.num_controllers_; i++)
    {
        // Optical packet headers
        CLBCommonHeader header_opt;
        header_opt.RunNumber = htonl(runNum);
        header_opt.DataType = htonl(ttdc);
        header_opt.UDPSequenceNumber = htonl(0);
        header_opt.Timestamp.Sec = htonl(time(0));
        header_opt.Timestamp.Tics = htonl(0);
        header_opt.POMIdentifier = htonl(fConfig.configs_[i].eid_);
        header_opt.POMStatus1 = htonl(0);
        header_opt.POMStatus2 = htonl(0);
        header_opt.POMStatus3 = htonl(0);
        header_opt.POMStatus4 = htonl(0);
        fCLB_opt_headers.push_back(header_opt);

        // Monitoring packet headers
        CLBCommonHeader header_mon;
        header_mon.RunNumber = htonl(runNum);
        header_mon.DataType = htonl(tmch);
        header_mon.UDPSequenceNumber = htonl(0);
        header_mon.Timestamp.Sec = htonl(time(0));
        header_mon.Timestamp.Tics = htonl(0);
        header_mon.POMIdentifier = htonl(fConfig.configs_[i].eid_);
        header_mon.POMStatus1 = htonl(0);
        header_mon.POMStatus2 = htonl(0);
        header_mon.POMStatus3 = htonl(0);
        header_mon.POMStatus4 = htonl(0);
        fCLB_mon_headers.push_back(header_mon);

        // Window hits
        std::array<int, 31> temp;
        fWindow_hits.push_back(temp);
        fWindow_hits[i][30] = 0;
    }

    // Setup generator variables
    fMax_packet_hits = (MTU - sizeof(CLBCommonHeader)) / sizeof(hit_t);
    fMax_payload_size = sizeof(hit_t) * ((MTU - sizeof(CLBCommonHeader)) / sizeof(hit_t));
    fMax_seqnumber = 30 * hitR * sizeof(hit_t) * time_slice_duration / (MTU - sizeof(CLBCommonHeader)) + 1;

    std::cout << "Max_packet_hits: " << fMax_packet_hits << std::endl;
    std::cout << "Max_payload_size: " << fMax_payload_size << std::endl;
    std::cout << "Max_seqnumber: " << fMax_seqnumber << std::endl
              << std::endl;

    // Setup the size of the packets
    fCLB_opt_data.resize(sizeof(CLBCommonHeader) + fMax_payload_size);
    ;
    fCLB_mon_data.resize(sizeof(CLBCommonHeader) + (sizeof(int) * 31) + sizeof(SCData));

    // Setup the monitoring data
    AHRSData ahrs;
    ahrs.yaw = htonl(0.0);   // Yaw in deg (Float)
    ahrs.pitch = htonl(0.0); // Pitch in deg (Float)
    ahrs.roll = htonl(0.0);  // Roll in deg (Float)
    ahrs.ax = htonl(0.0);    // Ax in g (Float)
    ahrs.ay = htonl(0.0);    // Ay in g (Float)
    ahrs.az = htonl(0.0);    // Az in g (Float)
    ahrs.gx = htonl(0.0);    // Gx in deg/sec (Float)
    ahrs.gy = htonl(0.0);    // Gy in deg/sec (Float)
    ahrs.gz = htonl(0.0);    // Gz in deg/sec (Float)
    ahrs.hx = htonl(0.0);    // Hx in gauss (Float)
    ahrs.hy = htonl(0.0);    // Hy in gauss (Float)
    ahrs.hz = htonl(0.0);    // Hz in gauss (Float)

    fMon_data.Valid = htonl(0);
    fMon_data.ahrs = ahrs;
    fMon_data.Temp = htons(0);     // Temperature in 100th of degrees
    fMon_data.Humidity = htonl(0); // Humidity in 100th RH

    // Log the setup to elasticsearch
    g_elastic.log(INFO, "Packet Generator Start");

    // Work the generation
    workGeneration();

    // Start on multiple threads
    boost::thread_group thread_group;
    for (int threadCount = 0; threadCount < 10; threadCount++)
    {
        thread_group.create_thread(boost::bind(&boost::asio::io_service::run, &fIO_service));
    }

    // Wait for all the threads
    thread_group.join_all();
}

PacketGenerator::~PacketGenerator()
{
    fFile.Close();
}

void PacketGenerator::workGeneration()
{
    //std::cout << fTime_taken << std::endl;
    fIO_service.post(boost::bind(&PacketGenerator::generate, this));
    fTimer.expires_from_now(boost::posix_time::millisec(fDelta_ts));
    fTimer.async_wait(boost::bind(&PacketGenerator::workGeneration, this));
}

void PacketGenerator::opticalSend(raw_data_t data)
{
    fSock_clb_opt.send_to(boost::asio::buffer(data), fCLB_opt_endpoint);
}

void PacketGenerator::monitoringSend(raw_data_t data)
{
    fSock_clb_mon.send_to(boost::asio::buffer(data), fCLB_mon_endpoint);
}

void PacketGenerator::generate()
{

    //std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

    // Generate the number of hits on each channel will have this window
    for (int clb = 0; clb < fConfig.num_controllers_; clb++)
    {
        for (int channel = 0; channel < 30; channel++)
        {
            fWindow_hits[clb][channel] = (int)fHit_dist(fGenerator);
        }
    }

    // Send the optical packets
    int seqNum = 0;
    do
    {
        for (int clb = 0; clb < fConfig.num_controllers_; clb++)
        {
            CLBCommonHeader &common_header = fCLB_opt_headers[clb];

            if (ntohl(common_header.UDPSequenceNumber) == (unsigned)fMax_seqnumber)
            {
                // Make a trailer packet
                common_header.POMStatus2 = htonl(128);
                fCLB_opt_data.resize(sizeof(common_header));
            }
            else if (ntohl(common_header.UDPSequenceNumber) == (unsigned)fMax_seqnumber + 1)
            {
                // Reset the header to a normal optical packet and update the timestamp
                common_header.UDPSequenceNumber = htonl(0);
                common_header.POMStatus2 = htonl(0);
                int tics = ntohl(common_header.Timestamp.Tics) + 62500 * fDelta_ts;
                common_header.Timestamp.Tics = htonl(tics);
                if (tics >= 62500000)
                {
                    int secs = ntohl(common_header.Timestamp.Sec) + 1;
                    common_header.Timestamp.Sec = htonl(secs);
                    common_header.Timestamp.Tics = htonl(0);
                }
                fCLB_opt_data.resize(sizeof(CLBCommonHeader) + fMax_payload_size);
                continue;
            }

            // Copy the header to the data to send in the packet and swap the endianness
            memcpy(fCLB_opt_data.data(), &common_header, sizeof(CLBCommonHeader));

            // Send the optical packet
            fIO_service.post(boost::bind(&PacketGenerator::opticalSend, this, fCLB_opt_data));

            // Increment the sequence number for this CLB
            int num = ntohl(common_header.UDPSequenceNumber);
            common_header.UDPSequenceNumber = htonl(num + 1);
        }

        // Increment the sequence number we now send
        seqNum++;

    } while (seqNum < (fMax_seqnumber + 1));

    // Send the monitoring packets
    for (int clb = 0; clb < fConfig.num_controllers_; clb++)
    {
        fMon_data.Temp = htons((short)fTemperature_dist(fGenerator));
        fMon_data.Humidity = htons((short)fHumidity_dist(fGenerator));

        // Update the time in the header
        CLBCommonHeader &common_header = fCLB_mon_headers[clb];
        int tics = ntohl(common_header.Timestamp.Tics) + 62500 * fDelta_ts;
        common_header.Timestamp.Tics = htonl(tics);
        if (tics >= 62500000)
        {
            int secs = ntohl(common_header.Timestamp.Sec) + 1;
            common_header.Timestamp.Sec = htonl(secs);
            common_header.Timestamp.Tics = htonl(0);
        }

        memcpy(fCLB_mon_data.data(), &common_header, sizeof(CLBCommonHeader));
        memcpy(fCLB_mon_data.data() + sizeof(CLBCommonHeader), &fWindow_hits[clb], (sizeof(int) * 31));
        memcpy(fCLB_mon_data.data() + sizeof(CLBCommonHeader) + (sizeof(int) * 31), &fMon_data, sizeof(SCData));

        // Send the monitoring packet
        fIO_service.post(boost::bind(&PacketGenerator::monitoringSend, this, fCLB_mon_data));
    }

    //std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
    //fTime_taken = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
}
