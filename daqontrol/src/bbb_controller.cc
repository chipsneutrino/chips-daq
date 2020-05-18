/**
 * BBBController - BBBController for an individual BBB
 */

#include "bbb_controller.h"

BBBController::BBBController(ControllerConfig config)
    : Controller { config }
    , badgerboard_ { new Badgerboard }
{
    setUnitName("BBBController[{}]", config.eid_);
    log(INFO, "Creating BBBController({})", config.eid_);
}

BBBController::~BBBController()
{
    // TODO: implement me
}

void BBBController::reset()
{
    // TODO: implement me
}

void BBBController::configure()
{
    // TODO: implement me

    state_ = Control::Configured; // Set the controller state to Configured
    log(DEBUG, "BBBController({}) Configure DONE", config_.eid_);
    working_ = false;
}

void BBBController::startData()
{
    log(DEBUG, "BBBController({}) Start Data...", config_.eid_);
    working_ = true;

    badgerboard_->beginDataRun();
    // TODO: handle errors

    state_ = Control::Started; // Set the controller state to Started
    log(DEBUG, "BBBController({}) Start Data DONE", config_.eid_);
    working_ = false;
}

void BBBController::stopData()
{
    log(DEBUG, "BBBController({}) Stop Data...", config_.eid_);
    working_ = true;

    // TODO: implement me

    state_ = Control::Configured; // Set the controller state to Configured
    log(DEBUG, "BBBController({}) Stop Data DONE", config_.eid_);
    working_ = false;
}
