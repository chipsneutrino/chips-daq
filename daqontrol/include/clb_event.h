/**
 * ClbEvent - Class dealing with  CLB event and possible transitions
 */

#pragma once

#include <util/elastic_interface.h>

/// Specifies the different CLB 
namespace ClbEvents 
{
	static const int INIT        = 1;  
	static const int CONFIGURE   = 2;  
	static const int START       = 3;  
	static const int PAUSE       = 4;  
	static const int CONTINUE    = 5;  
	static const int STOP        = 6;  
	static const int QUIT        = 7;  
	static const int RESET       = 8;  
};

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

	/// Set Id
	void SetId(int event);
	
	/// Get Id
	int GetEvent() 
	{ 
		return event_;
	};

	/// Get the Source State for this event
	ClbState GetSourceState() 
	{ 
		return source_; 
	};

	/// Get the Target  State for this event
	ClbState GetTargetState()
	{ 
		return target_; 
	};

	int       	event_;			///< Event Type
	ClbState    source_;    	///< Source State
	ClbState    target_;    	///< Target State

private: 
	/// Set the states
	void SetStates();
};
