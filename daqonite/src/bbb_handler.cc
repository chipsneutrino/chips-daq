/**
 * BBB_handler - Handler class for the BBB data stream
 */

#include "bbb_handler.h"
#include "assert.h"

BBB_handler::BBB_handler() {

	server_ip = (char*)DEFAULT_SERVER_IP;
	port = DEFAULT_PORT;

	// initialse messaging stack
	msg = fh_message_new();
}

BBB_handler::~BBB_handler() {

	fh_transport_destroy(&transport);
	fh_message_destroy(&msg);
}

void BBB_handler::bbb_connect(){
	// connect to server
	conn = fh_connector_new_tcp_client(server_ip, port);
	transport = fh_transport_new(fh_frame_protocol_new(MAX_MESSAGE_SIZE, FP_VERSION_2), fh_connector_connect(conn));
	assert(transport);
}

void BBB_handler::get_bbb_status(){
	// Send a GET_STATUS message
	std::cout << "Sending GET_STATUS message..." << std::endl;
	fh_message_setType(msg, MSG_SERVICE);
	fh_message_setSubtype(msg, MS_STATUS);
	fh_transport_send(transport, msg);
	fh_transport_receive(transport, msg);
	uint8_t status = *fh_message_getData(msg);
	printf("OK: Got status message:[%d]\n\n", status);
}

void BBB_handler::bbb_disconnect(){
	// send a MS_CLOSE message
	std::cout << "Sending MS_CLOSE message..." << std::endl;
	fh_message_setType(msg, MSG_SERVICE);
	fh_message_setSubtype(msg, MS_CLOSE);
	fh_message_setData(msg, NULL, 0);
	fh_transport_send(transport, msg);
	fh_transport_receive(transport, msg);
	printf("OK: Client disconnect.\n\n");
}
