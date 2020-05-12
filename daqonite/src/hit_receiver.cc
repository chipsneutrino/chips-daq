/**
 * HitReceiver - Handler class for the CLB & BBB optical data stream
 */

#include <boost/bind.hpp>

#include "hit_receiver.h"

using boost::asio::ip::udp;

HitReceiver::HitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
    std::shared_ptr<DataHandler> data_handler, bool* mode, int opt_port,
    int handler_id, std::size_t header_size)
    : Logging {}
    , data_handler_ { std::move(data_handler) }
    , mode_ { mode }
    , socket_optical_ { *io_service, udp::endpoint(udp::v4(), opt_port) }
    , datagram_buffer_ {}
    , data_slot_idx_ { data_handler_->assignNewSlot() }
    , handler_id_ { handler_id }
    , header_size_ { header_size }
{
    setUnitName("HitReceiver[{}]", handler_id);

    // Setup the sockets
    // TODO: make this constant configurable
    socket_optical_.set_option(udp::socket::receive_buffer_size { 33554432 });

    // TODO: make this constant configurable
    datagram_buffer_.resize(65536);
}

void HitReceiver::workOpticalData()
{
    using namespace boost::asio::placeholders;
    socket_optical_.async_receive(boost::asio::buffer(datagram_buffer_),
        boost::bind(&HitReceiver::handleOpticalData, this, error, bytes_transferred));
}

void HitReceiver::handleOpticalData(const boost::system::error_code& error, std::size_t size)
{
    if (error) {
        log(ERROR, "Caught error {}: {}", error.value(), error.category().name());
        log(WARNING, "Stopping work on optical socket due to error.");
        return;
    }

    if (*mode_ != true) {
        // We are not in a run so just call the work method again
        workOpticalData();
        return;
    }

    // Check the packet has at least a header in it
    if (size < header_size_) {
        log(WARNING, "Received packet without header (expected at least {}, got {})",
            header_size_, size);
        workOpticalData();
        return;
    }

    processPacket(datagram_buffer_.data(), datagram_buffer_.size());
    workOpticalData();
}
