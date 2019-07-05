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

void BBBController::postStart()
{
    io_service_->post(boost::bind(&BBBController::start, this)); 
}

void BBBController::postStop()
{
    io_service_->post(boost::bind(&BBBController::stop, this)); 
}

void BBBController::init()
{
    g_elastic.log(DEBUG, "BBBController::init"); 
}

void BBBController::configure()
{
    g_elastic.log(DEBUG, "BBBController::configure"); 
}

void BBBController::start() 
{
    g_elastic.log(DEBUG, "BBBController::start"); 
}

void BBBController::stop()
{
    g_elastic.log(DEBUG, "BBBController::stop"); 
}