/**
 * BBBController - BBBController for an individual CLB
 */

#pragma once

#include "controller.h"
#include "fh_library.h"
#include <util/elastic_interface.h>

class BBBController: public Controller {
public:
    /// Create a BBBController, calling Controller constructor and created MsgProcessor
    BBBController(ControllerConfig config);

    /// Destroy a BBBController
    ~BBBController() {};

    void postInit() { io_service_->post(boost::bind(&BBBController::init, this)); };
    void postConfigure() { io_service_->post(boost::bind(&BBBController::configure, this)); };
    void postStartData() { io_service_->post(boost::bind(&BBBController::startData, this)); };
    void postStopData() { io_service_->post(boost::bind(&BBBController::stopData, this)); };
    void postFlasherOn(float flasher_v) { io_service_->post(boost::bind(&BBBController::flasherOn, this, flasher_v)); };
    void postFlasherOff() { io_service_->post(boost::bind(&BBBController::flasherOff, this)); };

private:
    void init();
    void configure();
    void startData();
    void stopData();
    void flasherOn(float flasher_v);
    void flasherOff();
};
