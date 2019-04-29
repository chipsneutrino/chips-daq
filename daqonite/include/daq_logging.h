/**
 * DAQ Logging - Sets up logging for all DAQ applications
 * 
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef DAQ_LOGGING_H_
#define DAQ_LOGGING_H_

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/syslog_backend.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/utility/setup/console.hpp>

#define SYSLOG_SERVER_ADDRESS "localhost"
#define SYSLOG_SERVER_PORT 514

// Complete sink type
typedef boost::log::sinks::synchronous_sink< boost::log::sinks::syslog_backend > sink_t;

inline void init_console_log() {

    boost::log::add_console_log(std::cout, boost::log::keywords::format = ">> %Message%");

    boost::log::core::get()->set_filter
    (
        boost::log::trivial::severity >= boost::log::trivial::info
    );
}

inline void init_syslog_log() {
    boost::shared_ptr< boost::log::core > core = boost::log::core::get();

    // Create a new backend
    boost::shared_ptr< boost::log::sinks::syslog_backend > backend(new boost::log::sinks::syslog_backend(
        boost::log::keywords::facility = boost::log::sinks::syslog::local0,
        boost::log::keywords::use_impl = boost::log::sinks::syslog::udp_socket_based));

    // Setup the target address and port to send syslog messages to
    backend->set_target_address(SYSLOG_SERVER_ADDRESS, SYSLOG_SERVER_PORT);

    // Set the straightforward level translator for the "Severity" attribute of type int
    backend->set_severity_mapper(boost::log::sinks::syslog::direct_severity_mapping<int>("Severity"));

    // Wrap it into the frontend and register in the core.
    core->add_sink(boost::make_shared<sink_t>(backend));
}

#endif
