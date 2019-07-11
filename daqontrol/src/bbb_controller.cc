/**
 * BBBController - BBBController for an individual BBB
 */

#include "bbb_controller.h"

BBBController::BBBController(ControllerConfig config)
    : Controller(config)
{
    g_elastic.log(INFO, "Creating BBBController for BBB:{}", config.eid_); 
}

void BBBController::postInit()
{
    io_service_->post(boost::bind(&BBBController::init, this)); 
}

void BBBController::postConfigure()
{
    io_service_->post(boost::bind(&BBBController::configure, this)); 
}

void BBBController::postStartData()
{
    io_service_->post(boost::bind(&BBBController::startData, this)); 
}

void BBBController::postStopData()
{
    io_service_->post(boost::bind(&BBBController::stopData, this)); 
}

void BBBController::init()
{
    g_elastic.log(DEBUG, "BBBController Init"); 
}

void BBBController::configure()
{
    g_elastic.log(DEBUG, "BBBController Configure"); 
}

void BBBController::startData() 
{
    g_elastic.log(DEBUG, "BBBController Start Data"); 
}

void BBBController::stopData()
{
    g_elastic.log(DEBUG, "BBBController Stop Data"); 
}