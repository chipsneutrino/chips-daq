/**
 * Provides access to all variables of the various subsystems.
 */

#pragma once

namespace ProcVar 
{
    // ------------------------------------------------------------
    // Definitions for subsystem System
    // ------------------------------------------------------------

    /** 
     * The current run number
     * 20160704 Made run number configurable
     */
    static const int SYS_RUN_NUMBER           = 0x00127000;

    /** 
     * DOM identifier
     */
    static const int SYS_DOM_ID               = 0x00224000;

    /** 
     * Timeslice duration in microseconds
     */
    static const int SYS_TIME_SLICE_DUR       = 0x00327000;

    /** 
     * Hardware revision (YYMMDDBB hex)
     */
    static const int SYS_HW_DATE_REV          = 0x00624000;

    /** 
     * Software revision (YYMMDDBB hex)
     */
    static const int SYS_SW_DATE_REV          = 0x00724000;

    /** 
     * Max packet size in bytes as chopped by the HW-StateMachine
     */
    static const int SYS_STMACH_PKTSIZE       = 0x00417000;

    /** 
     * Power measurement readings
     */
    static const int SYS_PWR_MEAS             = 0x00514011;
    /** 12 Volt current, in milliamps */
    static const int SYS_PWR_MEAS_12V_CUR     = 0;
    /** 1 Volt current, in milliamps */
    static const int SYS_PWR_MEAS_1V_CUR      = 1;
    /** 1.8 Volt current, in milliamps */
    static const int SYS_PWR_MEAS_1V8_CUR     = 2;
    /** 2.5 Volt current, in milliamps */
    static const int SYS_PWR_MEAS_2V5_CUR     = 3;
    /** 3.3 Volt current, in milliamps */
    static const int SYS_PWR_MEAS_3V3_CUR     = 4;
    /** 5 Volt current, in milliamps */
    static const int SYS_PWR_MEAS_5V_CUR      = 5;
    /** 3.3 Volt PMT current, in milliamps */
    static const int SYS_PWR_MEAS_3V3PMT_CUR  = 6;
    /** LED current, in milliamps */
    static const int SYS_PWR_MEAS_VLED_CUR    = 7;
    /** LED Voltage level, in millivolts */
    static const int SYS_PWR_MEAS_VLED_LVL    = 8;
    /** 12 Volt level, in millivolts */
    static const int SYS_PWR_MEAS_12V_LVL     = 9;
    /** Temperature sensor voltage, in millivolts */
    static const int SYS_PWR_MEAS_TEMP_LVL    = 10;
    /** 1 Volt level, in millivolts */
    static const int SYS_PWR_MEAS_1V_LVL      = 11;
    /** 1.8 Volt level, in millivolts */
    static const int SYS_PWR_MEAS_1V8_LVL     = 12;
    /** 2.5 Volt level, in millivolts */
    static const int SYS_PWR_MEAS_2V5_LVL     = 13;
    /** 3.3 Volt level, in millivolts */
    static const int SYS_PWR_MEAS_3V3_LVL     = 14;
    /** 5 Volt level, in millivolts */
    static const int SYS_PWR_MEAS_5V_LVL      = 15;
    /** 3.3 Volt level, in millivolts */
    static const int SYS_PWR_MEAS_3V3PMT_LVL  = 16;
    /** DAC Control voltage, in millivolts */
    static const int SYS_PWR_MEAS_DACCTL_LVL  = 17;

    /** 
     * System temperatures and humidity summery
     * All values in 1/100th of a degree or 1/100th of a percent
     */
    static const int SYS_SYS_TEMP_HUM         = 0x00854005;
    /** FPGA core temperature */
    static const int SYS_SYS_TEMP_HUM_FPGA    = 0;
    /** Temperature (SHT21) */
    static const int SYS_SYS_TEMP_HUM_CLB     = 1;
    /** Humidity (SHT21) */
    static const int SYS_SYS_TEMP_HUM_CLB_HUM = 2;
    /** Power board temperature (MAX123) */
    static const int SYS_SYS_TEMP_HUM_PWR     = 3;
    /** White Rabbit (DS18S20) */
    static const int SYS_SYS_TEMP_HUM_WR      = 4;
    /** SFP Transceiver temperature */
    static const int SYS_SYS_TEMP_HUM_SFP     = 5;

