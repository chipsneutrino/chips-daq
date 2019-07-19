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
    state_ = Control::Idle; // Set the controller state to Configured
    g_elastic.log(DEBUG, "BBBController Init"); 
}

void BBBController::configure()
{
    state_ = Control::Configured; // Set the controller state to Configured
    g_elastic.log(DEBUG, "BBBController Configure"); 
}

void BBBController::startData() 
{
    state_ = Control::Started; // Set the controller state to Started
    g_elastic.log(DEBUG, "BBBController Start Data"); 
}

void BBBController::stopData()
{
    state_ = Control::Configured; // Set the controller state to Configured
    g_elastic.log(DEBUG, "BBBController Stop Data"); 
}