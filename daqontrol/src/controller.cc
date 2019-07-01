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

void Controller::setInitValues()
{
    std::vector<int>  var_ids;
    std::vector<long> var_values;

    unsigned char  addr4[4] = {192, 168, 11, 1};  // Server IP Address
    long ipi = ((0xFF & addr4[0]) << 24) |  ((0xFF & addr4[1]) << 16) |  ((0xFF & addr4[2]) <<  8) |  ((0xFF & addr4[3]) << 0);
    //  printf(">>>>>>    IPv4 : %d   %d.%d.%d.%d\n", ipi, addr4[0],addr4[2],addr4[3],addr4[4] );
    long tw = 1000; // Time Window ms 
    var_ids.push_back(ProcVar::NET_IPMUX_SRV_IP);       var_values.push_back(ipi);
    var_ids.push_back(ProcVar::SYS_TIME_SLICE_DUR);     var_values.push_back(tw);
    var_ids.push_back(ProcVar::OPT_HR_VETO_ENA_CH);     var_values.push_back(0x00000000); // Disable all Channels HR Veto  
    var_ids.push_back(ProcVar::OPT_MULHIT_ENA_CH);      var_values.push_back(0x00000000); // Disable all Channels Multi Hits
    //var_ids.push_back(ProcVar::SYS_STMACH_PKTSIZE);

    /*  
    if(isLED)
    {   // TODO Set a Flag  for LED Runs 
        g_elastic.log(INFO, "Enabling Nanobeacon");
        addNanobeacon( var_ids, var_values);  // To be completed  
    }
    */        

    MsgWriter mw;
    mw.writeU16(var_ids.size());

    for(int ii=0; ii<var_ids.size(); ++ii){
        mw.writeI32(var_ids[ii]);
        mw.writeU32(var_values[ii]);
    }

    g_elastic.log(DEBUG, "Setting Initial Values"); 
    MsgReader mr = processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw);   
}

void Controller::addNanobeacon(std::vector<int> &vid, std::vector<long> &vv)
{
    vid.push_back(ProcVar::OPT_NANO_VOLT);      vv.push_back(60000);                       // TODO Nanobeacon Voltdage should be set somewhere 
    vid.push_back(ProcVar::SYS_SYS_RUN_ENA);    vv.push_back(( 0x70000a0 | 0x8000000 ));   // | 0x8000000 Add  the Nanobeancon enabling bit to SYS_SYS_RUN_ENA 
    // Should probably get the current status or have a value stored???
}

void Controller::disableNanobeacon()
{
    std::vector<int>  var_ids;
    std::vector<long> var_values;
    var_ids.push_back(ProcVar::SYS_SYS_RUN_ENA);    var_values.push_back(( 0x70000a0 )); // Should probably get the current status or have a value stored???

    MsgWriter mw;
    mw.writeU16(var_ids.size());
    for(int ii=0; ii<var_ids.size(); ++ii)
    {
        mw.writeI32(var_ids[ii]);
        mw.writeU32(var_values[ii]);
    }

    g_elastic.log(DEBUG, "Disabling Nanobeacon");  

    MsgReader mr = processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw);   
}

void Controller::disableHV()
{
    MsgWriter mw;
    mw.writeU16(1);
    mw.writeI32(ProcVar::SYS_SYS_DISABLE);
    mw.writeU32((0 | ProcVar::SYS_SYS_DISABLE_PWR_MEAS ));
    
    g_elastic.log(DEBUG, "Disabling HV");  
    MsgReader mr = processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw);  
}

void Controller::clbEvent(int event_id)
{
    // TODO check event id is correct. Event IDs defined in clb_events.h
    MsgWriter mw;
    int subsys = ClbSys::CLB_SUB_ALL;
    mw.writeI8(subsys);
    mw.writeI8(event_id);

    MsgReader mr = processor_.processCommand(MsgTypes::MSG_CLB_EVENT, mw);  
}

void Controller::setPMTs()
{
    // TODO: get the PMT e-IDs and check they match with confog file. 
    // Threshold Always the same. Just add Voltage and enabling code
    std::vector<int>  var_ids;
    std::vector<long> var_values;
    var_ids.push_back(ProcVar::OPT_CHAN_ENABLE);          
    var_ids.push_back(ProcVar::OPT_PMT_HIGHVOLT);

    short int hv[31] = {0};

    // WARNING: Even if we use only 30 PMTS the array should ALWAYS contain 31 elements as the CLB expects them
    // TODO:    Load HV and Enable from Config File
    //          Set Just one channel for now (channel 27)  

    long enable = 1 << 27;
    hv[27] = 120;

    MsgWriter mw;
    mw.writeU16(var_ids.size());
    mw.writeI32(var_ids[0]);
    mw.writeU32(enable);
    mw.writeI32(var_ids[1]);
    for(int ipmt=0; ipmt<31; ++ipmt)  mw.writeU8( hv[ipmt]);

    MsgReader mr = processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw);       
}

void Controller::checkPMTs()
{
    // It should check PMT info  match config file or expectations
}  

void Controller::askState()
{
    // Should ask the CLB State (Running, Paused etc.) Not sure how to do that yet 
}

void Controller::askPMTsInfo(int info_type)
{    // Ask for PMT Info. TODO  add method (in msg_processor ?) to actually get the Information 

    if(info_type == ProcVar::OPT_PMT_HIGHVOLT  ||
       info_type == ProcVar::OPT_PMT_THRESHOLD ||
       info_type == ProcVar::OPT_PMT_ID        ||
       info_type == ProcVar::OPT_CHAN_ENABLE   )
    {
        std::vector<int> var_ids;
        var_ids.push_back(info_type);
        MsgWriter mw;
        mw.writeI32Arr(var_ids);

        MsgReader mr = processor_.processCommand(MsgTypes::MSG_CLB_GET_VARS, mw);   
    } else {
        g_elastic.log(ERROR, "askPMTInfo: Wrong Variable ID!");  
    }    
}

void Controller::askVars(std::vector<int> var_ids)
{    // Ask Vars. TODO  add method (in msg_processor ?) to actually get the Information 
    // TODO:  check  var ids ar correct
    MsgWriter mw;
    mw.writeI32Arr(var_ids);

    MsgReader mr = processor_.processCommand(MsgTypes::MSG_CLB_GET_VARS, mw);    
}