    /** 
     * Disable parts of the system
     */
    static const int SYS_SYS_DISABLE          = 0x00906000;
    /** Bit which disables AHRS (Compass) periodic readout */
    static const int SYS_SYS_DISABLE_AHRS     = 0x1;
    /** Bit which disables Temperature / Humidity */
    static const int SYS_SYS_DISABLE_TEMP_HUM = 0x2;
    /** Bit which disables High Voltage */
    static const int SYS_SYS_DISABLE_HV       = 0x4;
    /** Bit which disables the Nano-beacon step-up regulator */
    static const int SYS_SYS_DISABLE_NANO_STEP_UP = 0x8;
    /** Bit which disables the power (voltage/current) measurement */
    static const int SYS_SYS_DISABLE_PWR_MEAS = 0x10;

    /** 
     * Run-state Enable mask
     */
    static const int SYS_SYS_RUN_ENA          = 0x00a07000;
    /** Bit which enables TDC during run */
    static const int SYS_SYS_RUN_ENA_TDC      = 0x1;
    /** Bit which enables Acoustics during run */
    static const int SYS_SYS_RUN_ENA_ACS      = 0x2;
    /** Bit which enables Monitoring during run */
    static const int SYS_SYS_RUN_ENA_MON      = 0x4;
    /** Bit which enables Nano-Beacon during run */
    static const int SYS_SYS_RUN_ENA_NANO     = 0x8;

    /** 
     * Currently running image type
     */
    static const int SYS_IMGTYPE              = 0x00b04000;
    /** Golden image running */
    static const int SYS_IMGTYPE_GOLDEN       = 0x1;
    /** Detector running */
    static const int SYS_IMGTYPE_DETECTOR     = 0x2;
    /** Base image running */
    static const int SYS_IMGTYPE_BASE         = 0x3;
    /** Calibration image running */
    static const int SYS_IMGTYPE_CALIBRATION  = 0x4;

    /** 
     * Failure bitmask, same bits as SYS_DISABLE
     */
    static const int SYS_SYS_FAIL             = 0x00c06000;
    /** Bit which disables AHRS (Compass) periodic readout */
    static const int SYS_SYS_FAIL_AHRS        = 0x1;
    /** Bit which disables Temperature / Humidity */
    static const int SYS_SYS_FAIL_TEMP_HUM    = 0x2;
    /** Bit which disables the Nano-beacon step-up regulator */
    static const int SYS_SYS_FAIL_NANO_STEP_UP = 0x8;
    /** Bit which disables the power (voltage/current) measurement */
    static const int SYS_SYS_FAIL_PWR_MEAS    = 0x10;

    // ------------------------------------------------------------
    // Definitions for subsystem Networking & IPMux
    // ------------------------------------------------------------

    /** 
     * White Rabbit Round-Trip time
     */
    static const int NET_WR_MU                = 0x10834000;

    /** 
     * Cable roundtrip time in ps
     */
    static const int NET_CABLE_RTT            = 0x10c74000;

    /** 
     * Server IP address (0xAABBCCDD = AA.BB.CC.DD)
     */
    static const int NET_IPMUX_SRV_IP         = 0x10327000;

