/**
 * MonitoringServer - Stores histograms and provides the ROOT THttpServer 
 */

#include "monitoring_server.h"

/// Create a DAQuardian object
MonitoringServer::MonitoringServer(std::string config_file,  float clbFrac, float bbbFrac) : 
								   fSignal_set(fIO_service, SIGINT),
                                   fCLB_socket(fIO_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), CLBMONPORT)),
                                   fCLB_frac(clbFrac),
                                   fBBB_socket(fIO_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), BBBMONPORT)),
                                   fBBB_frac(bbbFrac) {

	// Initialise the random number generator
    srand((unsigned)time(NULL));

	// Open the monitoring file to save data to
	std::string fileName = generateFilename();
	g_elastic.log(INFO, "MonitoringServer: Opening fFile " + fileName);

	fFile = new TFile(fileName.c_str(), "RECREATE");
    if (!fFile) {
		g_elastic.log(FATAL, "MonitoringServer: Could not open fFile");
		throw std::runtime_error("MonitoringServer: Could not open fFile"); 
	}

	// Setup CLB socket
	boost::asio::ip::udp::socket::receive_buffer_size option_clb(33554432);
	fCLB_socket.set_option(option_clb);
	workCLBSocket();
	setupTree();

	// Setup BBB socket
	boost::asio::ip::udp::socket::receive_buffer_size option_bbb(33554432);
	fBBB_socket.set_option(option_bbb);
	workBBBSocket();

	// Start working signals
	workSignals();

	// Start the BOOST io_service
    fIO_service.run();
}

MonitoringServer::~MonitoringServer() {
    // Empty
}

// Generate filename
std::string MonitoringServer::generateFilename() {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,80,"../data/%d-%m-%Y-%H-%M-%S-monitoring.root",timeinfo);
    return std::string(buffer);	
}

// Setup the TTree
void MonitoringServer::setupTree() {
	fCLB_tree = new TTree("clb_tree", "clb_tree");
	if (!fCLB_tree) { 
		g_elastic.log(FATAL, "MonitoringServer: Could not open fCLB_tree");
		throw std::runtime_error("MonitoringServer: Could not open fCLB_tree"); 
	}

	// This is where I should read in a configuration file that specified the things I want to monitor
	// FOR NOW WE SHALL HARD CODE THIS!!!

	fCLB_tree->Branch("pom_id", 		&fCLB_pom_id, 		"fCLB_pom_id/i");
	fCLB_tree->Branch("timestamp_s", 	&fCLB_timestamp_s, 	"fCLB_timestamp_s/i");
	fCLB_tree->Branch("pad", 			&fCLB_pad, 			"fCLB_pad/i");
	fCLB_tree->Branch("valid", 			&fCLB_valid, 		"fCLB_valid/i");
	fCLB_tree->Branch("temperature", 	&fCLB_temperature, 	"fCLB_temperature/s");
	fCLB_tree->Branch("humidity", 		&fCLB_humidity, 	"fCLB_humidity/s");
    fCLB_tree->Branch("hits",			&fCLB_hits,			"fCLB_hits[30]/i");
}

// Work/Handle the CLB monitoring socket
void MonitoringServer::workCLBSocket() {
	fCLB_socket.async_receive(boost::asio::buffer(&fCLB_buffer[0], BUFFERSIZE),
						      boost::bind(&MonitoringServer::handleCLBSocket, this,
						      boost::asio::placeholders::error,
						      boost::asio::placeholders::bytes_transferred));
}

void MonitoringServer::handleCLBSocket(boost::system::error_code const& error, std::size_t size) {
	if (!error) {
        // Shall we skip this packet?
        if (((float)rand()/RAND_MAX)>fCLB_frac) {
			workCLBSocket();
			return;           
        }

		// Check the packet has atleast a CLB header in it
		if (size!=clb_max_size) {
			g_elastic.log(WARNING, "MonitoringServer: CLB socket invalid packet size");
			workCLBSocket();
			return;
		}

		// Cast the beggining of the packet to the CLBCommonHeader
		CLBCommonHeader const & header =
						*static_cast<CLBCommonHeader const*> (static_cast<void const*> (&fCLB_buffer[0]));

		// Check the type of the packet is monitoring from the CLBCommonHeader
		if (getType(header).first!=MONI) { 
			g_elastic.log(WARNING, "MonitoringServer: CLB socket incorrect packet type");
			workCLBSocket();
			return;
        }

		fCLB_pom_id = header.pomIdentifier();
		fCLB_timestamp_s = header.timeStamp().sec();

		// Get the monitoring hits data
		for (int i = 0; i < 30; ++i) {
			const uint32_t * const field = static_cast<const uint32_t* const >
									(static_cast<const void* const >(&fCLB_buffer[0] + sizeof(CLBCommonHeader) + i * 4));
			fCLB_hits[i] = htonl(*field);
		}

		// Get the other monitoring info by casting into the SCData struct
		const SCData * const scData =
						static_cast<const SCData* const > (static_cast<const void* const > (&fCLB_buffer[0]
								+ clb_minimum_size));

		fCLB_pad = ntohl(scData->pad);
		fCLB_valid = ntohl(scData->valid);
		fCLB_temperature = (uint16_t)ntohs(scData->temp) / (uint16_t)100.0;
		fCLB_humidity = (uint16_t)ntohs(scData->humidity) / (uint16_t)100.0;

		if (fCLB_tree!=NULL) { fCLB_tree->Fill(); }

	} else {
		g_elastic.log(WARNING, "MonitoringServer: CLB socket packet error");
	}

    workCLBSocket();
}

// Work/Handle the BBB monitoring socket
void MonitoringServer::workBBBSocket() {
	fBBB_socket.async_receive(boost::asio::buffer(&fBBB_buffer[0], BUFFERSIZE),
						      boost::bind(&MonitoringServer::handleBBBSocket, this,
						      boost::asio::placeholders::error,
						      boost::asio::placeholders::bytes_transferred));
}

void MonitoringServer::handleBBBSocket(boost::system::error_code const& error, std::size_t size) {
    if (!error) {
        // Shall we skip this packet?
        if (((float)rand()/RAND_MAX)>fBBB_frac) {
			workBBBSocket();
			return;     
        }
    } else {
		g_elastic.log(WARNING, "MonitoringServer: BBB socket packet error");
	}
    workBBBSocket();
}

// Work/Handle signals
void MonitoringServer::workSignals() {
	fSignal_set.async_wait(boost::bind(&MonitoringServer::handleSignals, this,
					   	   boost::asio::placeholders::error,
					   	   boost::asio::placeholders::signal_number));
}

void MonitoringServer::handleSignals(boost::system::error_code const& error, int signum) {
	if (!error) {
		if (signum == SIGINT) {
			std::cout << "\n";
			g_elastic.log(INFO, "MonitoringServer: Closing fFile");

			fCLB_tree->Write();
			fFile->Close();

			fIO_service.stop();
			return;
		}

		workSignals();
	}
}  