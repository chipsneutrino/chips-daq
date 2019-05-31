#include <thread>

#include <nngpp/protocol/rep0.h>

#include <util/elastic_interface.h>

#include "experiment/states.h"
#include "global.h"
#include "ops_uplink.h"

void OpsUplink::run()
{
    while (running_) {
        try {
            auto sock = nng::rep::open();
            nng::set_opt_recv_timeout(sock, 200);
            sock.listen(OpsMessage::URL);

            OpsMessage message{};

            while (running_) {
                try {
                    sock.recv(nng::view{ &message, sizeof(message) });
                } catch (const nng::exception& e) {
                    switch (e.get_error()) {
                    case nng::error::timedout:
                        continue;
                    default:
                        throw;
                    }
                }

                handleMessage(sock, message);
            }
        } catch (const nng::exception& e) {
            g_elastic.log(DEBUG, "Ops error: {}: {}", e.who(), e.what());
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}

void OpsUplink::handleMessage(nng::socket& sock, const OpsMessage& message)
{
    switch (message.Discriminator) {
    case OpsMessage::StartRun::Discriminator:
        global.sendEvent(OpsCommands::StartRun{});

        if (Experiment::FSM::is_in_state<Experiment::states::StartingRun>()
            || Experiment::FSM::is_in_state<Experiment::states::Run>()) {
            acknowledge(sock, true);
        } else {
            acknowledge(sock, false);
        }

        break;

    case OpsMessage::StopRun::Discriminator:
        global.sendEvent(OpsCommands::StopRun{});

        if (Experiment::FSM::is_in_state<Experiment::states::StoppingRun>()
            || Experiment::FSM::is_in_state<Experiment::states::Ready>()) {
            acknowledge(sock, true);
        } else {
            acknowledge(sock, false);
        }

        break;

    default:
        acknowledge(sock, false);
        break;
    }
}

void OpsUplink::acknowledge(nng::socket& sock, bool ok)
{
    static const bool t = true;
    static const bool f = false;
    static const nng::view ack{ &t, sizeof(t) };
    static const nng::view nak{ &f, sizeof(f) };

    if (ok) {
        sock.send(ack);
    } else {
        sock.send(nak);
    }
}
