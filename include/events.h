#ifndef GT_EVENTS_H
#define GT_EVENTS_H
//	
//				Event Utilities
//	
//		Handles custom user events and timing
//	
//		(Hierarchy-Level: Mid)
//	
//	To-Do:
//	 - Add input binding-events (bypass having to catch them manually)

#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include "util.h"
#include "log.h"
#include "button.h"
#include "slider.h"


//	
//		Constant Definitions
//	

// This event is sent for each clock tick
#define UEVENT_CLOCK_TICK		0x00

//	
//		Function Declarations
//	

//	Initialise Events-System and Main Clock
//	
//	This will initialise a user event and timer this will trigger
//	
void Events_Init(Uint32 ms_per_tick);

//	Pass current event on to GinTonic to handle
//	
//	This function should be called before you check any
//	received events yourself if you use any of GinTonic's
//	input utilities (<button.h>)
void Events_HandleInternal(SDL_Event event);

//	Pushes a gin-tonic event to the event queue
//	
//	Automatically sets the event's `type`
//	(Mainly used internally)
void Events_GTEvent_Push(Sint32 code, void *data1, void *data2);

#endif