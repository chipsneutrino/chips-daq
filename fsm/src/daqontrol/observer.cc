#include <thread>

#include <nngpp/protocol/sub0.h>

#include "daqontrol/observer.h"
#include "global.h"
#include <util/control_msg.h>
#include <util/elastic_interface.h>

namespace Daqontrol {
void Observer::run()
{
    while (running_) {
        try {
            auto sock = nng::sub::open();
            nng::sub::set_opt_subscribe(sock, "");
            nng::set_opt_recv_timeout(sock, 2000);
            sock.dial(DaqontrolStateMessage::URL);

            global.sendEvent(events::Connected{});

            DaqontrolStateMessage message{};
            while (running_) {
                sock.recv(nng::view{ &message, sizeof(message) });

                switch (message.Discriminator) {
                case DaqontrolStateMessage::Initialising::Discriminator:
                    global.sendEvent(events::Initialising{});
                    break;
            
                case DaqontrolStateMessage::Ready::Discriminator:
                    global.sendEvent(events::Ready{});
                    break;

                case DaqontrolStateMessage::Configured::Discriminator:
                    global.sendEvent(events::Configured{});
                    break;

                case DaqontrolStateMessage::Started::Discriminator:
                    global.sendEvent(events::Started{});
                    break;

                default:
                    g_elastic.log(WARNING, "Daqontrol received unknown discriminator: {}", message.Discriminator);
                    break;
                }
            }
        } catch (const nng::exception& e) {
            g_elastic.log(DEBUG, "Daqontrol error: {}: {}", e.who(), e.what());
            global.sendEvent(events::Disconnected{});
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}
}
