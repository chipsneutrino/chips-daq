/**
 * NssSpillInfo header file 
 * $Id: NssSpillInfo.h,v 1.11 2019/01/25 21:10:29 lackey32 Exp $
 */
#ifndef __NSS_SPILLINFO_H_
#define __NSS_SPILLINFO_H_

#include <list>
#include <stdint.h>
#include <string>

/**
 * Class NssSpillInfo
 *
 * This class holds all information for a spill, most notably the time
 * when it happened.
 * Times are stored in Nova time, 64 MHz ticks since nova epoch.
 * GPS time is different from UTC as explained in
 * http://tycho.usno.navy.mil/gpstt.html and elsewhere.
 * Note that GPS time, TAI (International Atomic time) and UTC all differ.
 * 
 * @author H. Meyer
 * @author <a href="mailto:holger.meyer@wichita.edu">holger.meyer@wichita.edu</a>
 */

namespace nova {
namespace NovaSpillServer {

enum SpillType{
  kNuMI,           //MIBS $74 proton extraction into NuMI
  kBNB,            //$1B paratisitic beam inhibit
  kNuMItclk,       //tevatron clock, either $A9 or $AD depending on xml parameter
  kBNBtclk,        //booster extraction, $1F (possibly sequence with $1D depending on configuration
  kAccelOneHztclk, //$8F 1 Hz clock
  kFake,           //assigned if there is a parity error
  kTestConnection,
  kSuperCycle,     //$00, Super cycle and master clock reset
  kNuMISampleTrig, //$A4,NuMI cycle sample trigger, reference for $A5
  kNuMIReset,      //$A5, NuMI reset for beam
  kTBSpill,        //$39, start of testbeam slow extraction
  kTBTrig,         //testbeam trigger card signal
  kNSpillType      // needs to be at the end, is used for range checking
};


class NssSpillInfo {

 public:

  NssSpillInfo () {;}
  NssSpillInfo (uint64_t const time, int const type);

  static bool getSpillTypeFromEvent(SpillType& type, uint16_t const evt);
  static int getSpillTypeFromName(std::string const& name);
  static std::string getSpillNameFromType(int type);
  static std::list< int > allSpillTypes();
  
  void set(uint64_t const time, int const type);

  uint64_t getTime() const {return _spilltime;}
  enum SpillType getType() const {return _spilltype;}


 private:

  enum SpillType _spilltype;
  uint64_t _spilltime; // see the NovaTimingUtilities package

  bool _checkSpillTypeRange(int type);

};

} // end of namespace NovaSpillServer
} // end of namespace nova

#endif // __NSS_SPILLINFO_H_
