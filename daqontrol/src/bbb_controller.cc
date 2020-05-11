/**
 * BBBController - BBBController for an individual BBB
 */

#include "bbb_controller.h"

BBBController::BBBController(ControllerConfig config)
    : Controller(config)
{
    g_elastic.log(INFO, "Creating BBBController({})", config.eid_);
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
    g_elastic.log(DEBUG, "BBBController({}) Configure DONE", config_.eid_);
    working_ = false;
}

void BBBController::startData()
{
    g_elastic.log(DEBUG, "BBBController({}) Start Data...", config_.eid_);
    working_ = true;

    // TODO: implement me

    state_ = Control::Started; // Set the controller state to Started
    g_elastic.log(DEBUG, "BBBController({}) Start Data DONE", config_.eid_);
    working_ = false;
}

void BBBController::stopData()
{
    g_elastic.log(DEBUG, "BBBController({}) Stop Data...", config_.eid_);
    working_ = true;

    // TODO: implement me

    state_ = Control::Configured; // Set the controller state to Configured
    g_elastic.log(DEBUG, "BBBController({}) Stop Data DONE", config_.eid_);
    working_ = false;
}