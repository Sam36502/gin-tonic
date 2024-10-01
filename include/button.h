#ifndef GT_BUTTON_H
#define GT_BUTTON_H
//	
//				Button Utilities
//	
//		Handles Capturing mouse input over a specific screen area.
//		When a button has been clicked, you can specify a callback function.
//	


#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "screen.h"


//	
//		Constant Definitions
//	

#define MAX_BUTTONS 256

//	
//		Type Definitions
//	

typedef struct Button {
	SDL_Rect rect;
	void (* callback_on)(int ID, void *udata);
	void (* callback_off)(int ID, void *udata);
	int id;
	void *udata;
	bool is_toggle;	// Whether this switch should act like a toggle
	bool is_on;
	bool hovering;
	SDL_Colour clr_off;
	SDL_Colour clr_hover;
	SDL_Colour clr_on;
} Button;


//	
//	Function Declarations
//	


//	Destroys all registered buttons and cleans up
//	
void Button_Term();

//	Creates a new Button
//	
//	`rect` is the screen space to check for mouse-clicks.
//	If a click is detected, `callback_on` will be called,
//	and similarly, `callback_off` is called if the mouse button is then released,
//	or leaves the button area. `udata` is arbitrary data to pass to the callbacks.
//	
//	The created button is stored in an internal registry, so that they can all be
//	easily checked on each frame.
//	
//	Returns the ID of the created Button.
int Button_Create(
	SDL_Rect rect,
	bool is_toggleable,
	void (* callback_on)(int ID, void *udata),
	void (* callback_off)(int ID, void *udata),
	void *udata,
	SDL_Colour clr_off, SDL_Colour clr_hover, SDL_Colour clr_on 
);

//	Retrieves the button of the given ID
//	
Button *Button_Get(int id);

//	Draws all onscreen buttons in their current states
//	
void Button_DrawAll();

//	Handles mouse-button events for buttons
//	
//	Used Internally.
//	You don't need to call this, but you do need to call `Events_HandleInternal()`
//	for buttons to function correctly.
void Button_HandleMouseButtonEvent(SDL_MouseButtonEvent event);

//	Handles mouse-motion events for buttons
//	
//	Used Internally.
//	You don't need to call this, but you do need to call `Events_HandleInternal()`
//	for buttons to function correctly.
void Button_HandleMouseMotionEvent(SDL_MouseMotionEvent event);

#endif