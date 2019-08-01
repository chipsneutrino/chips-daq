/**
 * BBBController - BBBController for an individual BBB
 */

#include "bbb_controller.h"

BBBController::BBBController(ControllerConfig config)
    : Controller(config)
{
    g_elastic.log(INFO, "Creating BBBController for BBB:{}", config.eid_); 
}

void BBBController::init()
{
    working_ = true;
    state_ = Control::Ready; // Set the controller state to Ready
    g_elastic.log(DEBUG, "BBBController Init"); 
    working_ = false;
}

void BBBController::configure()
{
    working_ = true;
    state_ = Control::Configured; // Set the controller state to Configured
    g_elastic.log(DEBUG, "BBBController Configure"); 
    working_ = false;
}

void BBBController::startData() 
{
    working_ = true;
    state_ = Control::Started; // Set the controller state to Started
    g_elastic.log(DEBUG, "BBBController Start Data"); 
    working_ = false;
}

void BBBController::stopData()
{
    working_ = true;
    state_ = Control::Configured; // Set the controller state to Configured
    g_elastic.log(DEBUG, "BBBController Stop Data"); 
    working_ = false;
}