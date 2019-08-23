/**
 * BBBController - BBBController for an individual CLB
 */

#pragma once

#include "controller.h"
#include "fh_library.h"
#include <util/elastic_interface.h>
#include <bbb_comms_api.h>

class BBBController: public Controller {
public:
    /// Create a BBBController, calling Controller constructor and created MsgProcessor
    BBBController(ControllerConfig config, bool disable_hv);

    /// Destroy a BBBController
    ~BBBController();

    void postInit() { io_service_->post(boost::bind(&BBBController::init, this)); };
    void postConfigure() { io_service_->post(boost::bind(&BBBController::configure, this)); };
    void postStartData() { io_service_->post(boost::bind(&BBBController::startData, this)); };
    void postStopData() { io_service_->post(boost::bind(&BBBController::stopData, this)); };

private:
    void init();
    void configure();
    void startData();
    void stopData();

    fh_transport_t *transport_;
    fh_message_t *msg_;
};
