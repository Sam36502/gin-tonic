#ifndef GT_SCREEN_H
#define GT_SCREEN_H
//	
//				Screen Utilities
//	
//		Handles creating and drawing to the screen/window.
//		Also includes global references to the window/renderer.
//	

#include <SDL2/SDL.h>
#include "../include/log.h"

//	
//		Constant Definitions
//	


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
void Util_ClearScreen();


#endif