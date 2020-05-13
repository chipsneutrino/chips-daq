/**
 * HitReceiver - Handler class for the CLB & BBB optical data stream
 */

#include <boost/bind.hpp>

#include "hit_receiver.h"

using boost::asio::ip::udp;

HitReceiver::HitReceiver(std::shared_ptr<boost::asio::io_service> io_service,
    std::shared_ptr<DataHandler> data_handler, int opt_port,
    std::size_t expected_header_size, std::size_t expected_hit_size)
    : Logging {}
    , mode_ { DataMode::Idle }
    , data_handler_ { std::move(data_handler) }
    , socket_optical_ { *io_service, udp::endpoint(udp::v4(), opt_port) }
    , datagram_buffer_ {}
    , data_slot_idx_ { data_handler_->assignNewSlot() }
    , expected_header_size_ { expected_header_size }
    , expected_hit_size_ { expected_hit_size }
{
    setUnitName("HitReceiver[{}]", opt_port);

    // Setup the sockets
    // TODO: make this constant configurable
    socket_optical_.set_option(udp::socket::receive_buffer_size { 33554432 });

    // TODO: make this constant configurable
    datagram_buffer_.resize(65536);
}

void HitReceiver::startData()
{
    log(INFO, "Starting work on socket.");

    mode_ = DataMode::Receiving;
    requestDatagram();
}

void HitReceiver::stopData()
{
    log(INFO, "Stopping work on socket.");

    mode_ = DataMode::Idle;

    /* TODO: there is probably an async_recv request hanging somewhere in the air,
     * although the next incoming hit will consume it, and new ones will not be
     * created, we should not *rely* on it
     * 
     * There is one more caveat:
     * According to the boost::asio manual, the following function can only cancel
     * requests made on the thread that calls it ... so, this will roll back the
     * initial requestDatagram() sent from DAQHandler but not all the subsequent
     * ones which are presumably called from the IO service threads. So far, it
     * seems like the only way to cancel those as well is to completely close the
     * socket.
     * 
     * When there's time, this should be more thoroughly investigated.
     */
    socket_optical_.cancel();
}

void HitReceiver::startRun(std::shared_ptr<DataRun>& run)
{
    mode_ = DataMode::Mining;
    run_ = run;
}

void HitReceiver::stopRun()
{
    mode_ = DataMode::Receiving;
    run_.reset();
}

void HitReceiver::requestDatagram()
{
    using namespace boost::asio::placeholders;
    socket_optical_.async_receive(boost::asio::buffer(datagram_buffer_),
        boost::bind(&HitReceiver::receiveDatagram, this, error, bytes_transferred));
}

void HitReceiver::receiveDatagram(const boost::system::error_code& error, std::size_t size)
{
    bool have_data { true };
    bool should_mine { true };
    bool should_request_more { true };

    if (error) {
        if (error.value() == boost::asio::error::operation_aborted && mode_ == DataMode::Idle) {
            /* Expected when the data is stopped. Do not clutter logs. */
        } else {
            log(WARNING, "Dropping datagram due to socket failure: {} {}", error.value(), error.category().name());
        }

        have_data = false;
    }

    switch (mode_) {
    case DataMode::Idle: // This must be a response to a request that we made earlier. Drop it.
        log(INFO, "Work on socket stopped.");
        should_mine = false;
        should_request_more = false;
        break;
    case DataMode::Receiving: // Getting responses as expected but not doing anything with them.
        should_mine = false;
        should_request_more = true;
        break;
    case DataMode::Mining: // Getting responses as expected and processing them
        should_mine = true;
        should_request_more = true;
        break;
    }

    if (have_data) {
        checkAndProcessDatagram(datagram_buffer_.data(), size, should_mine);
    }

    if (should_request_more) {
        requestDatagram();
    }
}

void HitReceiver::checkAndProcessDatagram(const char* datagram, std::size_t datagram_size, bool do_mine)
{
    // Check the packet has at least a header in it
    if (datagram_size < expected_header_size_) {
        log(WARNING, "Received datagram without header (expected at least {} bytes, got {})",
            expected_header_size_, datagram_size);
        reportBadDatagram();
        return;
    }

    // Check the size of the packet is consistent with opt_packet_header_t + some hits
    const std::size_t remaining_bytes = datagram_size - expected_header_size_;
    const std::ldiv_t div = std::div((long)remaining_bytes, expected_hit_size_);
    if (div.rem != 0) {
        log(WARNING, "Received datagram with invalid body (expected multiple of {} bytes, got {} which has nonzero remainder {})",
            expected_hit_size_, remaining_bytes, div.rem);
        reportBadDatagram();
        return;
    }

    processDatagram(datagram, datagram_size, div.quot, do_mine);
}

void HitReceiver::reportDataStreamGap(const tai_timestamp& gap_end)
{
    // TODO: implement me
}

void HitReceiver::reportBadDatagram()
{
    // TODO: implement me
}

void HitReceiver::reportGoodDatagram(std::uint32_t plane_id, const tai_timestamp& start_time, const tai_timestamp& end_time, std::uint64_t n_hits)
{
    // TODO: implement me
}
