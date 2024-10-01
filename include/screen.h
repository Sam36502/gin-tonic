#ifndef GT_SCREEN_H
#define GT_SCREEN_H
//	
//				Screen Utilities
//	
//		Handles creating and drawing to the screen/window.
//		Also includes global references to the window/renderer.
//	

#include <SDL2/SDL.h>
#include "log.h"

//	
//		Constants/Macro Definitions
//	

// Define some common colours
#define COLOUR_WHITE (struct SDL_Colour){ 0xFF, 0xFF, 0xFF, 0xFF }
#define COLOUR_BLACK (struct SDL_Colour){ 0x00, 0x00, 0x00, 0xFF }

#define CLR_TO_RGB(clr) clr.r, clr.g, clr.b
#define CLR_TO_RGBA(clr) clr.r, clr.g, clr.b, clr.a



//	
//		Type Definitions
//	

//	
//		Global Variable Declarations
//	

extern SDL_Window *g_window;
extern SDL_Renderer *g_renderer;
extern int g_screen_width;
extern int g_screen_height;


//	
//		Function Declarations
//	

void Screen_Init(const char *window_name, int screen_width, int screen_height);
void Screen_Term();
SDL_Colour Screen_ParseColour(char *hex_str);


#endif