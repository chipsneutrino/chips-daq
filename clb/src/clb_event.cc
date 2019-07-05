/**
 *  ClbEvent - Class dealing with  CLB event and possible transitions  
 */

#include <clb/clb_event.h>
	
ClbEvent::ClbEvent(int event)
{
	event_ = event;
	SetStates();
}

void ClbEvent::SetId(int event) 
{
	event_ = event;
	SetStates();
}

void ClbEvent::SetStates()
{  
	switch(event_) {
		case ClbEvents::INIT:    
			source_ = ClbState::IDLE;   
			target_ = ClbState::STAND_BY;    
			break;
	

		case ClbEvents::CONFIGURE:
			source_ = ClbState::STAND_BY;
			target_ = ClbState::READY;
			break;


		case ClbEvents::START:    
			source_ =  ClbState::READY;   
			target_ =  ClbState::RUNNING;
			break;


		case ClbEvents::PAUSE:
			source_ = ClbState::RUNNING;
			target_ = ClbState::PAUSED;
			break;


		case ClbEvents::CONTINUE:
			source_ = ClbState::PAUSED;
			target_ = ClbState::RUNNING;
			break;


		case ClbEvents::STOP:
			source_ = ClbState::PAUSED;
			target_ = ClbState::STAND_BY;
			break;


		case ClbEvents::QUIT:
			source_ = ClbState::READY;
			target_ = ClbState::STAND_BY;
			break;


		case ClbEvents::RESET:
			source_ = ClbState::STAND_BY;
			target_ = ClbState::IDLE;
			break;
	}
}

