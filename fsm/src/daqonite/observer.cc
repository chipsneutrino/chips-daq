#include <thread>

#include <nngpp/protocol/sub0.h>

#include "daqonite/observer.h"
#include "global.h"
#include <util/control_msg.h>
#include <util/elastic_interface.h>

namespace Daqonite {
Observer::Observer(const std::string& bus_url)
    : Logging {}
    , bus_url_ { bus_url }
{
    setUnitName("Daqonite::Observer");
}

void Observer::run()
{
    while (running_) {
        try {
            auto sock = nng::sub::open();
            nng::sub::set_opt_subscribe(sock, "");
            nng::set_opt_recv_timeout(sock, 2000);
            sock.dial(bus_url_.c_str());

            global.sendEvent(events::Connected {});

            DaqoniteStateMessage message {};
            while (running_) {
                sock.recv(nng::view { &message, sizeof(message) });

                switch (message.Discriminator) {
                case DaqoniteStateMessage::Ready::Discriminator:
                    global.sendEvent(events::Ready {});
                    break;

                case DaqoniteStateMessage::Running::Discriminator:
                    global.sendEvent(events::Running {});
                    break;

                default:
                    log(WARNING, "Daqonite received unknown discriminator: {}", message.Discriminator);
                    break;
                }
            }
        } catch (const nng::exception& e) {
            log(DEBUG, "Daqonite error: {}: {}", e.who(), e.what());
            global.sendEvent(events::Disconnected {});
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}
}
