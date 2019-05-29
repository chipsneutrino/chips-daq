#include <nngpp/nngpp.h>
#include <nngpp/protocol/pub0.h>

#include "control_msg.h"

int main(int argc, char* argv[])
{
    auto sock = nng::pub::open();
    sock.listen(control_msg::daq::url);

    for (;;)
        ;
}