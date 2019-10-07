/**
 * ops_cmd - Application to control FSM
 * 
 * Sends commands over a local IPC socket to FSM application to
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
#include <nngpp/protocol/req0.h>

#include <util/control_msg.h>

namespace ExitCode {
static constexpr int Success = 0;

static constexpr int BadArgs = 1;
static constexpr int UnknownCommand = 2;
static constexpr int NAK = 3;
static constexpr int CommError = 4;
} // namespace ExitCode

int main(int argc, char* argv[])
{
    // Check the command and additional arguments are valid
    if (argc < 3 || argc > 4) {
        std::cerr << "usage: " << argv[0] << " ops_bus_url [ config <opt file> | startData | stopData | startRun N | stopRun | exit ]" << std::endl;
        return ExitCode::BadArgs;
    }

    // Construct a message
    OpsMessage msg {};
    const std::string ops_bus_url { argv[1] };
    const std::string command { argv[2] };

    if (command == "config") {
        if (argc != 4) {
            std::cerr << argv[0] << ": expected a config file" << std::endl;
            return ExitCode::BadArgs;
        }

        msg.Discriminator = OpsMessage::Config::Discriminator;
        strcpy(msg.Payload.pConfig.config_file, argv[3]);
    } else if (command == "startData") {
        msg.Discriminator = OpsMessage::StartData::Discriminator;
    } else if (command == "stopData") {
        msg.Discriminator = OpsMessage::StopData::Discriminator;
    } else if (command == "startRun") {
        if (argc != 4) {
            std::cerr << argv[0] << ": expected a run type [1-4]" << std::endl;
            return ExitCode::BadArgs;
        }
        if (atoi(argv[3]) < 1 || atoi(argv[3]) > 4) {
            std::cerr << argv[0] << ": expected a run type between [1-4]" << std::endl;
            return ExitCode::BadArgs;
        }

        msg.Discriminator = OpsMessage::StartRun::Discriminator;
        msg.Payload.pStartRun.run_type = (RunType)std::atoi(argv[3]);
    } else if (command == "stopRun") {
        msg.Discriminator = OpsMessage::StopRun::Discriminator;
    } else if (command == "exit") {
        msg.Discriminator = OpsMessage::Exit::Discriminator;
    } else {
        std::cerr << argv[0] << ": expected a valid command" << std::endl;
        return ExitCode::UnknownCommand;
    }

    try {
        // Send the message
        auto sock = nng::req::open();
        sock.dial(ops_bus_url.c_str());

        sock.send(nng::view { &msg, sizeof(msg) });

        // Wait for ACK.
        bool ack;
        sock.recv(nng::view { &ack, sizeof(ack) });

        if (ack) {
            return ExitCode::Success;
        } else {
            std::cerr << argv[0] << ": received NAK" << std::endl;
            return ExitCode::NAK;
        }
    } catch (const nng::exception& e) {
        switch (e.get_error()) {
        case nng::error::connrefused:
            std::cerr << argv[0] << ": connection refused, is FSM running?" << std::endl;
            break;

        default:
            std::cerr << argv[0] << ": " << e.who() << ": " << e.what() << std::endl;
            break;
        }

        return ExitCode::CommError;
    }
}