    /** 
     * White Rabbit Delta values, where each entry is a delta-value.
     */
    static const int NET_WR_DELTA             = 0x10664003;
    /** Slave transmit delta delay index */
    static const int NET_WR_DELTA_SLAVE_TX    = 0;
    /** Slave receive delta delay index */
    static const int NET_WR_DELTA_SLAVE_RX    = 1;
    /** Master transmit delta delay index */
    static const int NET_WR_DELTA_MASTER_TX   = 2;
    /** Master receive delta delay index */
    static const int NET_WR_DELTA_MASTER_RX   = 3;

    /** 
     * Modules MAC address
     * Encoding, [0xAABB,0xCCDD,0xEEFF] => AA:BB:CC:DD:EE:FF
     */
    static const int NET_MAC_ADDR             = 0x10114002;

    /** 
     * IP Mux ports, 0 - TDC, 1 - Accoustics, 2 - Instruments
     */
    static const int NET_IPMUX_PORTS          = 0x10217003;

    /** 
     * Server MAC address, set to 00 to resolve.
     * Encoding, [0xAABB,0xCCDD,0xEEFF] => AA:BB:CC:DD:EE:FF
     */
    static const int NET_IPMUX_SRV_MAC        = 0x10417002;

    /** 
     * Slow control port
     */
    static const int NET_SC_PORT              = 0x10517000;
    /** Default slow control port */
    static const int NET_SC_PORT_DEFAULT      = 0xDACE;

    /** 
     * White Rabbit Bitslide value
     */
    static const int NET_WR_BITSLIDE          = 0x10714000;

    /** 
     * Receiver input power in 10^-1 uW
     */
    static const int NET_RX_INPUT_POWER       = 0x10d14000;

    /** 
     * White Rabbit PTP State
     */
    static const int NET_WR_ST_PTP            = 0x10944000;
    /** INITIALIZING */
    static const int NET_WR_ST_PTP_INITIALIZING = 0;
    /** FAULTY */
    static const int NET_WR_ST_PTP_FAULTY     = 1;
    /** DISABLED */
    static const int NET_WR_ST_PTP_DISABLED   = 2;
    /** LISTENING */
    static const int NET_WR_ST_PTP_LISTENING  = 3;
    /** PRE_MASTER */
    static const int NET_WR_ST_PTP_PRE_MASTER = 4;
    /** MASTER */
    static const int NET_WR_ST_PTP_MASTER     = 5;
    /** PASSIVE */
    static const int NET_WR_ST_PTP_PASSIVE    = 6;
    /** UNCALIBRATED */
    static const int NET_WR_ST_PTP_UNCALIBRATED = 7;
    /** SLAVE */
    static const int NET_WR_ST_PTP_SLAVE      = 8;
    /** UNINITIALIZED */
    static const int NET_WR_ST_PTP_UNINITIALIZED = -1;

