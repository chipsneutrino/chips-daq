/**
 * NovaSpillSignalType - NOvA spill signal types that correspond with different 
 * time points of the Fermilab accelerator cycle
 * 
 * NOTE: This file has been taken "as is" from NOvA code base and should always
 * match the version of software running the TDU
 *
 * Author: Petr Mánek
 * Contact: petr.manek.19@ucl.ac.uk
 */

#pragma once

#include <string>

enum NovaSpillSignalType : int {
    kNuMI, // MIBS $74 proton extraction into NuMI
    kBNB, // $1B paratisitic beam inhibit
    kNuMItclk, // tevatron clock, either $A9 or $AD depending on xml parameter
    kBNBtclk, // booster extraction, $1F (possibly sequence with $1D depending on configuration
    kAccelOneHztclk, // $8F 1 Hz clock
    kFake, // assigned if there is a parity error
    kTestConnection,
    kSuperCycle, // $00, Super cycle and master clock reset
    kNuMISampleTrig, // $A4,NuMI cycle sample trigger, reference for $A5
    kNuMIReset, // $A5, NuMI reset for beam
    kTBSpill, // $39, start of testbeam slow extraction
    kTBTrig, // testbeam trigger card signal
    kNSpillType // needs to be at the end, is used for range checking
};

static std::string getSpillSignalNameFromType(NovaSpillSignalType type);
