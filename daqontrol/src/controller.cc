/**
 * Controller - Controller for an individual CLB
 *
 */

#include "controller.h"

Controller::Controller(unsigned long ip_address)
    : io_service_{ new boost::asio::io_service }
    , run_work_{ new boost::asio::io_service::work(*io_service_) }
    , thread_([&]{(*io_service_).run();})
    , processor_(ip_address, io_service_)
{
    // Empty
}

Controller::~Controller()
{
    run_work_.reset();
    io_service_->stop();
}

void Controller::postDaterev()
{
    io_service_->post(boost::bind(&Controller::daterev, this)); 
}

void Controller::daterev()
{
    MsgWriter mw;
    MsgReader mr = processor_.processCommand(MsgTypes::MSG_SYS_DATEREV, mw); 

    long hwDateRev = mr.readU32();
	long swDateRev = mr.readU32();

    printf("Hardware Version: %08x\n", hwDateRev);
    printf("Software Version: %08x\n", swDateRev); 
}

////////////////////////////////////////////////////////////
// The get/set var methods have not been tested
// They most likely need the full VarInfo / VarTypes setup
////////////////////////////////////////////////////////////

void Controller::postGetVars(std::vector<int> varIds)
{
    io_service_->post(boost::bind(&Controller::getVars, this, varIds)); 
}

void Controller::getVars(std::vector<int> varIds)
{
    MsgWriter mw;
    mw.writeI32Arr(varIds);

    MsgReader mr = processor_.processCommand(MsgTypes::MSG_CLB_GET_VARS, mw); 
    
    // YOU CAN THEN DECODE THE VARIABLES FROM MSGREADER
}

void Controller::postSetVars(std::map<int, int> toModify)
{
    io_service_->post(boost::bind(&Controller::setVars, this, toModify)); 
}


void Controller::setVars(std::map<int, int> toModify) {
    
    MsgWriter mw;
    
    mw.writeU16(toModify.size());

    std::map<int, int>::iterator it;

    for ( it = toModify.begin(); it != toModify.end(); it++ )
    {
        int varId = it->first;
        mw.writeI32(varId);

        // TODO: This is not really going to work consistently without the VarInfo and VarTypes setup
        int value = it->second;
        mw.writeI32(value);
    }

    MsgReader mr = processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw); 
    
    // YOU CAN THEN DECODE THE VARIABLES FROM MSGREADER
}

