/**
 * Provides access to CLB Subsystems.
 */

#pragma once

namespace ClbSys
{

  /** CLB System subsystem */
  static const  int CLB_SUB_SYS = 0;

  /** Networking subsystem */
  static const  int CLB_SUB_NET = 1;

  /** Optics subsystem */
  static const  int CLB_SUB_OPT = 2;

  /** Acoustics subsystem */
  static const  int CLB_SUB_ACS = 3;

  /** Instrumentation subsystem */
  static const  int CLB_SUB_INS = 4;

  /** DU base subsystem */
  static const  int CLB_SUB_BSE = 5;
  
  /** No of subsystems */
  static const  int CLB_SUB_CNT = 6;

  
  /** Wild-card subsystem (ALL) */
  static const  int CLB_SUB_ALL = 255;

};
