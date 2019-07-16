/**
 *  CLBEvent - Class dealing with CLB event and possible transitions  
 */

#include <clb/clb_event.h>
	
CLBEvent::CLBEvent(int event)
{
	event_ = event;
	SetStates();
}

void CLBEvent::SetStates()
{  
	switch(event_) {
		case CLBEvents::INIT:    
			source_ = CLBState::IDLE;   
			target_ = CLBState::STAND_BY;    
			break;
	

		case CLBEvents::CONFIGURE:
			source_ = CLBState::STAND_BY;
			target_ = CLBState::READY;
			break;


		case CLBEvents::START:    
			source_ =  CLBState::READY;   
			target_ =  CLBState::RUNNING;
			break;


		case CLBEvents::PAUSE:
			source_ = CLBState::RUNNING;
			target_ = CLBState::PAUSED;
			break;


		case CLBEvents::CONTINUE:
			source_ = CLBState::PAUSED;
			target_ = CLBState::RUNNING;
			break;


		case CLBEvents::STOP:
			source_ = CLBState::PAUSED;
			target_ = CLBState::STAND_BY;
			break;


		case CLBEvents::QUIT:
			source_ = CLBState::READY;
			target_ = CLBState::STAND_BY;
			break;


		case CLBEvents::RESET:
			source_ = CLBState::STAND_BY;
			target_ = CLBState::IDLE;
			break;
	}
}

