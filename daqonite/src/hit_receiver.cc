/**
 * HitReceiver - Handler class for the CLB & BBB optical data stream
 */

#include <boost/bind.hpp>

#include <util/elastic_interface.h>

#include "hit_receiver.h"

using boost::asio::ip::udp;

HitReceiver::HitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
    std::shared_ptr<DataHandler> data_handler, bool* mode, int opt_port,
    int handler_id, std::size_t header_size)
    : data_handler_ { std::move(data_handler) }
    , mode_ { mode }
    , buffer_size_ { BUFFER_SIZE_HITS }
    , socket_optical_ { *io_service, udp::endpoint(udp::v4(), opt_port) }
    , data_slot_idx_ { data_handler_->assignNewSlot() }
    , handler_id_ { handler_id }
    , header_size_ { header_size }
{
    // Setup the sockets
    // TODO: make this constant configurable
    socket_optical_.set_option(udp::socket::receive_buffer_size { 33554432 });
}

void HitReceiver::workOpticalData()
{
    using namespace boost::asio::placeholders;
    socket_optical_.async_receive(boost::asio::buffer(&buffer_optical_[0], buffer_size_),
        boost::bind(&HitReceiver::handleOpticalData, this, error, bytes_transferred));
}

void HitReceiver::handleOpticalData(const boost::system::error_code& error, std::size_t size)
{
    if (error) {
        g_elastic.log(ERROR, "Hit receiver {} caught error {}: {}", handler_id_, error.value(), error.category().name());
        g_elastic.log(WARNING, "Hit receiver {} stopping work on optical socket due to error.", handler_id_);
        return;
    }

    if (*mode_ != true) {
        // We are not in a run so just call the work method again
        workOpticalData();
        return;
    }

    // Check the packet has at least a BBB header in it
    if (size < header_size_) {
        g_elastic.log(WARNING, "Hit receiver {} received packet without header (expected at least {}, got {})",
            handler_id_, header_size_, size);
        workOpticalData();
        return;
    }

    processPacket(&buffer_optical_[0], size);
    workOpticalData();
}
