/**
 * CLBController - CLBController for an individual CLB
 */

#include "clb_controller.h"

CLBController::CLBController(ControllerConfig config)
    : Controller(config)
    , processor_(config.ip_, io_service_)
{
    g_elastic.log(INFO, "Creating CLBController for CLB:{}", config.eid_); 
}

void CLBController::init()
{
    // In initialisation we test the CLB connection, set basic values
    // and set the CLB state to INIT
    g_elastic.log(DEBUG, "CLBController Init"); 
    test();
    sleep(1);
    setInitValues();
    sleep(1);
    clbEvent(ClbEvents::INIT);
    sleep(1);
}

void CLBController::configure()
{
    // In configuration we set and check the PMT voltages and set the 
    // CLB state to CONFIGURE
    g_elastic.log(DEBUG, "CLBController Configure"); 
    setPMTs();
    sleep(1);
    checkPMTs();   
    sleep(1);
    clbEvent(ClbEvents::CONFIGURE);
    sleep(1);
}

void CLBController::startData() 
{
    // When we start the data flow we set the CLB state to START
    g_elastic.log(DEBUG, "CLBController Start Data"); 
    clbEvent(ClbEvents::START);
    sleep(1);
}

void CLBController::stopData()
{
    // When we start the data flow we set the CLB state to STOP
    g_elastic.log(DEBUG, "CLBController Stop Data"); 
    clbEvent(ClbEvents::STOP);
    sleep(1);
}

void CLBController::flasherOn(float flasher_v)
{
    std::vector<int>  var_ids;
    std::vector<long> var_values;
    var_ids.push_back(ProcVar::OPT_NANO_VOLT);      var_values.push_back(60000);                       // TODO Nanobeacon Voltdage should be set somewhere 
    var_ids.push_back(ProcVar::SYS_SYS_RUN_ENA);    var_values.push_back(( 0x70000a0 | 0x8000000 ));   // | 0x8000000 Add  the Nanobeancon enabling bit to SYS_SYS_RUN_ENA 

    MsgWriter mw;
    mw.writeU16(var_ids.size());
    for(int ii=0; ii<var_ids.size(); ++ii)
    {
        mw.writeI32(var_ids[ii]);
        mw.writeU32(var_values[ii]);
    }

    g_elastic.log(DEBUG, "CLBController Enabling Nanobeacon");  
    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr);  
}

void CLBController::flasherOff()
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

    g_elastic.log(DEBUG, "CLBController Disabling Nanobeacon");  
    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr);  
}

void CLBController::test()
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
}

void CLBController::setInitValues()
{
    std::vector<int>  var_ids;
    std::vector<long> var_values;

    unsigned char  addr4[4] = {192, 168, 11, 1};  // Server IP Address
    unsigned long ipi = ((0xFF & addr4[0]) << 24) |  ((0xFF & addr4[1]) << 16) |  ((0xFF & addr4[2]) <<  8) |  ((0xFF & addr4[3]) << 0);
    var_ids.push_back(ProcVar::NET_IPMUX_SRV_IP);       var_values.push_back(ipi);

    var_ids.push_back(ProcVar::SYS_TIME_SLICE_DUR);     var_values.push_back(config_.window_dur_);
    var_ids.push_back(ProcVar::OPT_HR_VETO_ENA_CH);     var_values.push_back(0x00000000); // Disable all Channels HR Veto  
    var_ids.push_back(ProcVar::OPT_MULHIT_ENA_CH);      var_values.push_back(0x00000000); // Disable all Channels Multi Hits

    MsgWriter mw;
    mw.writeU16(var_ids.size());
    for(int ii=0; ii<var_ids.size(); ++ii){
        mw.writeI32(var_ids[ii]);
        mw.writeU32(var_values[ii]);
    }

    g_elastic.log(DEBUG, "Setting Initial Values..."); 
    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr);   
}

void CLBController::disableHV()
{
    MsgWriter mw;
    mw.writeU16(1);
    mw.writeI32(ProcVar::SYS_SYS_DISABLE);
    mw.writeU32((0 | ProcVar::SYS_SYS_DISABLE_PWR_MEAS ));
    
    g_elastic.log(DEBUG, "Disabling HV");  
    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr); 
}

void CLBController::clbEvent(int event_id)
{
    // TODO check event id is correct. Event IDs defined in clb_events.h
    MsgWriter mw;
    int subsys = ClbSys::CLB_SUB_ALL;
    mw.writeI8(subsys);
    mw.writeI8(event_id);

    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_EVENT, mw, mr);  
}

void CLBController::setPMTs()
{
    // TODO: get the PMT e-IDs and check they match with confog file. 
    // Threshold Always the same. Just add Voltage and enabling code
    std::vector<int>  var_ids;
    std::vector<long> var_values;
    var_ids.push_back(ProcVar::OPT_CHAN_ENABLE);          
    var_ids.push_back(ProcVar::OPT_PMT_HIGHVOLT);

    long enable = 1 << 27;

    MsgWriter mw;
    mw.writeU16(var_ids.size());
    mw.writeI32(var_ids[0]);
    mw.writeU32(enable);
    mw.writeI32(var_ids[1]);
    for(int ipmt=0; ipmt<31; ++ipmt) mw.writeU8((short)config_.chan_hv_[ipmt]);

    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr);     
}

void CLBController::checkPMTs()
{
    // It should check PMT info  match config file or expectations. Just get the information for now 
    askPMTsInfo(ProcVar::OPT_CHAN_ENABLE);
    sleep(2); // Need to check if ready, sleep 5 sec for now   
    askPMTsInfo(ProcVar::OPT_PMT_ID);
    sleep(2); // Need to check if ready, sleep 5 sec for now   
    askPMTsInfo(ProcVar::OPT_PMT_HIGHVOLT);
}  

void CLBController::askState()
{
    // Should ask the CLB State (Running, Paused etc.) Not sure how to do that yet 
}

void CLBController::askPMTsInfo(int info_type)
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

void CLBController::askVars(std::vector<int> var_ids)
{    // Ask Vars. TODO  add method (in msg_processor ?) to actually get the Information 
    // TODO:  check  var ids ar correct
    MsgWriter mw;
    mw.writeI32Arr(var_ids);

    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_GET_VARS, mw, mr);    
}

void CLBController::quit()
{
    clbEvent(ClbEvents::QUIT);
    sleep(1);
}

void CLBController::reset()
{
    clbEvent(ClbEvents::RESET);
    sleep(1);
}

void CLBController::pause()
{
    clbEvent(ClbEvents::PAUSE);
    sleep(1);
}

void CLBController::continueRun()
{
    clbEvent(ClbEvents::CONTINUE);
    sleep(1);
}

