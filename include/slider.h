#ifndef GT_SLIDER_C
#define GT_SLIDER_C
//
//		Basic Mouse-Slider interface element
//		
//		Creates sliders that can automatically update a
//		float through a pointer.


#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "screen.h"

#define MAX_SLIDERS 256
#define DEFAULT_KNOB_CLR (SDL_Colour){ 0x11, 0x11, 0x11, 0xFF }
#define DEFAULT_SLIDER_CLR (SDL_Colour){ 0xAA, 0xAA, 0xAA, 0xFF }

typedef enum SliderOrientation {
	SLIDER_HORIZONTAL,
	SLIDER_VERTICAL,
} SliderOrientation;

typedef struct Slider {
	int x; int y;
	int length;
	float factor;
	float *var;
	bool enabled;
	bool is_held;
	SliderOrientation orient;
} Slider;

//
//	Functions
//

//	Set the visual style of all slider knobs
//	
void Slider_SetKnobStyle(int width, int length, SDL_Colour colour);

//	Set the visual style of all slider sliders
//	
void Slider_SetSliderStyle(int width, SDL_Colour colour);

//	Creates a Slider
//	
//	Stores the slider data internally
//	Returns the ID of the created Slider
int Slider_Create(int x, int y, int length, SliderOrientation orient, float *var, float factor);

//	Gets a slider by its ID
//	
Slider *Slider_Get(int id);

//	Cleans up all the sliders
//	
void Slider_Term();

//	Draws all the sliders to the screen
//	
void Slider_DrawAll();

//	Handles mouse button events
//	
//	You don't need to worry about calling this, if you use `events.h`
void Slider_HandleMouseButtonEvent(SDL_MouseButtonEvent event);

//	Handles mouse motion events
//	
//	You don't need to worry about calling this, if you use `events.h`
void Slider_HandleMouseMotionEvent(SDL_MouseMotionEvent event);

#endif