    /** 
     * White Rabbit general state
     */
    static const int NET_WR_ST_GEN            = 0x10a44000;
    /** PRESENT */
    static const int NET_WR_ST_GEN_PRESENT    = 0;
    /** S_LOCK */
    static const int NET_WR_ST_GEN_S_LOCK     = 1;
    /** M_LOCK */
    static const int NET_WR_ST_GEN_M_LOCK     = 2;
    /** LOCKED */
    static const int NET_WR_ST_GEN_LOCKED     = 3;
    /** CALIBRATION */
    static const int NET_WR_ST_GEN_CALIBRATION = 4;
    /** CALIBRATED */
    static const int NET_WR_ST_GEN_CALIBRATED = 5;
    /** RESP_CALIB_REQ */
    static const int NET_WR_ST_GEN_RESP_CALIB_REQ = 6;
    /** WR_LINK_ON */
    static const int NET_WR_ST_GEN_WR_LINK_ON = 7;
    /** TIMER_ARRAY_SIZE */
    static const int NET_WR_ST_GEN_TIMER_ARRAY_SIZE = 8;
    /** IDLE */
    static const int NET_WR_ST_GEN_IDLE       = 9;
    /** S_LOCK_1 */
    static const int NET_WR_ST_GEN_S_LOCK_1   = 10;
    /** S_LOCK_2 */
    static const int NET_WR_ST_GEN_S_LOCK_2   = 11;
    /** CALIBRATION_1 */
    static const int NET_WR_ST_GEN_CALIBRATION_1 = 12;
    /** CALIBRATION_2 */
    static const int NET_WR_ST_GEN_CALIBRATION_2 = 13;
    /** CALIBRATION_3 */
    static const int NET_WR_ST_GEN_CALIBRATION_3 = 14;
    /** CALIBRATION_4 */
    static const int NET_WR_ST_GEN_CALIBRATION_4 = 15;
    /** CALIBRATION_5 */
    static const int NET_WR_ST_GEN_CALIBRATION_5 = 16;
    /** CALIBRATION_6 */
    static const int NET_WR_ST_GEN_CALIBRATION_6 = 17;
    /** CALIBRATION_7 */
    static const int NET_WR_ST_GEN_CALIBRATION_7 = 18;
    /** CALIBRATION_8 */
    static const int NET_WR_ST_GEN_CALIBRATION_8 = 19;
    /** RESP_CALIB_REQ_1 */
    static const int NET_WR_ST_GEN_RESP_CALIB_REQ_1 = 20;
    /** RESP_CALIB_REQ_2 */
    static const int NET_WR_ST_GEN_RESP_CALIB_REQ_2 = 21;
    /** RESP_CALIB_REQ_3 */
    static const int NET_WR_ST_GEN_RESP_CALIB_REQ_3 = 22;
    /** UNINITIALIZED */
    static const int NET_WR_ST_GEN_UNINITIALIZED = -1;

    /** 
     * White Rabbit servo state
     */
    static const int NET_WR_ST_SERV           = 0x10b44000;
    /** SYNC_NSEC */
    static const int NET_WR_ST_SERV_SYNC_NSEC = 1;
    /** SYNC_TAI */
    static const int NET_WR_ST_SERV_SYNC_TAI  = 2;
    /** SYNC_PHASE */
    static const int NET_WR_ST_SERV_SYNC_PHASE = 3;
    /** TRACK_PHASE */
    static const int NET_WR_ST_SERV_TRACK_PHASE = 4;
    /** WAIT_SYNC_IDLE */
    static const int NET_WR_ST_SERV_WAIT_SYNC_IDLE = 5;
    /** WAIT_OFFSET_STABLE */
    static const int NET_WR_ST_SERV_WAIT_OFFSET_STABLE = 6;
    /** UNINITIALIZED */
    static const int NET_WR_ST_SERV_UNINITIALIZED = -1;

    // ------------------------------------------------------------
    // Definitions for subsystem Optics
    // ------------------------------------------------------------

    /** 
     * All PMT ID's
     */
    static const int OPT_PMT_ID               = 0x2052401e;

    /** 
     * High-rate veto (hits per timeslice)
     */
    static const int OPT_HR_VETO_THRES        = 0x20627000;

    /** 
     * Channel enable, bit per channel
     */
    static const int OPT_CHAN_ENABLE          = 0x20827000;

    /** 
     * High-rate veto enable, bit per channel
     */
    static const int OPT_HR_VETO_ENA_CH       = 0x20e27000;

    /** 
     * Multi-hit enable, bit per channel
     */
    static const int OPT_MULHIT_ENA_CH        = 0x20f27000;

    /** 
     * Length of pulse in 16 ns ticks, default is 4 ticks
     */
    static const int OPT_NANO_LENGTH          = 0x20917000;

    /** 
     * Period of beacon in 16 ns ticks, default is 0xC35 ticks, or 50us
     */
    static const int OPT_NANO_PERIOD          = 0x20a17000;

    /** 
     * Nano-beacon voltage in millivolts, 0-30000V
     */
    static const int OPT_NANO_VOLT            = 0x20b17000;

