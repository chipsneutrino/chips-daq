/**
 * DAQommand - Application to control DAQonite
 * 
 * Sends commands over a local UDP socket to a DAQonite application to
 * start and stop runs etc...
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 *
 * Co-author: Petr Manek
 * Contact: pmanek@fnal.gov
 */

#include <iostream>
#include <string>

#include <nngpp/nngpp.h>
#include <nngpp/protocol/pub0.h>

#include "control_msg.h"

namespace exit_code {
static constexpr int success = 0;

static constexpr int bad_args = 1;
static constexpr int unknown_cmd = 2;
}

int main(int argc, char* argv[])
{
    // Check the command and additional arguments are valid
    if (argc < 2 || argc > 3) {
        std::cerr << argv[0] << ": expected a command [ start N | stop | exit ]" << std::endl;
        return exit_code::bad_args;
    }

    // Construct a message
    control_msg::daq msg{};
    const std::string command{ argv[1] };

    if (command == "start") {
        if (argc != 3) {
            std::cerr << argv[0] << ": expected a run type [1-4]" << std::endl;
            return exit_code::bad_args;
        }

        msg.disc = control_msg::daq::start_run::disc_value;

        {
            const std::string run_type_str{ argv[2] };
            msg.payload.p_start_run.which = static_cast<control_msg::daq::start_run::run_type>(std::stoi(run_type_str));
        }
    } else if (command == "stop") {
        msg.disc = control_msg::daq::stop_run::disc_value;
    } else if (command == "exit") {
        msg.disc = control_msg::daq::exit::disc_value;
    } else {
        std::cerr << argv[0] << ": expected a valid command" << std::endl;
        return exit_code::unknown_cmd;
    }

    // Send the message
    auto sock = nng::pub::open();
    sock.listen(control_msg::daq::url);
    sock.send(nng::view{ &msg, sizeof(msg) });

    return exit_code::success;
}