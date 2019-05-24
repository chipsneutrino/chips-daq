/**
 * CLBHandler - Handler class for the CLB optical data stream
 */

#include "clb_handler.h"

CLBHandler::CLBHandler(boost::asio::io_service* io_service,
    DataHandler* data_handler,
    bool* mode)
    : fData_handler(data_handler)
    , fMode(mode)
    , fBuffer_size(buffer_size_opt)
    , fSocket_optical(*io_service, udp::endpoint(udp::v4(), default_opto_port))
{

    // Setup the sockets
    udp::socket::receive_buffer_size option_clb(33554432);
    fSocket_optical.set_option(option_clb);
}

CLBHandler::~CLBHandler()
{
    // Empty
}

void CLBHandler::workOpticalData()
{
    if (*fMode == true) {
        fSocket_optical.async_receive(boost::asio::buffer(&fBuffer_optical[0], fBuffer_size),
            boost::bind(&CLBHandler::handleOpticalData, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
}

void CLBHandler::handleOpticalData(boost::system::error_code const& error, std::size_t size)
{
    if (!error) {
        // Check the packet has atleast a CLB header in it
        if (size - sizeof(CLBCommonHeader) < 0) {
            g_elastic.log(WARNING, "CLB Handler invalid packet size (expected {}, got {})", sizeof(CLBCommonHeader), size);
            workOpticalData();
            return;
        }

        // Check the size of the packet is consistent with CLBCommonHeader + some hits
        if (((size - sizeof(CLBCommonHeader)) % sizeof(hit_t)) != 0) {
            g_elastic.log(WARNING, "CLB Handler bad packet");
            workOpticalData();
            return;
        }

        // Cast the beggining of the packet to the CLBCommonHeader
        CLBCommonHeader const& header_optical = *static_cast<CLBCommonHeader const*>(static_cast<void const*>(&fBuffer_optical[0]));

        // Check the type of the packet is optical from the CLBCommonHeader
        std::pair<int, std::string> const& type = getType(header_optical);
        if (type.first != OPTO) {
            g_elastic.log(WARNING, "CLB Handler not optical packet (expected {}, got {})", OPTO, type.first);
            workOpticalData();
            return;
        }

        CLBEvent new_event{};

        // Assign the variables we need from the header
        new_event.PomId = header_optical.pomIdentifier();
        new_event.Timestamp_s = header_optical.timeStamp().sec();
        uint32_t time_stamp_ns_ticks = header_optical.timeStamp().tics();

        fData_handler->updateLastApproxTimestamp(new_event.Timestamp_s);
        CLBEventMultiQueue* multi_queue = fData_handler->findCLBOpticalQueue(
            new_event.Timestamp_s + 1e-9 * (time_stamp_ns_ticks * 16));

        if (multi_queue) {
            // FIXME: This is terribly slow and we need to get rid of it!
            std::lock_guard<std::mutex> l{ multi_queue->write_mutex };

            // Find/create queue for this POM
            CLBEventQueue& event_queue = multi_queue->get_queue_for_writing(new_event.PomId);

            // Find the number of hits this packet contains and loop over them all
            const unsigned int num_hits = (size - sizeof(CLBCommonHeader)) / sizeof(hit_t);
            if (num_hits) {
                for (int i = 0; i < (int)num_hits; ++i) {
                    // Cast the hits individually to the hit_t struct
                    const hit_t* const hit = static_cast<const hit_t* const>(static_cast<const void* const>(&fBuffer_optical[0]
                        + sizeof(CLBCommonHeader) + i * sizeof(hit_t)));

                    // Assign the hit channel
                    new_event.Channel = hit->channel;

                    uint8_t time1 = hit->timestamp1;
                    uint8_t time2 = hit->timestamp2;
                    uint8_t time3 = hit->timestamp3;
                    uint8_t time4 = hit->timestamp4;

                    // Need to change the ordering of the bytes to get the correct hit time
                    uint32_t ordered_time = (((uint32_t)time1) << 24) + (((uint32_t)time2) << 16) + (((uint32_t)time3) << 8) + ((uint32_t)time4);
                    new_event.Timestamp_ns = (time_stamp_ns_ticks * 16) + ordered_time;

                    // Assign the hit TOT
                    new_event.Tot = hit->ToT;

                    if (*fMode == true) {
                        event_queue.emplace_back(std::cref(new_event));
                    }
                }
            }
        }

        workOpticalData();
    } else {
        g_elastic.log(WARNING, "CLB Handler packet error");
    }
}