    /** 
     * Nano-beacon delay in 16 ns ticks
     */
    static const int OPT_NANO_DELAY           = 0x20c17000;

    /** 
     * Nano-beacon pulse train count, each second
     */
    static const int OPT_NANO_PCOUNT          = 0x20d17000;

    /** 
     * Channel configuration (depricated, use CHAN_ENABLE)
     */
    static const int OPT_CHAN_CONFIG          = 0x2010701e;

    /** 
     * Channel status
     */
    static const int OPT_CHAN_STATUS          = 0x2020401e;

    /** 
     * High voltage settings per channel
     * Scaling: 0: -700V, 255: -1500V
     */
    static const int OPT_PMT_HIGHVOLT         = 0x2030701e;

    /** 
     * Threshold settings per channel
     * Scaling: 0: 800mV, 255: 2400mV
     */
    static const int OPT_PMT_THRESHOLD        = 0x2040701e;

    /** 
     * Enable nano-beacon (deprecated, see SYS_RUN_ENA)
     */
    static const int OPT_NANO_ENABLE          = 0x20787000;
    
    
    ///////////////////////////////////////////////////
    /*
    GOING TO NEED TO ADD COME OPT_CLB AND OPT_RELAY HERE
    */
    ///////////////////////////////////////////////////
    /*
    static const int CLB_EID	= 	0xaaaaaaa	;

    static const int CLB_IP	= 	0xaaaaaab	;
    
    static const int CLB_MAC	= 	0xaaaaaac	;
    
    static const int CLB_HID_TYPE = 	0xaaaaaad	;
    
    static const int CLB_HID_X = 	0xaaaaaae	;
    
    static const int CLB_HID_Y = 	0xaaaaaaf	;
    
    static const int CLB_CHANNELS	=	0xaaaaaba	;
    
    static const int CLB_ENABLE	=		0xaaaaabb	;
    
    static const int RELAY_CHANNEL	= 	0xbaaaaaa	;
    
    static const int RELAY_IP	= 	0xbaaaaab	;
    
    */
    // ------------------------------------------------------------
    // Definitions for subsystem Instrumentation
    // ------------------------------------------------------------

    /** 
     * Pitch in degrees
     */
    static const int INS_AHRS_PITCH           = 0x301a4000;

    /** 
     * Roll in degrees
     */
    static const int INS_AHRS_ROLL            = 0x302a4000;

    /** 
     * Yaw in degrees
     */
    static const int INS_AHRS_YAW             = 0x303a4000;

    /** 
     * Acceleration projection, (Index 0=X, 1=Y, 2=Z)
     */
    static const int INS_AHRS_A               = 0x304a4002;

    /** 
     * Angular velocity projection, (Index 0=X, 1=Y, 2=Z)
     */
    static const int INS_AHRS_G               = 0x305a4002;

    /** 
     * Magnetic field projection, (Index 0=X, 1=Y, 2=Z)
     */
    static const int INS_AHRS_H               = 0x306a4002;

    /** 
     * Temperature in 1/100th of a degree
     */
    static const int INS_TEMP                 = 0x30754000;

    /** 
     * Humidity in 1/100th RH
     */
    static const int INS_HUMID                = 0x30814000;

    /** 
     * Compass enable (deprecated, see SYS_DISABLE)
     */
    static const int INS_AHRS_ENABLE          = 0x30987000;

    /** 
     * Compass data are valid
     */
    static const int INS_AHRS_VALID           = 0x30a84000;

    /** 
     * Compass version:
     * 0 if not available or very old version.
     * 255 for LSM303.
     * Otherwise version * 10, e.g. version 4.1 would be value 41
     */
    static const int INS_COMPASS_VERSION      = 0x30b04000;

    // ------------------------------------------------------------
    // Definitions for subsystem Acoustics
    // ------------------------------------------------------------

    /** 
     * Acoustic enable (deprecated, see SYS_RUN_ENA)
     */
    static const int ACS_ACOU_ENABLE          = 0x40187000;

