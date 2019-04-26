/**
 * MonitoringServer - Stores histograms and provides the ROOT THttpServer 
 */

#include "monitoring_server.h"

/// Create a DAQuardian object
MonitoringServer::MonitoringServer(std::string config_file, 
                                   float generalFrac, float clbFrac, float bbbFrac) : 
                                   fServer("http:8080"),
								   fSignal_set(fIO_service, SIGINT),
                                   fGeneral_socket(fIO_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), GENERALPORT)),
                                   fGeneral_frac(generalFrac),
                                   fCLB_socket(fIO_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), CLBMONPORT)),
                                   fCLB_frac(clbFrac),
                                   fBBB_socket(fIO_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), BBBMONPORT)),
                                   fBBB_frac(bbbFrac),
                                   fProcess_timer(fIO_service, boost::posix_time::millisec(PROCESSRATE)),
                                   fUpdate_timer(fIO_service, boost::posix_time::millisec(UPDATERATE)) {

	// Initialise the random number generator
    srand((unsigned)time(NULL));

	// Open the monitoring file to save data to
	fFile = new TFile(generateFilename().c_str(), "RECREATE");
    if (!fFile) { throw std::runtime_error("Error Opening Output File!"); }

	// Setup general socket
	boost::asio::ip::udp::socket::receive_buffer_size option_general(33554432);
	fGeneral_socket.set_option(option_general);
	workGeneralSocket();

	// Setup CLB socket
	boost::asio::ip::udp::socket::receive_buffer_size option_clb(33554432);
	fCLB_socket.set_option(option_clb);
	workCLBSocket();
	setupTree();

	// Setup BBB socket
	boost::asio::ip::udp::socket::receive_buffer_size option_bbb(33554432);
	fBBB_socket.set_option(option_bbb);
	workBBBSocket();

	// Start ROOT processing
    workROOTProcess();

	// Start plot updating
    workPlotUpdate();

	// Start working signals
	workSignals();

	// Start the BOOST io_service
    std::cout << "Starting io_service ..." << std::endl;
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
	if (!fCLB_tree) { throw std::runtime_error("Error Creating CLB Tree!"); }

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

// Work/Handle the general input socket
void MonitoringServer::workGeneralSocket() {
	fGeneral_socket.async_receive(boost::asio::buffer(&fGeneral_buffer[0], BUFFERSIZE),
						          boost::bind(&MonitoringServer::handleGeneralSocket, this,
						          boost::asio::placeholders::error,
						          boost::asio::placeholders::bytes_transferred));
}

void MonitoringServer::handleGeneralSocket(boost::system::error_code const& error, std::size_t size) {
    std::cout << "handleGeneralSocket" << std::endl;
    if (!error) {
        // Shall we skip this packet?
        if (((float)rand()/RAND_MAX)>fGeneral_frac) {
			workGeneralSocket();
			return;     
        }
    } else {
		std::cout << "Packet Error General Socket" << std::endl;
	}

    workGeneralSocket();
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
			std::cout << "Invalid monitoring packet size: " << size << std::endl;
			workCLBSocket();
			return;
		}

		// Cast the beggining of the packet to the CLBCommonHeader
		CLBCommonHeader const & header =
						*static_cast<CLBCommonHeader const*> (static_cast<void const*> (&fCLB_buffer[0]));

		// Check the type of the packet is monitoring from the CLBCommonHeader
		if (getType(header).first!=MONI) { 
            std::cout << "Incorrect packet type!" << std::endl;
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
		std::cout << "Packet Error CLB Socket" << std::endl;
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
    std::cout << "handleBBBSocket" << std::endl;
    if (!error) {
        // Shall we skip this packet?
        if (((float)rand()/RAND_MAX)>fBBB_frac) {
			workBBBSocket();
			return;     
        }
    } else {
		std::cout << "Packet Error BBB Socket" << std::endl;
	}
    workBBBSocket();
}

// Work/Handle the ROOT Process to keep HTTPServer responsive
void MonitoringServer::workROOTProcess() {
    fProcess_timer.expires_from_now(boost::posix_time::millisec(PROCESSRATE));
    fProcess_timer.async_wait(boost::bind(&MonitoringServer::handleROOTProcess, this));
}

void MonitoringServer::handleROOTProcess() {
    gSystem->ProcessEvents();
    workROOTProcess();
}

// Work/Handle updating the plots
void MonitoringServer::workPlotUpdate() {
    fUpdate_timer.expires_from_now(boost::posix_time::millisec(UPDATERATE));
    fUpdate_timer.async_wait(boost::bind(&MonitoringServer::handlePlotUpdate, this));
}

void MonitoringServer::handlePlotUpdate() {
    // fIO_service.post(boost::bind(&MonitoringServer::handleUpdate, this));
	fCLB_tree->AutoSave("SaveSelf");

	TH1F *test_del = ((TH1F *)(gROOT->FindObject("hits"))); delete test_del;
	fCLB_tree->Draw("temperature:timestamp_s>>hits", "pom_id==816989347", "goff");

    workPlotUpdate();
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
			std::cout << "\nClosing file!" << std::endl;

			fCLB_tree->Write();
			fFile->Close();

			fIO_service.stop();
			return;
		}

		workSignals();
	}
}  