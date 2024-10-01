#include "../include/events.h"

//	
//		Global Variable Definitions
//	

static SDL_EventType __uevent_type_clock;


//	
//		Function Definitions
//	

//	Internal Clock Callback function
//	
//	This function is called for every clock tick
//	and just serves to push the clock event
static Uint32 __cb_clock(Uint32 interval, void *param) {
	SDL_UserEvent event = {
		.type = __uevent_type_clock,
		.code = UEVENT_CLOCK_TICK,
	};
	SDL_PushEvent((SDL_Event *) &event);

	// Handle all internal things that need to be called every frame


	if (g_isRunning) return interval;
	else return 0;
}


void Events_Init(Uint32 ms_per_tick) {
	if (SDL_InitSubSystem(SDL_INIT_TIMER) != 0) {
		Log_MessageFull(NULL, LOG_FATAL, "Failed to initialise Timer Subsystem", true);
	};

	// Register User Events
	__uevent_type_clock = SDL_RegisterEvents(1);

	// Start System Clock
	SDL_AddTimer(ms_per_tick, __cb_clock, NULL);
}

void Events_HandleInternal(SDL_Event event) {
	switch (event.type) {
		case SDL_QUIT: g_isRunning = false; return;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP: {
			Button_HandleMouseButtonEvent(event.button);
			Slider_HandleMouseButtonEvent(event.button);
		} break;

		case SDL_MOUSEMOTION: {
			Button_HandleMouseMotionEvent(event.motion);
			Slider_HandleMouseMotionEvent(event.motion);
		} break;
		
	}
}