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

CLBController::~CLBController()
{
    //clbEvent(ClbEvents::RESET);
    //sleep(1);
}

void CLBController::init()
{
    // In initialisation we test the CLB connection, set basic values
    // and set the CLB state to INIT
    g_elastic.log(DEBUG, "CLBController Init..."); 
    testConnection();
    sleep(1);
    setInitValues();
    sleep(1);
    clbEvent(ClbEvents::RESET);
    sleep(1);
    clbEvent(ClbEvents::INIT);
    sleep(1);
    g_elastic.log(DEBUG, "CLBController Init DONE"); 
}

void CLBController::configure()
{
    // In configuration we set and check the PMT voltages and set the 
    // CLB state to CONFIGURE
    g_elastic.log(DEBUG, "CLBController Configure..."); 
    setPMTs();
    sleep(1);
    checkPMTs();   
    sleep(1);
    clbEvent(ClbEvents::CONFIGURE);
    sleep(1);
    g_elastic.log(DEBUG, "CLBController Configure DONE"); 
}

void CLBController::startData() 
{
    // When we start the data flow we set the CLB state to START
    g_elastic.log(DEBUG, "CLBController Start Data...");
    clbEvent(ClbEvents::START);
    sleep(1);
    g_elastic.log(DEBUG, "CLBController Start Data DONE");
}

void CLBController::stopData()
{
    // When we start the data flow we set the CLB state to STOP
    g_elastic.log(DEBUG, "CLBController Stop Data..."); 
    clbEvent(ClbEvents::STOP);
    sleep(1);
    g_elastic.log(DEBUG, "CLBController Stop Data DONE");
}

void CLBController::flasherOn(float flasher_v)
{
    g_elastic.log(DEBUG, "CLBController Enabling Nanobeacon...");
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
 
    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr); 
    sleep(1);
    g_elastic.log(DEBUG, "CLBController Enabling Nanobeacon DONE");  
}

void CLBController::flasherOff()
{
    g_elastic.log(DEBUG, "CLBController Disabling Nanobeacon..."); 
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

     
    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr);  
    sleep(1);
    g_elastic.log(DEBUG, "CLBController Disabling Nanobeacon DONE"); 
}

void CLBController::testConnection()
{
    // To test the connection we ask for the date of the software revisions
    MsgWriter mw;
    MsgReader mr;
    if(!processor_.processCommand(MsgTypes::MSG_SYS_DATEREV, mw, mr))
    {
        g_elastic.log(ERROR, "Could not get response from CLB in test!"); 
        return;
    }
    //long hwDateRev = mr.readU32();
    //long swDateRev = mr.readU32();
    //g_elastic.log(INFO, "Test successful, hardware({0:8x}), software({0:8x})", hwDateRev, swDateRev); 
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

    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr);   
}

void CLBController::disableHV()
{
    MsgWriter mw;
    mw.writeU16(1);
    mw.writeI32(ProcVar::SYS_SYS_DISABLE);
    unsigned long disable = (0 | ProcVar::SYS_SYS_DISABLE_HV);
    mw.writeU32(disable);
    
    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr); 
}

void CLBController::clbEvent(int event_id)
{
    // First lets set the CLB state
    MsgWriter mw;
    int subsys = ClbSys::CLB_SUB_ALL;
    mw.writeI8(subsys);
    mw.writeI8(event_id);

    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_EVENT, mw, mr);  

    // Now we check the CLB state
    /*
    MsgWriter mw2;
    MsgReader mr2;
    processor_.processCommand(MsgTypes::MSG_CLB_EXT_UPDATE, mw2, mr2);  
    int size = mr2.readU8();
    int subsys_ret  = mr2.readU8();
    int state   = mr2.readU8();
    int status  = mr2.readU8();
    int errCode = mr.readI32();
    std::string errMsg;
    if (errCode > 0) {
        errMsg = mr.readString();
    } else {
        errMsg = "";
    }

    std::cout << size << "-" << subsys_ret << "-" << state << "-" << status << std::endl;
    */
}

void CLBController::setPMTs()
{
    std::vector<int>  var_ids;
    std::vector<long> var_values;
    var_ids.push_back(ProcVar::OPT_CHAN_ENABLE);          
    var_ids.push_back(ProcVar::OPT_PMT_HIGHVOLT);

    unsigned long enabled = config_.chan_enabled_.to_ulong();

    MsgWriter mw;
    mw.writeU16(var_ids.size());
    mw.writeI32(var_ids[0]);
    mw.writeU32(enabled);
    mw.writeI32(var_ids[1]);
    for(int ipmt=0; ipmt<31; ++ipmt) mw.writeU8((short)config_.chan_hv_[ipmt]);

    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_SET_VARS, mw, mr);     
}

void CLBController::checkPMTs()
{
    // First lets get all the info we want back from the CLB
    std::vector<int> var_ids;
    var_ids.push_back(ProcVar::OPT_CHAN_ENABLE);
    var_ids.push_back(ProcVar::OPT_PMT_ID);
    var_ids.push_back(ProcVar::OPT_PMT_HIGHVOLT);
    MsgWriter mw;
    mw.writeU16(var_ids.size());
    mw.writeI32(var_ids[0]);
    mw.writeI32(var_ids[1]);
    mw.writeI32(var_ids[2]);

    MsgReader mr;
    processor_.processCommand(MsgTypes::MSG_CLB_GET_VARS, mw, mr);  

    int count = mr.readU16();               
    if (count != var_ids.size())
    {
        g_elastic.log(ERROR, "Got wrong number of return variables {}", count); 
        return;           
    }

    // Check the enabled channels
    int varId    = mr.readI32();   
    std::bitset<32> enabled(mr.readU32());
    if (enabled != config_.chan_enabled_)
    {
        g_elastic.log(ERROR, "Enabled channels do not match!");       
    }

    // Check the channel eids
    varId = mr.readI32();            
	for(int ipmt =0; ipmt<31; ++ipmt){  
        long eid = mr.readU32();
        if (eid != config_.chan_eid_[ipmt] && enabled[ipmt])
        {
            g_elastic.log(ERROR, "Non matching eid for PMT {}, {} vs {}!", ipmt, eid, config_.chan_eid_[ipmt]);            
        }
    }

    // Check the channel high voltage
    varId = mr.readI32();         
	for(int ipmt =0; ipmt<31; ++ipmt){
        long voltage = (long)mr.readU8();
        if (voltage != config_.chan_hv_[ipmt] && enabled[ipmt])
        {
            g_elastic.log(ERROR, "Non matching voltage for PMT {}, {} vs {}!", ipmt, voltage, config_.chan_hv_[ipmt]);           
        }
    }
}

void CLBController::askVars(std::vector<int> var_ids)
{
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

