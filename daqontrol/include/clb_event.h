/**
 * ClbEvent - Class dealing with  CLB event and possible transitions
 */

#pragma once

#include "clb_events.h"
#include <util/elastic_interface.h>


/// Enumeration for the different CLB States
enum ClbState
  {
  UNDEFINED,
  IDLE,
  STAND_BY,
  READY,
  PAUSED,
  RUNNING

};

class ClbEvent {
public:
	/// ClbEvent Constructor
	ClbEvent() {};

	/// ClbEvent Constructor with initialisation
	ClbEvent(int event); 

	~ClbEvent() {};

	//Set Id
	void SetId(int event);
	
	// Get Id
	int GetEvent() { return event_;};

	// Get the Source State for this event
	ClbState GetSourceState() { return source_; };

	// Get the Target  State for this event
	ClbState GetTargetState() { return target_; };


 private: 
	
	void SetStates();

 public:

	int       	event_;					///< Event Type
	ClbState        source_;                                ///< Source State
	ClbState        target_;                                ///< Target State


};