    /** 
     * Acoustics channel config
     */
    static const int ACS_ACOU_CHAN            = 0x40207000;
    /** Enable both acoustic channels */
    static const int ACS_ACOU_CHAN_BOTH       = 0;
    /** Enable only channel 1 (default) */
    static const int ACS_ACOU_CHAN_ONE        = 1;
    /** Enable only channel 2 */
    static const int ACS_ACOU_CHAN_TWO        = 2;

    /** 
     * Acoustic resolution
     */
    static const int ACS_ACOU_RES             = 0x40307000;
    /** 12 bit resolution */
    static const int ACS_ACOU_RES_12_BITS     = 0;
    /** 16 bit resolution */
    static const int ACS_ACOU_RES_16_BITS     = 1;
    /** 24 bit resolution (default) */
    static const int ACS_ACOU_RES_24_BITS     = 2;

    // ------------------------------------------------------------
    // Definitions for subsystem Base
    // ------------------------------------------------------------

    /** 
     * Monitored Power Setpoint for APC mode (in milli dBm)
     */
    static const int BSE_EDFA_SETP_RD         = 0x50264000;

    /** 
     * Input Optical Power (in milli dBm)
     */
    static const int BSE_EDFA_IOP             = 0x50364000;

    /** 
     * Output Optical Power (in milli dBm)
     */
    static const int BSE_EDFA_OOP             = 0x50464000;

    /** 
     * Diode Pump Current (in micro A)
     */
    static const int BSE_EDFA_PUMPDC          = 0x50564000;

    /** 
     * Temperature (in milli degrees celsius)
     */
    static const int BSE_EDFA_TEMP            = 0x50664000;

    /** 
     * Settings for Power Setpoint (in dBm)
     */
    static const int BSE_EDFA_SETP_WR         = 0x508a2000;

    /** 
     * Backbone current (0 to 1023 ADC raw)
     */
    static const int BSE_BPS_IBACK            = 0x50914000;

    /** 
     * Current of 12V line (0 to 1023 ADC raw)
     */
    static const int BSE_BPS_I12              = 0x50a14000;

    /** 
     * Voltage of 375V line (0 to 1023 ADC raw)
     */
    static const int BSE_BPS_V375             = 0x50b14000;

    /** 
     * Current of 375V line (0 to 1023 ADC raw)
     */
    static const int BSE_BPS_I375             = 0x50c14000;

    /** 
     * Voltage of 5V line (0 to 1023 ADC raw)
     */
    static const int BSE_BPS_V5               = 0x50d14000;

    /** 
     * Breaker status
     */
    static const int BSE_BPS_BREAKER          = 0x50e14000;

    /** 
     * Alarm Status
     */
    static const int BSE_BPS_ALARM            = 0x50f14000;

    /** 
     * ACDC Voltage, between 300 and 556
     */
    static const int BSE_ACDC_VOLT            = 0x51014000;

    /** 
     * ACDC Current, between 0 and 2560A
     */
    static const int BSE_ACDC_CUR             = 0x51114000;

    /** 
     * Emission
     */
    static const int BSE_EDFA_EMI             = 0x50186000;

    /** 
     * Serial number (Ascii string)
     */
    static const int BSE_EDFA_SN              = 0x5070401f;

    /** 
     * ACDC Temperature in degrees Centigrade
     */
    static const int BSE_ACDC_TEMP            = 0x51244000;

    /** 
     * Bitmask containing error devices. Once errored they will not
     * be read again until cleared
     */
    static const int BSE_INST_FAIL            = 0x51306000;
    /** EDFA */
    static const int BSE_INST_FAIL_EDFA       = 0x1;
    /** BPS */
    static const int BSE_INST_FAIL_BPS        = 0x2;
    /** ACDC */
    static const int BSE_INST_FAIL_ACDC       = 0x4;    
};
