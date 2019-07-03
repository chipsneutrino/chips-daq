/**
 * Controller - Controller for an individual CLB
 *
 */

#include "controller.h"

Controller::Controller(ControllerConfig config)
    : config_(config)
    , io_service_{ new boost::asio::io_service }
    , run_work_{ new boost::asio::io_service::work(*io_service_) }
    , thread_( [&]{(*io_service_).run();} )
    , processor_(config.ip_, io_service_)
{
    // Empty
}

Controller::~Controller()
{
    run_work_.reset();
    io_service_->stop();
}

void Controller::postTest()
{
    io_service_->post(boost::bind(&Controller::test, this)); 
}

void Controller::postInitClb()
{
    io_service_->post(boost::bind(&Controller::initClb, this)); 
}

void Controller::postConfigureClb()
{
    io_service_->post(boost::bind(&Controller::configureClb, this)); 
}

void Controller::postStartClb()
{
    io_service_->post(boost::bind(&Controller::startClb, this)); 
}

void Controller::postStopClb()
{
    io_service_->post(boost::bind(&Controller::stopClb, this)); 
}

void Controller::postQuitClb()
{
    io_service_->post(boost::bind(&Controller::quitClb, this)); 
}

void Controller::postResetClb()
{
    io_service_->post(boost::bind(&Controller::resetClb, this)); 
}

void Controller::postPauseClb()
{
    io_service_->post(boost::bind(&Controller::pauseClb, this)); 
}

void Controller::postContinueClb()
{
    io_service_->post(boost::bind(&Controller::continueClb, this)); 
}



void Controller::test()
{
    // To test the connection we ask for the date of the software revisions
    MsgWriter mw;
    MsgReader mr;
    if(!processor_.processCommand(MsgTypes::MSG_SYS_DATEREV, mw, mr))
    {
        g_elastic.log(WARNING, "Could not get response from CLB in test!"); 
        return;
    }

    long hwDateRev = mr.readU32();
    long swDateRev = mr.readU32();

    g_elastic.log(INFO, "Test successful, hardware({0:8x}), software({0:8x})", hwDateRev, swDateRev); 

    sleep(1);
}

void Controller::initClb()
{
    setInitValues();
    sleep(1);
    clbEvent(ClbEvents::INIT);
    sleep(1);
}

void Controller::configureClb()
{
    setPMTs();
    sleep(1);
    checkPMTs();   
    sleep(1);
    clbEvent(ClbEvents::CONFIGURE);
    sleep(1);
}

void Controller::startClb() 
{
    clbEvent(ClbEvents::START);
    sleep(1);
}

void Controller::stopClb()
{
    clbEvent(ClbEvents::STOP);
    sleep(1);
}


void Controller::quitClb()
{
    clbEvent(ClbEvents::QUIT);
    sleep(1);
}

void Controller::resetClb()
{
    clbEvent(ClbEvents::RESET);
    sleep(1);
}

void Controller::pauseClb()
{
    clbEvent(ClbEvents::PAUSE);
    sleep(1);
}

void Controller::continueClb()
{
    clbEvent(ClbEvents::CONTINUE);
    sleep(1);
}


void Controller::setInitValues()
{
    std::vector<int>  var_ids;
    std::vector<long> var_values;

    // TODO: Check this is the same as the "3232238337" default is ControllerConfig
    unsigned char  addr4[4] = {192, 168, 11, 1};  // Server IP Address
    unsigned long ipi = ((0xFF & addr4[0]) << 24) |  ((0xFF & addr4[1]) << 16) |  ((0xFF & addr4[2]) <<  8) |  ((0xFF & addr4[3]) << 0);
    var_ids.push_back(ProcVar::NET_IPMUX_SRV_IP);       var_values.push_back(ipi);

    var_ids.push_back(ProcVar::SYS_TIME_SLICE_DUR);     var_values.push_back(config_.window_dur_);
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

    g_elastic.log(DEBUG, "Setting Initial Values..."); 
    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr);   

    // YOU CAN THEN DECODE THE VARIABLES FROM MSGREADER
}

void Controller::addNanobeacon(std::vector<int> &vid, std::vector<long> &vv)
{   // Should probably get the current status or have a value stored???
    vid.push_back(ProcVar::OPT_NANO_VOLT);      vv.push_back(60000);                       // TODO Nanobeacon Voltdage should be set somewhere 
    vid.push_back(ProcVar::SYS_SYS_RUN_ENA);    vv.push_back(( 0x70000a0 | 0x8000000 ));   // | 0x8000000 Add  the Nanobeancon enabling bit to SYS_SYS_RUN_ENA 
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

    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr);   

    // YOU CAN THEN DECODE THE VARIABLES FROM MSGREADER
}

