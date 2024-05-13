#include "../include/screen.h"


//	
//		Global Variable Definitions
//	

SDL_Window *g_window = NULL;
SDL_Renderer *g_renderer = NULL;
int g_screen_width = 0;
int g_screen_height = 0;


//	
//		Function Definitions
//	

//	Initialises the screen/window
//	
//	This initialises the window as a pixelated one.
//	This means that the provided size is the logical size,
//	and the actual window size will be automatically determined
//	from the full screen size
void Screen_Init(const char *window_name, int screen_width, int screen_height) {
	if (g_window != NULL || g_renderer != NULL)
		Log_Message(LOG_WARNING, "Tried to init screen after it was already initialised");

	// Get Window Size
	SDL_DisplayMode dmode;
	float logical_aspect_ratio = screen_height / screen_width;
	int window_w, window_h;
	if (SDL_GetCurrentDisplayMode(0, &dmode) == 0) {
		window_w = (dmode.h/2);
		window_h = (dmode.h/2)*logical_aspect_ratio;
	} else {
		Log_SDLMessage(LOG_ERROR, "Failed to retrieve current display mode");
		window_w = 650;
		window_h = 480;
	}

	// Create Window
	g_window = SDL_CreateWindow(
		window_name,
		window_w, window_h,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOW_SHOWN
		| SDL_WINDOW_RESIZABLE
	);
	if (g_window == NULL) Log_SDLMessage(LOG_FATAL, "Failed to create Window");

	// Set up renderer
	g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
	if (g_renderer == NULL)
		Log_SDLMessage(LOG_FATAL, "Failed to create Renderer");
	if (SDL_RenderSetLogicalSize(g_renderer, screen_width, screen_height) != 0)
		Log_SDLMessage(LOG_ERROR, "Failed to set renderer's logical size");
	if (SDL_RenderSetIntegerScale(g_renderer, true) != 0)
		Log_SDLMessage(LOG_ERROR, "Failed to set renderer to integer scaling");
}

void Screen_Clear() {

}