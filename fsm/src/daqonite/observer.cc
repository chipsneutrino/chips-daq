#include <thread>

#include <nngpp/protocol/sub0.h>

#include "daqonite/observer.h"
#include "global.h"
#include <util/control_msg.h>
#include <util/elastic_interface.h>

namespace Daqonite {
void Observer::run()
{
    while (running_) {
        try {
            auto sock = nng::sub::open();
            nng::sub::set_opt_subscribe(sock, "");
            nng::set_opt_recv_timeout(sock, 1000);
            sock.dial(DaqoniteStateMessage::URL);

            global.sendEvent(events::Connected{});

            DaqoniteStateMessage message{};
            while (running_) {
                sock.recv(nng::view{ &message, sizeof(message) });

                if (message.RunMode) {
                    global.sendEvent(events::Mining{});
                } else {
                    global.sendEvent(events::Idle{});
                }
            }
        } catch (const nng::exception& e) {
            g_elastic.log(DEBUG, "Daqonite error: {}: {}", e.who(), e.what());
            global.sendEvent(events::Disconnected{});
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}
}