/**
 * BBBController - BBBController for an individual BBB
 */

#include "bbb_controller.h"

BBBController::BBBController(ControllerConfig config, bool disable_hv)
    : Controller(config, disable_hv)
{
    g_elastic.log(INFO, "Creating BBBController({})", config.eid_); 
}

BBBController::~BBBController() 
{
    fh_transport_destroy(&transport_);
    fh_message_destroy(&msg_);    
}

void BBBController::init()
{
    g_elastic.log(DEBUG, "BBBController({}) Init...", config_.eid_);  
    working_ = true;

    // Connect to the bbb
    fh_connector_t *conn = fh_connector_new_tcp_client(&config_.ipAsString()[0], config_.port_);
    fh_stream_t *stream = fh_connector_connect(conn);

    if (stream == NULL) // Check we have managed to connect
    {
        working_ = false;
        return;
    }
    
    transport_ = fh_transport_new(fh_protocol_new_plain(), stream);
    assert(transport_);

    msg_ = fh_message_new(); // Initialise messaging stack

    //fh_transport_enable_protocol_trace(transport_, stdout);
    //fh_transport_enable_stream_trace(transport_, stdout);

    state_ = Control::Ready; // Set the controller state to Ready
    g_elastic.log(DEBUG, "BBBController({}) Init DONE", config_.eid_); 
    working_ = false;
}

void BBBController::configure()
{
    g_elastic.log(DEBUG, "BBBController({}) Configure...", config_.eid_);
    working_ = true;

    // Send a FH_CONFIGURE command to configure the BBB/uDAQs
    fh_message_init(msg_, FH_CTRL_SERVICE, FH_CONFIGURE);
    fh_transport_send(transport_, msg_);
    fh_transport_receive(transport_, msg_);

    // Check response
    if(fh_message_getType(msg_)!=FH_CTRL_SERVICE || fh_message_getSubtype(msg_)!=FH_CONFIGURE || fh_message_dataLen(msg_)!=0) {	
        g_elastic.log(ERROR, "BBBController Message error!");
	}

    state_ = Control::Configured; // Set the controller state to Configured
    g_elastic.log(DEBUG, "BBBController({}) Configure DONE", config_.eid_); 
    working_ = false;
}

void BBBController::startData() 
{
    g_elastic.log(DEBUG, "BBBController({}) Start Data...", config_.eid_);
    working_ = true;

    state_ = Control::Started; // Set the controller state to Started
    g_elastic.log(DEBUG, "BBBController({}) Start Data DONE", config_.eid_);
    working_ = false;
}

void BBBController::stopData()
{
    g_elastic.log(DEBUG, "BBBController({}) Stop Data...", config_.eid_);
    working_ = true;

    state_ = Control::Configured; // Set the controller state to Configured
    g_elastic.log(DEBUG, "BBBController({}) Stop Data DONE", config_.eid_);
    working_ = false;
}