/**
 * DAQHandler - Handler class for combining data streams
 */

#include "daq_handler.h"

DAQHandler::DAQHandler(bool collect_clb_data, bool collect_bbb_data,
    int numThreads)
    : fCollect_clb_data(collect_clb_data)
    , fCollect_bbb_data(collect_bbb_data)
    , fNum_threads(numThreads)
    , fSignal_set(fIO_service, SIGINT)
    , fLocal_socket(fIO_service, udp::endpoint(udp::v4(), 1096))
    , fData_handler()
{

    // Set to not-running mode
    fMode = false;

    // Work the linux signals
    workSignals();

    // Setup and work the local socket (daqommand)
    udp::socket::receive_buffer_size option_local(33554432);
    fLocal_socket.set_option(option_local);
    workLocalSocket();

    // Setup the CLB handler (if required)
    if (fCollect_clb_data) {
        fCLB_handler = new CLBHandler(&fIO_service, &fData_handler, &fMode);
    } else {
        fCLB_handler = NULL;
    }

    // Setup the BBB handler (if required)
    if (fCollect_bbb_data) {
        fBBB_handler = new BBBHandler();
    } else {
        fBBB_handler = NULL;
    }

    // 8) Setup the thread group and call io_service.run() in each
    g_elastic.log(INFO, "DAQ Handler Starting io_service on {} threads", fNum_threads);
    for (int threadCount = 0; threadCount < fNum_threads; threadCount++) {
        fThread_group.create_thread(boost::bind(&DAQHandler::ioServiceThread, this));
    }

    // 9) Wait for all the threads to finish
    fThread_group.join_all();
}

DAQHandler::~DAQHandler()
{
    if (fCLB_handler != NULL) {
        delete fCLB_handler;
    }
    if (fBBB_handler != NULL) {
        delete fBBB_handler;
    }
}

void DAQHandler::ioServiceThread()
{
    fIO_service.run();
}

void DAQHandler::handleSignals(boost::system::error_code const& error, int signum)
{
    if (!error) {
        if (signum == SIGINT) {

            std::cout << "\n";

            if (fMode) {
                g_elastic.log(INFO, "DAQ Handler stopping current mine");

                // Set the mode to monitoring
                fMode = false;

                // Stop the data_handler run
                fData_handler.stopRun();
            }
            fIO_service.stop();

            return;
        }

        // Incase we want to add other signals, you need to call the work method again
        workSignals();
    }
}

void DAQHandler::workSignals()
{
    fSignal_set.async_wait(boost::bind(&DAQHandler::handleSignals, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::signal_number));
}

void DAQHandler::handleLocalSocket(boost::system::error_code const& error, std::size_t size)
{
    if (!error) {
        if (strncmp(fBuffer_local, "start", 5) == 0) {
            // If we are currently running first stop the current run
            if (fMode == true) {
                g_elastic.log(INFO, "DAQ Handler stopping current mine");

                // Set the mode to monitoring
                fMode = false;

                // Stop the data_handler run
                fData_handler.stopRun();
            }

            // Start a data_handler run
            fData_handler.startRun((int)fBuffer_local[5] - 48);

            // Set the mode to data taking
            fMode = true;

            // Call the first work method to the optical data
            fCLB_handler->workOpticalData();
        } else if (strncmp(fBuffer_local, "stop", 4) == 0) {
            // Check we are actually running
            if (fMode == true) {
                // Set the mode to monitoring
                fMode = false;

                // Stop the data_handler run
                fData_handler.stopRun();
            } else {
                g_elastic.log(INFO, "DAQ Handler already stopped mining");
            }
        } else if (strncmp(fBuffer_local, "exit", 4) == 0) {
            if (fMode == true) {
                g_elastic.log(INFO, "DAQ Handler stopping current mine");

                // Set the mode to monitoring
                fMode = false;

                // Stop the data_handler run
                fData_handler.stopRun();
            }
            fIO_service.stop();
            return;
        } else {
            g_elastic.log(INFO, "DAQ Handler received unknown command");
        }

        workLocalSocket();
    }
}

void DAQHandler::workLocalSocket()
{
    fLocal_socket.async_receive(boost::asio::buffer(&fBuffer_local[0], buffer_size_local),
        boost::bind(&DAQHandler::handleLocalSocket, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}
