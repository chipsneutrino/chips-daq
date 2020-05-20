/**
 * BBBController - BBBController for an individual BBB
 */

#pragma once

#include <bbb/badgerboard_control.h>
#include <bbb/badgerboard_heartbeat.h>

#include "controller.h"

class BBBController : public Controller {
public:
    /// Create a BBBController, calling Controller constructor and created MsgProcessor
    BBBController(ControllerConfig config);

    /// Destroy a BBBController
    ~BBBController();

    void postReset() { io_service_->post(boost::bind(&BBBController::reset, this)); };
    void postConfigure() { io_service_->post(boost::bind(&BBBController::configure, this)); };
    void postStartData() { io_service_->post(boost::bind(&BBBController::startData, this)); };
    void postStopData() { io_service_->post(boost::bind(&BBBController::stopData, this)); };

private:
    void reset();
    void configure();
    void startData();
    void stopData();

    std::unique_ptr<BadgerboardControl> control_;
    std::unique_ptr<BadgerboardHeartbeat> heartbeat_;

    static bool loadFileContents(std::vector<char>& dest_buffer, const char* file_path); // FIXME: temporary, remove me
};
