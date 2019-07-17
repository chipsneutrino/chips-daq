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

void BBBController::enableFlasher(float flasher_v)
{
    g_elastic.log(DEBUG, "BBBController Enabling Nanobeacon");  
}

void BBBController::disableFlasher()
{
    g_elastic.log(DEBUG, "BBBController Disabling Nanobeacon");  
}