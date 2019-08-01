#include <thread>

#include <nngpp/protocol/sub0.h>

#include "daqsitter/observer.h"
#include "global.h"
#include <util/control_msg.h>
#include <util/elastic_interface.h>

namespace Daqsitter {
void Observer::run()
{
    while (running_) {
        try {
            auto sock = nng::sub::open();
            nng::sub::set_opt_subscribe(sock, "");
            nng::set_opt_recv_timeout(sock, 1000);
            sock.dial(DaqsitterStateMessage::URL);

            global.sendEvent(events::Connected{});

            DaqsitterStateMessage message{};
            while (running_) {
                sock.recv(nng::view{ &message, sizeof(message) });

                switch (message.Discriminator) {
                case DaqsitterStateMessage::Ready::Discriminator:
                    global.sendEvent(events::Ready{});
                    break;

                case DaqsitterStateMessage::Started::Discriminator:
                    global.sendEvent(events::Started{});
                    break;

                default:
                    g_elastic.log(WARNING, "Daqsitter received unknown discriminator: {}", message.Discriminator);
                    break;
                }
            }
        } catch (const nng::exception& e) {
            g_elastic.log(DEBUG, "Daqsitter error: {}: {}", e.who(), e.what());
            global.sendEvent(events::Disconnected{});
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}
}