void Controller::disableHV()
{
    MsgWriter mw;
    mw.writeU16(1);
    mw.writeI32(ProcVar::SYS_SYS_DISABLE);
    mw.writeU32((0 | ProcVar::SYS_SYS_DISABLE_PWR_MEAS ));
    
    g_elastic.log(DEBUG, "Disabling HV");  
    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr); 

    // YOU CAN THEN DECODE THE VARIABLES FROM MSGREADER 
}

void Controller::clbEvent(int event_id)
{
    // TODO check event id is correct. Event IDs defined in clb_events.h
    MsgWriter mw;
    int subsys = ClbSys::CLB_SUB_ALL;
    mw.writeI8(subsys);
    mw.writeI8(event_id);

    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_EVENT, mw, mr);  

    // YOU CAN THEN DECODE THE VARIABLES FROM MSGREADER
}

void Controller::setPMTs()
{
    // TODO: get the PMT e-IDs and check they match with confog file. 
    // Threshold Always the same. Just add Voltage and enabling code
    std::vector<int>  var_ids;
    std::vector<long> var_values;
    var_ids.push_back(ProcVar::OPT_CHAN_ENABLE);          
    var_ids.push_back(ProcVar::OPT_PMT_HIGHVOLT);

    //short int hv[31] = {0};
    //hv[27] = 120;

    long enable = 1 << 27;

    MsgWriter mw;
    mw.writeU16(var_ids.size());
    mw.writeI32(var_ids[0]);
    mw.writeU32(enable);
    mw.writeI32(var_ids[1]);
    for(int ipmt=0; ipmt<31; ++ipmt) mw.writeU8((short)config_.chan_hv_[ipmt]);

    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr);

    // YOU CAN THEN DECODE THE VARIABLES FROM MSGREADER       
}

void Controller::checkPMTs()
{
    // It should check PMT info  match config file or expectations. Just get the information for now 
    askPMTsInfo(ProcVar::OPT_CHAN_ENABLE);
    sleep(2); // Need to check if ready, sleep 5 sec for now   
    askPMTsInfo(ProcVar::OPT_PMT_ID);
    sleep(2); // Need to check if ready, sleep 5 sec for now   
    askPMTsInfo(ProcVar::OPT_PMT_HIGHVOLT);
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
        mw.writeU16(var_ids.size());
        mw.writeI32(var_ids[0]);
        //mw.writeI32Arr(var_ids);

        MsgReader mr;
        processor_.processCommand(MsgTypes::MSG_CLB_GET_VARS, mw, mr);  

        // YOU CAN THEN DECODE THE VARIABLES FROM MSGREADER 
        int count    = mr.readU16();               
        std::cout << " var counts " << count << std::endl;                                                                                                     
        int varId    = mr.readI32();   
        
        if(info_type == ProcVar::OPT_CHAN_ENABLE) {
        long enable =  mr.readU32();
        printf("Enabled channel %x\n", enable);
        //use it somehow
        } else {

	  VarInfo vt = VarInfo(varId);	  
	  std::vector<long int> varVal;            
	  for(int ipmt =0; ipmt<31; ++ipmt){            
	    if(vt.type_  ==  VarType::U8  ) varVal.push_back((long)mr.readU8());
            if(vt.type_  ==  VarType::U32 ) varVal.push_back(mr.readU32());
            printf("=====>>>>>  %d    Var Id %x  -  Value  %d %x\n", ipmt,  varId, varVal[ipmt], varVal[ipmt]);

        }// for ipmt 
        
        /// use pmt var values somehow 

        }// else

    } else {
        g_elastic.log(ERROR, "askPMTInfo: Wrong Variable ID!");  
    }    
}

void Controller::askVars(std::vector<int> var_ids)
{    // Ask Vars. TODO  add method (in msg_processor ?) to actually get the Information 
    // TODO:  check  var ids ar correct
    MsgWriter mw;
    mw.writeI32Arr(var_ids);

    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_GET_VARS, mw, mr);    

    // YOU CAN THEN DECODE THE VARIABLES FROM MSGREADER
}

