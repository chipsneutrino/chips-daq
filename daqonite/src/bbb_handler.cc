/**
 * BBBHandler - Handler class for the BBB data stream
 */

#include "bbb_handler.h"

BBBHandler::BBBHandler()
{
    fServer_ip = (char*)DEFAULT_SERVER_IP;
    fPort = DEFAULT_PORT;

    // initialise messaging stack
    fMsg = fh_message_new();

    this->bbb_connect();
    this->get_bbb_status();
    this->bbb_disconnect();
}

BBBHandler::~BBBHandler()
{
    fh_transport_destroy(&fTransport);
    fh_message_destroy(&fMsg);
}

void BBBHandler::bbb_connect()
{
    fConn = fh_connector_new_tcp_client(fServer_ip, fPort);
    fTransport = fh_transport_new(fh_frame_protocol_new(MAX_MESSAGE_SIZE, FP_VERSION_2), fh_connector_connect(fConn));
    assert(fTransport);
}

void BBBHandler::get_bbb_status()
{
    std::cout << "daqonite - Sending BBB GET_STATUS message" << std::endl;
    fh_message_setType(fMsg, MSG_SERVICE);
    fh_message_setSubtype(fMsg, MS_STATUS);
    fh_transport_send(fTransport, fMsg);
    fh_transport_receive(fTransport, fMsg);
    uint8_t status = *fh_message_getData(fMsg);
    printf("daqonite - Got status message:[%d]\n\n", status);
}

void BBBHandler::bbb_disconnect()
{
    std::cout << "daqonite - Sending BBB MS_CLOSE message" << std::endl;
    fh_message_setType(fMsg, MSG_SERVICE);
    fh_message_setSubtype(fMsg, MS_CLOSE);
    fh_message_setData(fMsg, NULL, 0);
    fh_transport_send(fTransport, fMsg);
    fh_transport_receive(fTransport, fMsg);
    printf("daqonite - BBB client disconnect.\n\n");
}
