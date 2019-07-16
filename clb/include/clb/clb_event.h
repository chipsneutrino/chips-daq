/**
 * CLBEvent - Class dealing with  CLB event and possible transitions
 */

#pragma once

#include <util/elastic_interface.h>

/// Specifies the different CLB 
namespace CLBEvents 
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
enum CLBState
{
	UNDEFINED,
	IDLE,
	STAND_BY,
	READY,
	PAUSED,
	RUNNING
};

class CLBEvent {
public:
	/// CLBEvent Constructor
	CLBEvent() {};

	/// CLBEvent Constructor with initialisation
	CLBEvent(int event); 

	~CLBEvent() {};
	
	/// Get the source state for this event
	CLBState GetSourceState() 
	{ 
		return source_; 
	};

	/// Get the target state for this event
	CLBState GetTargetState()
	{ 
		return target_; 
	};

	int       	event_;			///< Event Type
	CLBState    source_;    	///< Source State
	CLBState    target_;    	///< Target State

private: 
	/// Set the states
	void SetStates();
};
