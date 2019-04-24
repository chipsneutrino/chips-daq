/**
 * MonitoringServer - Stores histograms and provides the ROOT THttpServer 
 */

#include "monitoring_server.h"

/// Create a DAQuardian object
MonitoringServer::MonitoringServer(std::string config_file) : 
                                   fProcess_timer(fIO_service, boost::posix_time::millisec(PROCESSRATE)),
                                   fUpdate_timer(fIO_service, boost::posix_time::millisec(UPDATERATE)),
                                   fSocket(fIO_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), PORT)),
                                   server("http:8080") {

	boost::asio::ip::udp::socket::receive_buffer_size option_local(33554432);
	fSocket.set_option(option_local);
	workSocket();

    file = new TFile("../data/type1_run1.root");
    tree = (TTree*)file->Get("CLBOpt_tree");

    workProcess();
    workUpdate();

    tree->Draw("TimeStamp_s>>hpx", "", "", 10000000, first);
    first = (Int_t)tree->GetEntries();
    c1.Update();
    tree->Refresh();

    // Start the io_service
    fIO_service.run();
}

MonitoringServer::~MonitoringServer() {
    // Empty
}

void MonitoringServer::handleSocket(boost::system::error_code const& error, std::size_t size) {
    std::cout << "Received Socket Message!" << std::endl;
    workSocket();
}

void MonitoringServer::workSocket() {
	fSocket.async_receive(boost::asio::buffer(&fBuffer[0], BUFFERSIZE),
						  boost::bind(&MonitoringServer::handleSocket, this,
						  boost::asio::placeholders::error,
						  boost::asio::placeholders::bytes_transferred));
}

void MonitoringServer::workProcess() {
    gSystem->ProcessEvents();
    fProcess_timer.expires_from_now(boost::posix_time::millisec(PROCESSRATE));
    fProcess_timer.async_wait(boost::bind(&MonitoringServer::workProcess, this));
}

void MonitoringServer::handleUpdate() {
    // Update all the plots
    tree->Draw("TimeStamp_s>>+hpx","","",10000000,first);
    first = (Int_t)tree->GetEntries();
    c1.Update();
    tree->Refresh();
}

void MonitoringServer::workUpdate() {
    fIO_service.post(boost::bind(&MonitoringServer::handleUpdate, this));
    fUpdate_timer.expires_from_now(boost::posix_time::millisec(UPDATERATE));
    fUpdate_timer.async_wait(boost::bind(&MonitoringServer::workUpdate, this));
}
